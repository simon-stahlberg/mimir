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

#ifndef MIMIR_SEARCH_MATCH_TREE_NODE_SPLITTERS_BASE_IMPL_HPP_
#define MIMIR_SEARCH_MATCH_TREE_NODE_SPLITTERS_BASE_IMPL_HPP_

#include "mimir/common/filesystem.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_node_creation.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/interface.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/placeholder.hpp"
#include "mimir/search/match_tree/construction_helpers/node_creation.hpp"
#include "mimir/search/match_tree/construction_helpers/split.hpp"
#include "mimir/search/match_tree/construction_helpers/split_metrics.hpp"
#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/node_splitters/base.hpp"
#include "mimir/search/match_tree/node_splitters/interface.hpp"
#include "mimir/search/match_tree/nodes/interface.hpp"
#include "mimir/search/match_tree/options.hpp"
#include "mimir/search/match_tree/statistics.hpp"

#include <queue>

namespace mimir::search::match_tree
{

template<formalism::IsFluentOrDerivedTag P>
using AtomDistributions = std::unordered_map<formalism::GroundAtom<P>, AtomSplitDistribution>;
using NumericConstraintDistributions = std::unordered_map<formalism::GroundNumericConstraint, NumericConstraintSplitDistribution>;

template<typename Derived_, formalism::HasConjunctiveCondition E>
NodeSplitterBase<Derived_, E>::NodeSplitterBase(const formalism::Repositories& pddl_repositories, const Options& options) :
    m_pddl_repositories(pddl_repositories),
    m_options(options)
{
}

template<typename Derived_, formalism::HasConjunctiveCondition E>
SplitSet NodeSplitterBase<Derived_, E>::compute_splits(const std::span<const E*>& elements)
{
    auto fluent_atom_distributions = AtomDistributions<formalism::FluentTag> {};
    auto derived_atom_distributions = AtomDistributions<formalism::DerivedTag> {};
    auto numeric_constraint_distributions = NumericConstraintDistributions {};

    for (const auto& element : elements)
    {
        const auto& conjunctive_condition = element->get_conjunctive_condition();

        /* Fluent */
        for (const auto& index : conjunctive_condition->template get_positive_precondition<formalism::FluentTag>())
        {
            ++fluent_atom_distributions[this->m_pddl_repositories.template get_ground_atom<formalism::FluentTag>(index)].num_true_elements;
        }
        for (const auto& index : conjunctive_condition->template get_negative_precondition<formalism::FluentTag>())
        {
            ++fluent_atom_distributions[this->m_pddl_repositories.template get_ground_atom<formalism::FluentTag>(index)].num_false_elements;
        }
        /* Derived */
        for (const auto& index : conjunctive_condition->template get_positive_precondition<formalism::DerivedTag>())
        {
            ++derived_atom_distributions[this->m_pddl_repositories.template get_ground_atom<formalism::DerivedTag>(index)].num_true_elements;
        }
        for (const auto& index : conjunctive_condition->template get_negative_precondition<formalism::DerivedTag>())
        {
            ++derived_atom_distributions[this->m_pddl_repositories.template get_ground_atom<formalism::DerivedTag>(index)].num_false_elements;
        }
        /* Numeric constraint */
        for (const auto& numeric_constraint : conjunctive_condition->get_numeric_constraints())
        {
            ++numeric_constraint_distributions[numeric_constraint].num_true_elements;
        }
    }
    for (auto& [atom, distribution] : fluent_atom_distributions)
    {
        distribution.num_dont_care_elements = elements.size() - distribution.num_true_elements - distribution.num_false_elements;
    }
    for (auto& [atom, distribution] : derived_atom_distributions)
    {
        distribution.num_dont_care_elements = elements.size() - distribution.num_true_elements - distribution.num_false_elements;
    }
    for (auto& [constraint, distribution] : numeric_constraint_distributions)
    {
        distribution.num_dont_care_elements = elements.size() - distribution.num_true_elements;
    }

    auto splits = SplitSet {};
    for (const auto& [atom, distribution] : fluent_atom_distributions)
    {
        splits.insert(AtomSplit<formalism::FluentTag> { atom, distribution });
    }
    for (const auto& [atom, distribution] : derived_atom_distributions)
    {
        splits.insert(AtomSplit<formalism::DerivedTag> { atom, distribution });
    }
    for (const auto& [constraint, distribution] : numeric_constraint_distributions)
    {
        splits.insert(NumericConstraintSplit { constraint, distribution });
    }

    return splits;
}

template<typename Derived_, formalism::HasConjunctiveCondition E>
std::optional<SplitScoreAndUselessSplits> NodeSplitterBase<Derived_, E>::compute_refinement_data(const PlaceholderNode<E>& node)
{
    auto splits = compute_splits(node->get_elements());

    /* Erase previous useless splits */
    for (auto cur_node = node->get_parent(); cur_node; cur_node = cur_node->get_parent())
    {
        for (const auto& useless_split : cur_node->get_useless_splits())
        {
            splits.erase(useless_split);
        }
    }

    /* Compute useless splits and best split */
    auto best_split = std::optional<Split>();
    auto best_score = worst_score(m_options.optimization_direction);
    auto useless_splits = SplitList {};
    for (const auto& split : splits)
    {
        if (is_useless_split(split))
        {
            useless_splits.push_back(split);
        }
        else
        {
            const auto score = compute_score(split, m_options.split_metric);

            // std::cout << "Evaluate split: " << split << " " << score << " " << best_score << std::endl;

            if (better_score(score, best_score, m_options.optimization_direction))
            {
                best_split = split;
                best_score = score;
            }
        }
    }

    if (!best_split)
    {
        return std::nullopt;  ///< no more splitting is needed
    }

    // std::cout << best_split.value() << " " << best_score << std::endl;

    /* Mark the current split as useless for subsequent splittings */
    if (best_split)
    {
        useless_splits.push_back(best_split.value());
    }

    // std::cout << "useless splits: " << useless_splits << std::endl;

    return SplitScoreAndUselessSplits { best_split.value(), best_score, std::move(useless_splits) };
}

template<typename Derived_, formalism::HasConjunctiveCondition E>
std::pair<Node<E>, Statistics> NodeSplitterBase<Derived_, E>::fit(std::span<const E*> elements)
{
    auto statistics = Statistics();
    statistics.construction_start_time_point = std::chrono::high_resolution_clock::now();
    statistics.num_elements = elements.size();

    auto inverse_root = self().fit_impl(elements, statistics);

    auto root = parse_inverse_tree_iteratively(inverse_root);

    if (m_options.enable_dump_dot_file)
    {
        auto ss = std::stringstream {};
        ss << std::make_tuple(std::cref(inverse_root), DotPrinterTag {}) << std::endl;
        write_to_file(m_options.output_dot_file, ss.str());
    }

    parse_generator_distribution_iteratively(root, statistics);

    statistics.construction_end_time_point = std::chrono::high_resolution_clock::now();

    return { std::move(root), statistics };
}
}

#endif
