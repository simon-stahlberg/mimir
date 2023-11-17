/*
 * Copyright (C) 2023 Simon Stahlberg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "../../include/mimir/datastructures/robin_map.hpp"
#include "../../include/mimir/generators/grounded_successor_generator.hpp"

#include <algorithm>

namespace mimir::planners
{
    DecisionNode::~DecisionNode() = default;

    BranchNode::BranchNode(uint32_t rank) : rank_(rank) {}

    LeafNode::LeafNode(const mimir::formalism::ActionList& actions) : actions_(actions) {}

    GroundedSuccessorGenerator::GroundedSuccessorGenerator(const mimir::formalism::ProblemDescription& problem,
                                                           const mimir::formalism::ActionList& ground_actions) :
        problem_(problem),
        actions_(ground_actions),
        root_(nullptr)
    {
        root_ = build_decision_tree(problem, ground_actions);
    }

    mimir::formalism::ProblemDescription GroundedSuccessorGenerator::get_problem() const { return problem_; }

    const mimir::formalism::ActionList& GroundedSuccessorGenerator::get_actions() const { return actions_; }

    mimir::formalism::ActionList GroundedSuccessorGenerator::get_applicable_actions(const mimir::formalism::State& state) const
    {
        if (problem_ != state->get_problem())
        {
            throw std::invalid_argument("successor generator is built for a different problem");
        }

        mimir::formalism::ActionList applicable_actions;

        if (root_)
        {
            root_->get_applicable_actions(state, applicable_actions);
        }

        return applicable_actions;
    }

    mimir::formalism::AtomList::const_iterator GroundedSuccessorGenerator::select_branching_atom(const mimir::formalism::ActionList& ground_actions,
                                                                                                 const mimir::formalism::AtomList& atoms,
                                                                                                 mimir::formalism::AtomList::const_iterator next_atom)
    {
        std::equal_to<mimir::formalism::Atom> equal_to;

        while (next_atom != atoms.end())
        {
            const auto atom = *next_atom;

            for (const auto& ground_action : ground_actions)
            {
                for (const auto& literal : ground_action->get_precondition())
                {
                    if (equal_to(literal->atom, atom))
                    {
                        return next_atom;
                    }
                }
            }

            ++next_atom;
        }

        return next_atom;
    }

    std::unique_ptr<DecisionNode> GroundedSuccessorGenerator::build_decision_tree(const mimir::formalism::ProblemDescription& problem,
                                                                                  const mimir::formalism::ActionList& ground_actions)
    {
        const auto& static_predicates = problem->domain->static_predicates;
        const auto& static_atoms = problem->get_static_atoms();

        mimir::tsl::robin_map<mimir::formalism::Atom, uint32_t> atom_occurrances;

        // Find unique atoms from ground action preconditions
        for (const auto& action : ground_actions)
        {
            for (const auto& literal : action->get_precondition())
            {
                const auto is_static_predicate = std::count(static_predicates.begin(), static_predicates.end(), literal->atom->predicate);

                if (is_static_predicate)
                {
                    // We can disregard static atoms, provided that all actions in "ground_actions" meet the precondition for doing so

                    if (static_atoms.contains(literal->atom) == literal->negated)
                    {
                        throw std::runtime_error("ground_actions contains an always inapplicable action");
                    }
                }
                else
                {
                    ++atom_occurrances[literal->atom];
                }
            }
        }

        mimir::formalism::AtomList atoms;
        atoms.reserve(atom_occurrances.size());

        for (const auto& [key, value] : atom_occurrances)
        {
            atoms.emplace_back(key);
        }

        const auto occurance_descending = [&atom_occurrances](const mimir::formalism::Atom& lhs, const mimir::formalism::Atom& rhs)
        { return atom_occurrances[lhs] > atom_occurrances[rhs]; };

        std::sort(atoms.begin(), atoms.end(), occurance_descending);

        return build_decision_tree_recursive(problem, ground_actions, atoms, atoms.begin());
    }

    std::unique_ptr<DecisionNode> GroundedSuccessorGenerator::build_decision_tree_recursive(const mimir::formalism::ProblemDescription& problem,
                                                                                            const mimir::formalism::ActionList& ground_actions,
                                                                                            const mimir::formalism::AtomList& atoms,
                                                                                            mimir::formalism::AtomList::const_iterator next_atom)
    {
        next_atom = select_branching_atom(ground_actions, atoms, next_atom);

        if ((next_atom == atoms.end()) || ground_actions.empty())
        {
            return std::make_unique<LeafNode>(ground_actions);
        }

        mimir::formalism::Atom branching_atom = *next_atom;

        mimir::formalism::ActionList present_actions;
        mimir::formalism::ActionList not_present_actions;
        mimir::formalism::ActionList dont_care_actions;

        std::equal_to<mimir::formalism::Atom> equal_to;

        for (const auto& ground_action : ground_actions)
        {
            bool present = false;
            bool not_present = false;

            for (const auto& literal : ground_action->get_precondition())
            {
                if (equal_to(literal->atom, branching_atom))
                {
                    if (literal->negated)
                    {
                        not_present = true;
                    }
                    else
                    {
                        present = true;
                    }

                    break;
                }
            }

            if (present)
            {
                present_actions.push_back(ground_action);
            }
            else if (not_present)
            {
                not_present_actions.push_back(ground_action);
            }
            else
            {
                dont_care_actions.push_back(ground_action);
            }
        }

        ++next_atom;

        if ((present_actions.size() == 0) && (not_present_actions.size() == 0) && (dont_care_actions.size() > 0))
        {
            return build_decision_tree_recursive(problem, dont_care_actions, atoms, next_atom);
        }
        else
        {
            auto branch_node = std::make_unique<BranchNode>(problem->get_rank(branching_atom));
            branch_node->present_ = build_decision_tree_recursive(problem, present_actions, atoms, next_atom);
            branch_node->not_present_ = build_decision_tree_recursive(problem, not_present_actions, atoms, next_atom);
            branch_node->dont_care_ = build_decision_tree_recursive(problem, dont_care_actions, atoms, next_atom);
            return branch_node;
        }
    }

    void BranchNode::get_applicable_actions(const mimir::formalism::State& state, mimir::formalism::ActionList& applicable_actions) const
    {
        const bool atom_present = mimir::formalism::is_in_state(rank_, state);

        if (atom_present)
        {
            present_->get_applicable_actions(state, applicable_actions);
        }
        else
        {
            not_present_->get_applicable_actions(state, applicable_actions);
        }

        dont_care_->get_applicable_actions(state, applicable_actions);
    }

    void LeafNode::get_applicable_actions(const mimir::formalism::State& state, mimir::formalism::ActionList& applicable_actions) const
    {
        applicable_actions.insert(applicable_actions.end(), actions_.cbegin(), actions_.cend());
    }
}  // namespace planners
