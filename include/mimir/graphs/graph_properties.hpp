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

#ifndef MIMIR_GRAPHS_GRAPH_PROPERTIES_HPP_
#define MIMIR_GRAPHS_GRAPH_PROPERTIES_HPP_

#include "mimir/common/hash.hpp"
#include "mimir/graphs/graph_edges.hpp"
#include "mimir/graphs/graph_interface.hpp"

#include <unordered_set>

namespace mimir
{

template<typename G>
requires IsEdgeListGraph<G>  //
bool is_undirected_graph(const G& graph)
{
    // Create datastructure for efficient lookup
    auto directed_edges = std::unordered_set<std::pair<Index, Index>, Hash<std::pair<Index, Index>>>();
    for (const auto& edge : graph.get_edges())
    {
        directed_edges.emplace(edge.get_source(), edge.get_target());
    }

    for (const auto& edge : graph.get_edges())
    {
        if (!directed_edges.contains(std::make_pair(edge.get_target(), edge.get_source())))
        {
            return false;  // found no matching anti-parallel edge => directed graph
        }
    }

    return true;  // found matching parallel edges => undirected graph
}

template<typename G>
requires IsEdgeListGraph<G>  //
bool is_multi_graph(const G& graph)
{
    auto directed_edges = std::unordered_set<std::pair<Index, Index>, Hash<std::pair<Index, Index>>>();
    for (const auto& edge : graph.get_edges())
    {
        if (!directed_edges.emplace(edge.get_source(), edge.get_target()).second)
        {
            return true;  // found parallel edge => multi-graph
        }
    }
    return false;  // found no parallel edges => not multi-graph
}

}

#endif