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

#include "mimir/search/match_tree/construction_helpers/inverse_node_creation.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/interface.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/placeholder.hpp"
#include "mimir/search/match_tree/construction_helpers/node_creation.hpp"
#include "mimir/search/match_tree/construction_helpers/split_metrics.hpp"
#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/node_splitters/base.hpp"
#include "mimir/search/match_tree/nodes/interface.hpp"
#include "mimir/search/match_tree/options.hpp"
#include "mimir/search/match_tree/statistics.hpp"

namespace mimir::match_tree
{

template<typename Derived_, HasConjunctiveCondition Element>
NodeSplitterBase<Derived_, Element>::NodeSplitterBase(const PDDLRepositories& pddl_repositories, const Options& options) :
    m_pddl_repositories(pddl_repositories),
    m_options(options)
{
}

template<typename Derived_, HasConjunctiveCondition Element>
std::pair<Node<Element>, Statistics> NodeSplitterBase<Derived_, Element>::fit(std::span<const Element*> elements)
{
    auto statistics = Statistics();
    statistics.construction_start_time_point = std::chrono::high_resolution_clock::now();
    statistics.num_elements = elements.size();

    auto placeholder_leafs = PlaceholderNodeList<Element> {};
    placeholder_leafs.push_back(create_root_placeholder_node(std::span<const Element*>(elements.begin(), elements.end())));
    auto inverse_generator_leafs = InverseNodeList<Element> {};
    auto inverse_root = InverseNode<Element> { nullptr };

    while (!placeholder_leafs.empty())
    {
        /* Customization point in derived classes: how to select the node and the split? */
        auto [inverse_root_, placeholder_children_] = self().split_node_impl(placeholder_leafs);

        if (inverse_root_)
        {
            inverse_root = std::move(inverse_root_);
        }
        statistics.num_nodes += placeholder_children_.size();
        for (auto& child : placeholder_children_)
        {
            placeholder_leafs.push_back(std::move(child));
        }

        if (statistics.num_nodes >= m_options.max_num_nodes)
        {
            /* Mark the tree as imperfect and translate the remaining placeholder nodes to generator nodes. */
            for (auto& placeholder_leaf : placeholder_leafs)
            {
                create_imperfect_generator_node(placeholder_leaf);
            }
            break;
        }
    }

    auto root = parse_inverse_tree_iteratively(inverse_root);

    parse_generator_distribution_iteratively(root, statistics);

    statistics.construction_end_time_point = std::chrono::high_resolution_clock::now();

    return { std::move(root), statistics };
}
}

#endif
