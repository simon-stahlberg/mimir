/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
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

#ifndef MIMIR_SEARCH_MATCH_TREE_NODE_SPLITTERS_DYNAMIC_HPP_
#define MIMIR_SEARCH_MATCH_TREE_NODE_SPLITTERS_DYNAMIC_HPP_

#include "mimir/search/match_tree/construction_helpers/inverse_nodes/atom.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/generator.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/interface.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/numeric_constraint.hpp"
#include "mimir/search/match_tree/node_splitters/interface.hpp"

namespace mimir::match_tree
{
template<HasConjunctiveCondition Element>
class DynamicNodeSplitter : public INodeSplitter<Element>
{
private:
    const PDDLRepositories& m_pddl_repositories;

public:
    explicit DynamicNodeSplitter(const PDDLRepositories& pddl_repositories) : m_pddl_repositories(pddl_repositories) {}

    std::variant<PlaceholderNodeList<Element>, InverseNode<Element>> compute_best_split(std::span<const Element*> elements,
                                                                                        const PlaceholderNode<Element>& node) override
    {
        std::cout << "compute_node" << std::endl;

        /* Collect frequencies */
        auto fluent_atom_frequencies = std::unordered_map<GroundAtom<Fluent>, size_t> {};
        auto derived_atom_frequencies = std::unordered_map<GroundAtom<Derived>, size_t> {};
        auto numeric_constraint_frequencies = std::unordered_map<GroundNumericConstraint, size_t> {};

        for (const auto& element : elements)
        {
            const auto& conjunctive_condition = element->get_conjunctive_condition();

            /* Fluent */
            for (const auto& index : conjunctive_condition.template get_positive_precondition<Fluent>())
            {
                ++fluent_atom_frequencies[m_pddl_repositories.template get_ground_atom<Fluent>(index)];
            }
            for (const auto& index : conjunctive_condition.template get_negative_precondition<Fluent>())
            {
                ++fluent_atom_frequencies[m_pddl_repositories.template get_ground_atom<Fluent>(index)];
            }
            /* Derived */
            for (const auto& index : conjunctive_condition.template get_positive_precondition<Derived>())
            {
                ++derived_atom_frequencies[m_pddl_repositories.template get_ground_atom<Derived>(index)];
            }
            for (const auto& index : conjunctive_condition.template get_negative_precondition<Derived>())
            {
                ++derived_atom_frequencies[m_pddl_repositories.template get_ground_atom<Derived>(index)];
            }
            /* Numeric constraint */
            for (const auto& numeric_constraint : conjunctive_condition.get_numeric_constraints())
            {
                ++numeric_constraint_frequencies[numeric_constraint.get()];
            }
        }

        std::cout << "fluent_atom_frequencies: " << fluent_atom_frequencies << std::endl;
        std::cout << "derived_atom_frequencies: " << derived_atom_frequencies << std::endl;
        std::cout << "numeric_constraint_frequencies: " << numeric_constraint_frequencies << std::endl;

        /* Erase previous splits */

        class ErasePreviousSplitInverseNodeVisitor : public IInverseNodeVisitor<Element>
        {
        private:
            std::unordered_map<GroundAtom<Fluent>, size_t>& m_fluent_atom_frequencies;
            std::unordered_map<GroundAtom<Derived>, size_t>& m_derived_atom_frequencies;
            std::unordered_map<GroundNumericConstraint, size_t>& m_numeric_constraint_frequencies;

        public:
            ErasePreviousSplitInverseNodeVisitor(std::unordered_map<GroundAtom<Fluent>, size_t>& fluent_atom_frequencies,
                                                 std::unordered_map<GroundAtom<Derived>, size_t>& derived_atom_frequencies,
                                                 std::unordered_map<GroundNumericConstraint, size_t>& numeric_constraint_frequencies) :
                m_fluent_atom_frequencies(fluent_atom_frequencies),
                m_derived_atom_frequencies(derived_atom_frequencies),
                m_numeric_constraint_frequencies(numeric_constraint_frequencies)
            {
            }

            void repeat_with_parent(const InverseNode<Element>& parent)
            {
                if (parent)
                {
                    parent->visit(*this);
                }
            }

            void accept(const InverseAtomSelectorNode<Element, Fluent>& atom) override
            {
                m_fluent_atom_frequencies.erase(atom.get_atom());
                repeat_with_parent(atom.get_parent());
            }
            void accept(const InverseAtomSelectorNode<Element, Derived>& atom) override
            {
                m_derived_atom_frequencies.erase(atom.get_atom());
                repeat_with_parent(atom.get_parent());
            }
            void accept(const InverseNumericConstraintSelectorNode<Element>& constraint) override
            {
                m_numeric_constraint_frequencies.erase(constraint.get_constraint());
                repeat_with_parent(constraint.get_parent());
            }
            void accept(const InverseElementGeneratorNode<Element>& generator) override { repeat_with_parent(generator.get_parent()); }
        };

        if (parent.has_value())
        {
            auto erase_past_split_visitor =
                ErasePreviousSplitInverseNodeVisitor(fluent_atom_frequencies, derived_atom_frequencies, numeric_constraint_frequencies);
            parent.value()->visit(erase_past_split_visitor);
        }

        /* Compute highest frequency */
        auto highest_frequency = size_t(0);
        for (const auto& [atom, frequency] : fluent_atom_frequencies)
        {
            highest_frequency = std::max(highest_frequency, frequency);
        }
        for (const auto& [atom, frequency] : derived_atom_frequencies)
        {
            highest_frequency = std::max(highest_frequency, frequency);
        }
        for (const auto& [constraint, frequency] : numeric_constraint_frequencies)
        {
            highest_frequency = std::max(highest_frequency, frequency);
        }

        std::cout << "Highest frequency: " << highest_frequency << std::endl;

        /* Collect splits with highest frequency */

        // TODO: we must change this.
        // Iterate descending in the frequency.
        // Collect all splits that would result in a useless node until a useful one is found.
        // Then in the instantiated node, we must store the useless splits.
        auto highest_frequency_splits = CandidateSplitList {};
        for (const auto& [atom, frequency] : fluent_atom_frequencies)
        {
            if (frequency == highest_frequency)
                highest_frequency_splits.push_back(atom);
        }
        for (const auto& [atom, frequency] : derived_atom_frequencies)
        {
            if (frequency == highest_frequency)
                highest_frequency_splits.push_back(atom);
        }
        for (const auto& [constraint, frequency] : numeric_constraint_frequencies)
        {
            if (frequency == highest_frequency)
                highest_frequency_splits.push_back(constraint);
        }

        std::cout << "Highest frequency splits: " << highest_frequency_splits << std::endl;

        /* Break ties in favor of lexicographically smallest */
        // Note: in case we do not get deterministic results, we should work on normal form of function expressions in the translator.
        std::sort(highest_frequency_splits.begin(),
                  highest_frequency_splits.end(),
                  [](auto&& lhs, auto&& rhs)
                  {
                      std::stringstream ss_lhs;
                      ss_lhs << lhs;
                      std::stringstream ss_rhs;
                      ss_rhs << rhs;
                      return ss_lhs.str() < ss_rhs.str();
                  });

        std::cout << "Final split: " << highest_frequency_splits.front() << std::endl;

        // TODO: must check whether the split is actually useful...
        return { highest_frequency_splits.front(), SplitList {} };
    }
};

}

#endif
