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

#include "mimir/graphs/graph_edge_interface.hpp"
#include "mimir/graphs/graph_traversal_interface.hpp"
#include "mimir/graphs/static_graph_iterators.hpp"

#include <concepts>
#include <cstdint>
#include <ranges>
#include <span>
#include <vector>

namespace mimir
{
template<typename T, typename Value>
concept IsRangeOver = std::ranges::range<T> && std::same_as<std::ranges::range_value_t<T>, Value>;

template<typename T>
concept IsGraph = requires(T a, VertexIndex vertex)
{
    typename T::GraphType;
    typename T::VertexType;
    requires IsVertex<typename T::VertexType>;
    typename T::EdgeType;
    requires IsEdge<typename T::EdgeType>;

    typename T::VertexIndexConstIteratorType;
    typename T::EdgeIndexConstIteratorType;

    typename T::template AdjacentVertexConstIteratorType<ForwardTraversal>;
    typename T::template AdjacentVertexConstIteratorType<BackwardTraversal>;
    typename T::template AdjacentVertexIndexConstIteratorType<ForwardTraversal>;
    typename T::template AdjacentVertexIndexConstIteratorType<BackwardTraversal>;
    typename T::template AdjacentEdgeConstIteratorType<ForwardTraversal>;
    typename T::template AdjacentEdgeConstIteratorType<BackwardTraversal>;
    typename T::template AdjacentEdgeIndexConstIteratorType<ForwardTraversal>;
    typename T::template AdjacentEdgeIndexConstIteratorType<BackwardTraversal>;

    /* Iterators */

    {
        a.get_vertex_indices()
        } -> IsRangeOver<VertexIndex>;
    {
        a.get_edge_indices()
        } -> IsRangeOver<EdgeIndex>;
    {
        a.template get_adjacent_vertices<ForwardTraversal>(vertex)
        } -> IsRangeOver<typename T::VertexType>;
    {
        a.template get_adjacent_vertices<BackwardTraversal>(vertex)
        } -> IsRangeOver<typename T::VertexType>;
    {
        a.template get_adjacent_vertex_indices<ForwardTraversal>(vertex)
        } -> IsRangeOver<VertexIndex>;
    {
        a.template get_adjacent_vertex_indices<BackwardTraversal>(vertex)
        } -> IsRangeOver<VertexIndex>;
    {
        a.template get_adjacent_edges<ForwardTraversal>(vertex)
        } -> IsRangeOver<typename T::EdgeType>;
    {
        a.template get_adjacent_edges<BackwardTraversal>(vertex)
        } -> IsRangeOver<typename T::EdgeType>;
    {
        a.template get_adjacent_edge_indices<ForwardTraversal>(vertex)
        } -> IsRangeOver<EdgeIndex>;
    {
        a.template get_adjacent_edge_indices<BackwardTraversal>(vertex)
        } -> IsRangeOver<EdgeIndex>;

    /* Getters */

    {
        a.get_vertices()
        } -> std::same_as<const std::vector<typename T::VertexType>&>;
    {
        a.get_edges()
        } -> std::same_as<const std::vector<typename T::EdgeType>&>;
    {
        a.get_num_vertices()
        } -> std::same_as<size_t>;
    {
        a.get_num_edges()
        } -> std::same_as<size_t>;
    {
        a.template get_degrees<ForwardTraversal>()
        } -> std::same_as<const DegreeList&>;
    {
        a.template get_degrees<BackwardTraversal>()
        } -> std::same_as<const DegreeList&>;
    {
        a.template get_degree<ForwardTraversal>(vertex)
        } -> std::same_as<Degree>;
    {
        a.template get_degree<BackwardTraversal>(vertex)
        } -> std::same_as<Degree>;
};

template<typename T>
concept IsConstructibleGraph = requires(T a, VertexIndex vertex)
{
    {
        a.add_vertex()
        } -> std::same_as<VertexIndex>;
    {
        a.add_directed_edge(vertex, vertex)
        } -> std::same_as<EdgeIndex>;
    {
        a.add_undirected_edge(vertex, vertex)
        } -> std::same_as<std::pair<EdgeIndex, EdgeIndex>>;
};

template<typename T>
concept IsDestructibleGraph = requires(T a, VertexIndex vertex, EdgeIndex edge)
{
    { a.remove_vertex(vertex) };
    { a.remove_edge(edge) };
};

/**
 * Static graphs require a contiguous indexing scheme, allowing us to use efficient vector data structures.
 * For dynamic graphs, we must use less efficient data structures.
 */

struct StaticGraphTag
{
};
struct DynamicGraphTag
{
};

template<typename T>
concept IsStaticGraph = requires(T a)
{
    typename T::GraphType;
    requires std::same_as<typename T::GraphType, StaticGraphTag>;
};

template<typename T>
concept IsDynamicGraph = requires(T a)
{
    typename T::GraphType;
    requires std::same_as<typename T::GraphType, DynamicGraphTag>;
};

}

#endif
