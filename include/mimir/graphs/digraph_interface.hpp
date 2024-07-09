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

#ifndef MIMIR_GRAPHS_DIGRAPH_INTERFACE_HPP_
#define MIMIR_GRAPHS_DIGRAPH_INTERFACE_HPP_

#include "mimir/graphs/digraph_edge_interface.hpp"
#include "mimir/graphs/digraph_iterators.hpp"

#include <concepts>
#include <cstdint>
#include <ranges>
#include <span>
#include <vector>

namespace mimir
{

template<typename T>
concept IsDigraph = requires(T a, DigraphVertexIndex vertex_index, DigraphEdgeWeight edge_weight) {
    typename T::EdgeType;
    requires IsDigraphEdge<typename T::EdgeType>;

    {
        a.add_edge(vertex_index, vertex_index, edge_weight)
    } -> std::same_as<DigraphEdgeIndex>;
    {
        a.get_targets(vertex_index)
    } -> std::same_as<DigraphTargetIndexIterator<typename T::EdgeType>>;
    {
        a.get_sources(vertex_index)
    } -> std::same_as<DigraphSourceIndexIterator<typename T::EdgeType>>;
    {
        a.get_forward_edge_indices(vertex_index)
    } -> std::same_as<DigraphForwardEdgeIndexIterator<typename T::EdgeType>>;
    {
        a.get_backward_edge_indices(vertex_index)
    } -> std::same_as<DigraphBackwardEdgeIndexIterator<typename T::EdgeType>>;
    {
        a.get_forward_edges(vertex_index)
    } -> std::same_as<DigraphForwardEdgeIterator<typename T::EdgeType>>;
    {
        a.get_backward_edges(vertex_index)
    } -> std::same_as<DigraphBackwardEdgeIterator<typename T::EdgeType>>;

    {
        a.get_num_vertices()
    } -> std::same_as<size_t>;
    {
        a.get_num_edges()
    } -> std::same_as<size_t>;
    {
        a.is_directed()
    } -> std::same_as<bool>;
    {
        a.get_edges()
    } -> std::same_as<const std::vector<typename T::EdgeType>&>;
};
}

#endif
