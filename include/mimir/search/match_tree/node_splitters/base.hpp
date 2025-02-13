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

#ifndef MIMIR_SEARCH_MATCH_TREE_NODE_SPLITTERS_BASE_HPP_
#define MIMIR_SEARCH_MATCH_TREE_NODE_SPLITTERS_BASE_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/node_splitters/interface.hpp"

namespace mimir::match_tree
{

template<typename Derived_, HasConjunctiveCondition Element>
class NodeSplitterBase : public INodeSplitter<Element>
{
private:
    /// @brief Helper to cast to Derived_.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

protected:
    const PDDLRepositories& m_pddl_repositories;
    const Options& m_options;

    template<DynamicPredicateTag P>
    using AtomDistributions = std::unordered_map<GroundAtom<P>, AtomSplitDistribution>;
    using NumericConstraintDistributions = std::unordered_map<GroundNumericConstraint, NumericConstraintSplitDistribution>;

    /// @brief Compute the precondition distributions over all elements.
    /// @param elements the elements in a placeholder node.
    /// @return a distribution for each precondition, i.e., fluent atom, derived atom, and numeric constraint.
    SplitList compute_splits(const std::span<const Element*>& elements)
    {
        auto fluent_atom_distributions = AtomDistributions<Fluent> {};
        auto derived_atom_distributions = AtomDistributions<Derived> {};
        auto numeric_constraint_distributions = NumericConstraintDistributions {};

        for (const auto& element : elements)
        {
            const auto& conjunctive_condition = element->get_conjunctive_condition();

            /* Fluent */
            for (const auto& index : conjunctive_condition.template get_positive_precondition<Fluent>())
            {
                ++fluent_atom_distributions[this->m_pddl_repositories.template get_ground_atom<Fluent>(index)].m_num_true_elements;
            }
            for (const auto& index : conjunctive_condition.template get_negative_precondition<Fluent>())
            {
                ++fluent_atom_distributions[this->m_pddl_repositories.template get_ground_atom<Fluent>(index)].m_num_false_elements;
            }
            /* Derived */
            for (const auto& index : conjunctive_condition.template get_positive_precondition<Derived>())
            {
                ++derived_atom_distributions[this->m_pddl_repositories.template get_ground_atom<Derived>(index)].m_num_true_elements;
            }
            for (const auto& index : conjunctive_condition.template get_negative_precondition<Derived>())
            {
                ++derived_atom_distributions[this->m_pddl_repositories.template get_ground_atom<Derived>(index)].m_num_false_elements;
            }
            /* Numeric constraint */
            for (const auto& numeric_constraint : conjunctive_condition.get_numeric_constraints())
            {
                ++numeric_constraint_distributions[numeric_constraint.get()].m_num_true_elements;
            }
        }
        for (auto& [atom, distribution] : fluent_atom_distributions)
        {
            distribution.m_num_dont_care_elements = elements.size() - distribution.m_num_true_elements - distribution.m_num_false_elements;
        }
        for (auto& [atom, distribution] : derived_atom_distributions)
        {
            distribution.m_num_dont_care_elements = elements.size() - distribution.m_num_true_elements - distribution.m_num_false_elements;
        }
        for (auto& [constraint, distribution] : numeric_constraint_distributions)
        {
            distribution.m_num_dont_care_elements = elements.size() - distribution.m_num_true_elements;
        }

        /* Sort splits descending in score, break ties in lexicographically*/
        std::map<std::pair<double, std::string>, Split, std::greater<std::pair<double, std::string>>> sorted_splits;
        for (const auto& [atom, distribution] : fluent_atom_distributions)
        {
            const auto split = AtomSplit<Fluent> { atom, distribution };
            const auto score = compute_score(distribution, this->m_options.split_metric);
            sorted_splits.emplace(std::make_pair(score, to_string(atom)), split);
        }
        for (const auto& [atom, distribution] : derived_atom_distributions)
        {
            const auto split = AtomSplit<Derived> { atom, distribution };
            const auto score = compute_score(distribution, this->m_options.split_metric);
            sorted_splits.emplace(std::make_pair(score, to_string(atom)), split);
        }
        for (const auto& [constraint, distribution] : numeric_constraint_distributions)
        {
            const auto split = NumericConstraintSplit { constraint, distribution };
            const auto score = compute_score(distribution, this->m_options.split_metric);
            sorted_splits.emplace(std::make_pair(score, to_string(constraint)), split);
        }
        assert(sorted_splits.size() == fluent_atom_distributions.size() + derived_atom_distributions.size() + numeric_constraint_distributions.size());

        auto splits = SplitList {};
        splits.reserve(sorted_splits.size());
        for (const auto& [key, split] : sorted_splits)
        {
            splits.push_back(split);
        }

        return splits;
    }

public:
    NodeSplitterBase(const PDDLRepositories& pddl_repositories, const Options& options);
    std::pair<Node<Element>, Statistics> fit(std::span<const Element*> elements) override;
};
}

#endif
