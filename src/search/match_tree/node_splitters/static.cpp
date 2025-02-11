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

#include "mimir/search/match_tree/node_splitters/static.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_node_creation.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/interface.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/placeholder.hpp"
#include "mimir/search/match_tree/construction_helpers/split_metrics.hpp"
#include "mimir/search/match_tree/node_splitters/interface.hpp"

#include <map>

namespace mimir::match_tree
{
template<HasConjunctiveCondition Element>
SplitList StaticNodeSplitter<Element>::compute_statically_best_splits(const std::vector<const Element*>& elements)
{
    /* Collect frequencies */
    auto fluent_atom_distributions = std::unordered_map<GroundAtom<Fluent>, AtomSplitDistribution> {};
    auto derived_atom_distributions = std::unordered_map<GroundAtom<Derived>, AtomSplitDistribution> {};
    auto numeric_constraint_distributions = std::unordered_map<GroundNumericConstraint, NumericConstraintSplitDistribution> {};

    for (const auto& element : elements)
    {
        const auto& conjunctive_condition = element->get_conjunctive_condition();

        /* Fluent */
        for (const auto& index : conjunctive_condition.template get_positive_precondition<Fluent>())
        {
            ++fluent_atom_distributions[m_pddl_repositories.template get_ground_atom<Fluent>(index)].m_num_true_elements;
        }
        for (const auto& index : conjunctive_condition.template get_negative_precondition<Fluent>())
        {
            ++fluent_atom_distributions[m_pddl_repositories.template get_ground_atom<Fluent>(index)].m_num_false_elements;
        }
        /* Derived */
        for (const auto& index : conjunctive_condition.template get_positive_precondition<Derived>())
        {
            ++derived_atom_distributions[m_pddl_repositories.template get_ground_atom<Derived>(index)].m_num_true_elements;
        }
        for (const auto& index : conjunctive_condition.template get_negative_precondition<Derived>())
        {
            ++derived_atom_distributions[m_pddl_repositories.template get_ground_atom<Derived>(index)].m_num_false_elements;
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
        const auto score = compute_score(distribution, m_split_metric);
        sorted_splits.emplace(std::make_pair(score, to_string(atom)), split);
    }
    for (const auto& [atom, distribution] : derived_atom_distributions)
    {
        const auto split = AtomSplit<Derived> { atom, distribution };
        const auto score = compute_score(distribution, m_split_metric);
        sorted_splits.emplace(std::make_pair(score, to_string(atom)), split);
    }
    for (const auto& [constraint, distribution] : numeric_constraint_distributions)
    {
        const auto split = NumericConstraintSplit { constraint, distribution };
        const auto score = compute_score(distribution, m_split_metric);
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

template<HasConjunctiveCondition Element>
StaticNodeSplitter<Element>::StaticNodeSplitter(const PDDLRepositories& pddl_repositories,
                                                SplitMetricEnum split_metric,
                                                const std::vector<const Element*>& elements) :
    m_pddl_repositories(pddl_repositories),
    m_split_metric(split_metric),
    m_splits(compute_statically_best_splits(elements))
{
    std::cout << "[MatchTree] Statically split ordering determined with " << to_string(m_split_metric) << " score: " << std::endl;
    for (size_t i = 0; i < m_splits.size(); ++i)
    {
        std::cout << "    " << i << ". " << m_splits[i] << std::endl;
    }
}

template<HasConjunctiveCondition Element>
std::pair<InverseNode<Element>, PlaceholderNodeList<Element>> StaticNodeSplitter<Element>::compute_best_split(const PlaceholderNode<Element>& node)
{
    auto useless_splits = SplitList {};

    for (size_t i = node->get_root_distance(); i < m_splits.size(); ++i)
    {
        auto result = create_node_and_placeholder_children(node, useless_splits, i, m_splits[i]);

        if (result.has_value())
        {
            return std::move(result.value());
        }

        useless_splits.push_back(m_splits[i]);
    }

    return create_generator_node(node, m_splits.size());
}

template<HasConjunctiveCondition Element>
InverseNode<Element> StaticNodeSplitter<Element>::translate_to_generator_node(const PlaceholderNode<Element>& node) const
{
    auto [generator_node, children] = create_generator_node(node, m_splits.size());
    assert(children.empty());

    return std::move(generator_node);
}

template class StaticNodeSplitter<GroundActionImpl>;
template class StaticNodeSplitter<GroundAxiomImpl>;

}
