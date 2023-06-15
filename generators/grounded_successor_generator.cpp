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


#include "grounded_successor_generator.hpp"

namespace planners
{
    DecisionNode::~DecisionNode() = default;

    BranchNode::BranchNode(uint32_t rank) : rank_(rank) {}

    LeafNode::LeafNode(const formalism::ActionList& actions) : actions_(actions) {}

    GroundedSuccessorGenerator::GroundedSuccessorGenerator(const formalism::ProblemDescription& problem, const formalism::ActionList& ground_actions) :
        problem_(problem),
        root_(nullptr)
    {
        root_ = build_decision_tree(problem, ground_actions);
    }

    formalism::ActionList GroundedSuccessorGenerator::get_applicable_actions(const formalism::State& state) const
    {
        if (problem_ != state->get_problem())
        {
            throw std::invalid_argument("successor generator is built for a different problem");
        }

        formalism::ActionList applicable_actions;
        get_applicable_actions_recursive(root_.get(), state, applicable_actions);
        return applicable_actions;
    }

    formalism::AtomSet::const_iterator GroundedSuccessorGenerator::select_branching_atom(const formalism::ActionList& ground_actions,
                                                                                         const formalism::AtomSet& unique_atoms,
                                                                                         formalism::AtomSet::const_iterator next_atom)
    {
        std::equal_to<formalism::Atom> equal_to;

        while (next_atom != unique_atoms.end())
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

    std::unique_ptr<DecisionNode> GroundedSuccessorGenerator::build_decision_tree(const formalism::ProblemDescription& problem,
                                                                                  const formalism::ActionList& ground_actions)
    {
        std::unordered_set<formalism::Atom> unique_atoms;

        // Find unique atoms from ground action preconditions
        for (const auto& action : ground_actions)
        {
            for (const auto& literal : action->get_precondition())
            {
                unique_atoms.insert(literal->atom);
            }
        }

        return build_decision_tree_recursive(problem, ground_actions, unique_atoms, unique_atoms.begin());
    }

    std::unique_ptr<DecisionNode> GroundedSuccessorGenerator::build_decision_tree_recursive(const formalism::ProblemDescription& problem,
                                                                                            const formalism::ActionList& ground_actions,
                                                                                            const formalism::AtomSet& unique_atoms,
                                                                                            formalism::AtomSet::const_iterator next_atom)
    {
        next_atom = select_branching_atom(ground_actions, unique_atoms, next_atom);

        if ((next_atom == unique_atoms.end()) || ground_actions.empty())
        {
            return std::make_unique<LeafNode>(ground_actions);
        }

        formalism::Atom branching_atom = *next_atom;

        formalism::ActionList present_actions;
        formalism::ActionList not_present_actions;
        formalism::ActionList dont_care_actions;

        std::equal_to<formalism::Atom> equal_to;

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
        auto branch_node = std::make_unique<BranchNode>(problem->get_rank(branching_atom));
        branch_node->present_ = build_decision_tree_recursive(problem, present_actions, unique_atoms, next_atom);
        branch_node->not_present_ = build_decision_tree_recursive(problem, not_present_actions, unique_atoms, next_atom);
        branch_node->dont_care_ = build_decision_tree_recursive(problem, dont_care_actions, unique_atoms, next_atom);
        return branch_node;
    }

    void GroundedSuccessorGenerator::get_applicable_actions_recursive(const DecisionNode* node,
                                                                      const formalism::State& state,
                                                                      formalism::ActionList& applicable_actions) const
    {
        if (!node)
        {
            return;
        }

        if (const auto leaf = dynamic_cast<const LeafNode*>(node))
        {
            applicable_actions.insert(applicable_actions.end(), leaf->actions_.begin(), leaf->actions_.end());
        }
        else if (const auto branch = dynamic_cast<const BranchNode*>(node))
        {
            bool atom_present = formalism::is_in_state(branch->rank_, state);

            if (atom_present)
            {
                get_applicable_actions_recursive(branch->present_.get(), state, applicable_actions);
            }
            else
            {
                get_applicable_actions_recursive(branch->not_present_.get(), state, applicable_actions);
            }

            get_applicable_actions_recursive(branch->dont_care_.get(), state, applicable_actions);
        }
    }
}  // namespace planners
