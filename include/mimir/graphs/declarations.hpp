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

#ifndef MIMIR_GRAPHS_DECLARATIONS_HPP_
#define MIMIR_GRAPHS_DECLARATIONS_HPP_

#include "mimir/common/declarations.hpp"

#include <cstdint>
#include <loki/loki.hpp>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mimir::graphs
{
using VertexIndex = uint32_t;
using VertexIndexList = std::vector<VertexIndex>;
using VertexIndexSet = std::unordered_set<VertexIndex>;

using EdgeIndex = uint32_t;
using EdgeIndexList = std::vector<EdgeIndex>;
using EdgeIndexSet = std::unordered_set<EdgeIndex>;

using Degree = uint32_t;
using DegreeList = std::vector<Degree>;
using DegreeMap = std::unordered_map<VertexIndex, Degree>;

using ColorIndex = uint32_t;
using ColorIndexList = std::vector<ColorIndex>;
template<size_t K>
using ColorIndexArray = std::array<ColorIndex, K>;
template<size_t K>
using ColorIndexArrayList = std::vector<ColorIndexArray<K>>;
using ColorIndexSet = std::unordered_set<ColorIndex>;
template<typename T>
using ColorIndexMap = std::unordered_map<ColorIndex, T>;

struct BackwardTag;

/// @brief `ForwardTag` represents traversal of edges in forward direction.
struct ForwardTag
{
    using Inverse = BackwardTag;
};
/// @brief `BackwardTag` represents traversal of edges in backward direction.
struct BackwardTag
{
    using Inverse = ForwardTag;
};

template<class T>
concept Property = requires(const T& a, const T& b, std::ostream& os) {
    { loki::Hash<T> {}(a) } -> std::convertible_to<std::size_t>;
    { loki::EqualTo<T> {}(a, b) } -> std::same_as<bool>;
    // { os << a } -> std::same_as<std::ostream&>;
};

class IProperty;
template<Property... Ps>
class Properties;
class PropertyValue;

template<typename T>
concept IsVertex = requires(T a) {
    typename T::PropertyType;

    { a.get_index() } -> std::convertible_to<VertexIndex>;
};

template<typename T>
concept IsDirection = std::same_as<T, ForwardTag> || std::same_as<T, BackwardTag>;

/// Check whether `T` is a vertex with the given properties `P`.
template<typename T, typename P>
concept HasVertexProperty = IsVertex<T> && std::is_same_v<typename T::PropertyType, std::decay_t<P>>;

template<typename T>
concept IsEdge = requires(T a) {
    typename T::PropertyType;

    { a.get_index() } -> std::convertible_to<EdgeIndex>;
    { a.get_source() } -> std::convertible_to<VertexIndex>;
    { a.get_target() } -> std::convertible_to<VertexIndex>;
};

/// Check whether `T` is an Edge with the given properties `P`.
template<typename T, typename P>
concept HasEdgeProperty = IsEdge<T> && std::is_same_v<typename T::PropertyType, std::decay_t<P>>;

/**
 * A collection of graph concepts required by many graph algorithms.
 */

template<typename T>
concept IsVertexListGraph = requires(T a, VertexIndex vertex) {
    typename T::VertexIndexConstIterator;

    { a.get_vertex_indices() } -> IsRangeOver<VertexIndex>;
    { a.get_num_vertices() } -> std::same_as<size_t>;
};

template<typename T>
concept IsIncidenceGraph = requires(T a, VertexIndex vertex, EdgeIndex edge) {
    { a.template get_source<ForwardTag>(edge) } -> std::same_as<VertexIndex>;
    { a.template get_target<ForwardTag>(edge) } -> std::same_as<VertexIndex>;
    { a.template get_adjacent_edge_indices<ForwardTag>(vertex) } -> IsRangeOver<EdgeIndex>;
    { a.template get_degree<ForwardTag>(vertex) } -> std::same_as<Degree>;
};

template<typename T>
concept IsEdgeListGraph = requires(T a) {
    typename T::EdgeIndexConstIterator;

    { a.get_edge_indices() } -> IsRangeOver<EdgeIndex>;
    { a.get_num_edges() } -> std::same_as<size_t>;
};

template<typename T>
concept IsAdjacencyGraph = requires(T a, VertexIndex vertex) {
    { a.template get_adjacent_vertex_indices<ForwardTag>(vertex) } -> IsRangeOver<VertexIndex>;
    { a.template get_adjacent_vertex_indices<BackwardTag>(vertex) } -> IsRangeOver<VertexIndex>;
};

template<typename T>
concept IsBidirectionalGraph = requires(T a, VertexIndex vertex, EdgeIndex edge) {
    { a.template get_source<ForwardTag>(edge) } -> std::same_as<VertexIndex>;
    { a.template get_source<BackwardTag>(edge) } -> std::same_as<VertexIndex>;
    { a.template get_target<ForwardTag>(edge) } -> std::same_as<VertexIndex>;
    { a.template get_target<BackwardTag>(edge) } -> std::same_as<VertexIndex>;
    { a.template get_adjacent_edge_indices<ForwardTag>(vertex) } -> IsRangeOver<EdgeIndex>;
    { a.template get_adjacent_edge_indices<BackwardTag>(vertex) } -> IsRangeOver<EdgeIndex>;
    { a.template get_degree<ForwardTag>(vertex) } -> std::same_as<Degree>;
    { a.template get_degree<BackwardTag>(vertex) } -> std::same_as<Degree>;
};

/**
 * Static graphs do not allow for deletion of vertices and edges, allowing vector data structures.
 */

template<typename T>
concept IsStaticGraph = requires(T a) {
    requires IsVertexListGraph<T>;
    requires IsEdgeListGraph<T>;
    requires IsIncidenceGraph<T>;
    requires IsAdjacencyGraph<T>;
    requires IsBidirectionalGraph<T>;

    // Ensure that StaticGraph uses std::vector to store vertices and edges.
    { a.get_vertices() } -> std::same_as<const std::vector<typename T::VertexType>&>;
    { a.get_edges() } -> std::same_as<const std::vector<typename T::EdgeType>&>;
};

/**
 * Dynamic graphs allow for deletion of vertices and edges, requiring hash map data structures.
 */

template<typename T>
concept IsDynamicGraph = requires(T a, VertexIndex vertex, EdgeIndex edge) {
    requires IsVertexListGraph<T>;
    requires IsEdgeListGraph<T>;
    requires IsIncidenceGraph<T>;
    requires IsAdjacencyGraph<T>;
    requires IsBidirectionalGraph<T>;

    // Ensure that DynamicGraph uses std::unordered_map to store vertices and edges.
    { a.get_vertices() } -> std::same_as<const std::unordered_map<VertexIndex, typename T::VertexType>&>;
    { a.get_edges() } -> std::same_as<const std::unordered_map<EdgeIndex, typename T::EdgeType>&>;

    // Ensure that DynamicGraph supports deletion of vertices and edges
    { a.remove_vertex(vertex) };
    { a.remove_edge(edge) };
};

template<Property P = std::tuple<>>
class Vertex;

template<Property P = std::tuple<>>
class Edge;

template<IsVertex V, IsEdge E>
class StaticGraph;

template<IsStaticGraph G>
class StaticForwardGraph;

template<IsStaticGraph G>
class StaticBidirectionalGraph;

template<IsVertex V, IsEdge E>
class DynamicGraph;

namespace nauty
{
class SparseGraph;
}

namespace color_refinement
{
class CertificateImpl;
}

namespace kfwl
{
template<size_t K>
class CertificateImpl;
}
}

#endif
