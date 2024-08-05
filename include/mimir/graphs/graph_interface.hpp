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

#ifndef MIMIR_GRAPHS_GRAPH_INTERFACE_HPP_
#define MIMIR_GRAPHS_GRAPH_INTERFACE_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/graphs/graph_edge_interface.hpp"
#include "mimir/graphs/graph_traversal_interface.hpp"

#include <concepts>
#include <cstdint>
#include <ranges>
#include <span>
#include <vector>

namespace mimir
{

/**
 * A collection of graph concepts required by many graph algorithms.
 */

template<typename T>
concept IsVertexListGraph = requires(T a, VertexIndex vertex)
{
    typename T::VertexIndexConstIteratorType;

    {
        a.get_vertex_indices()
        } -> IsRangeOver<VertexIndex>;
    {
        a.get_num_vertices()
        } -> std::same_as<size_t>;
};

template<typename T>
concept IsIncidenceGraph = requires(T a, VertexIndex vertex, EdgeIndex edge)
{
    typename T::template AdjacentEdgeIndexConstIteratorType<ForwardTraversal>;
    typename T::template AdjacentEdgeIndexConstIteratorType<BackwardTraversal>;

    {
        a.template get_source<ForwardTraversal>(edge)
        } -> std::same_as<VertexIndex>;
    {
        a.template get_source<BackwardTraversal>(edge)
        } -> std::same_as<VertexIndex>;
    {
        a.template get_target<ForwardTraversal>(edge)
        } -> std::same_as<VertexIndex>;
    {
        a.template get_target<BackwardTraversal>(edge)
        } -> std::same_as<VertexIndex>;
    {
        a.template get_adjacent_edge_indices<ForwardTraversal>(vertex)
        } -> IsRangeOver<EdgeIndex>;
    {
        a.template get_adjacent_edge_indices<BackwardTraversal>(vertex)
        } -> IsRangeOver<EdgeIndex>;
    {
        a.template get_degree<ForwardTraversal>(vertex)
        } -> std::same_as<Degree>;
    {
        a.template get_degree<BackwardTraversal>(vertex)
        } -> std::same_as<Degree>;
};

template<typename T>
concept IsEdgeListGraph = requires(T a)
{
    typename T::EdgeIndexConstIteratorType;

    {
        a.get_edge_indices()
        } -> IsRangeOver<EdgeIndex>;
    {
        a.get_num_edges()
        } -> std::same_as<size_t>;
};

template<typename T>
concept IsAdjacencyGraph = requires(T a, VertexIndex vertex)
{
    typename T::template AdjacentVertexIndexConstIteratorType<ForwardTraversal>;
    typename T::template AdjacentVertexIndexConstIteratorType<BackwardTraversal>;

    {
        a.template get_adjacent_vertex_indices<ForwardTraversal>(vertex)
        } -> IsRangeOver<VertexIndex>;
    {
        a.template get_adjacent_vertex_indices<BackwardTraversal>(vertex)
        } -> IsRangeOver<VertexIndex>;
};

}

#endif
