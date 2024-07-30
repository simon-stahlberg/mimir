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

#ifndef MIMIR_SEARCH_SEARCH_NODES_UTILS_HPP_
#define MIMIR_SEARCH_SEARCH_NODES_UTILS_HPP_

#include "mimir/search/action.hpp"
#include "mimir/search/search_nodes/interface.hpp"

#include <algorithm>

namespace mimir
{

/// @brief Compute the plan consisting of ground actions by collecting the creating actions
///        and reversing them.
/// @param search_nodes The collection of all search nodes.
/// @param search_node The search node from which to start backtracking.
/// @param[out] out_plan The sequence of ground actions that leads from the initial state to
///                      the to the state underlying the search node.
template<IsPathExtractableSearchNode SearchNode>
void set_plan(const flatmemory::FixedSizedTypeVector<typename SearchNode::Layout>& search_nodes,  //
              const SearchNode& search_node,
              GroundActionList& out_plan)
{
    out_plan.clear();
    auto cur_search_node = search_node;

    while (cur_search_node.get_parent_state().has_value())
    {
        assert(cur_search_node.get_creating_action().has_value());

        out_plan.push_back(cur_search_node.get_creating_action().value());

        cur_search_node = SearchNode(search_nodes.at(cur_search_node.get_parent_state().value().get_id()));
    }

    std::reverse(out_plan.begin(), out_plan.end());
}

}

#endif
