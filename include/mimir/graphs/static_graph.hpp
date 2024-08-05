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

#ifndef MIMIR_GRAPHS_STATIC_GRAPH_HPP_
#define MIMIR_GRAPHS_STATIC_GRAPH_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/common/grouped_vector.hpp"
#include "mimir/graphs/graph_edge_interface.hpp"
#include "mimir/graphs/graph_vertex_interface.hpp"
#include "mimir/graphs/static_graph_interface.hpp"
#include "mimir/graphs/static_graph_iterators.hpp"

#include <ranges>
#include <span>
#include <vector>

namespace mimir
{

/**
 * Declarations
 */

/* StaticGraph */

/// @brief A `StaticGraph` G consists of vertices V and edges E that have contiguous indexing schemes,
/// allowing for using efficient vector data structure.
///
/// A `StaticGraph` provides functionality for adding vertices and edges, and iterating over vertices and edges.
/// Iterating over the outgoing or incoming edges, or the ingoing or outgoing adjacent vertices requires time O(|E|).
///
/// A `StaticGraph` can be translated into an equivalent StaticForwardGraph or StaticBidirectional in O(|V|+|E|*Log2(|E|))
/// to iterate more efficiently over the outgoing or incoming edges, or the ingoing or outgoing adjacent vertices.
/// @tparam Vertex is vertex type.
/// @tparam Edge is the edge type.
template<IsVertex Vertex, IsEdge Edge>
class StaticGraph
{
public:
    using GraphType = StaticGraphTag;
    using VertexType = Vertex;
    using VertexList = std::vector<Vertex>;
    using EdgeType = Edge;
    using EdgeList = std::vector<Edge>;

    using VertexIndexConstIteratorType = VertexIndexConstIterator<Vertex>;
    using EdgeIndexConstIteratorType = EdgeIndexConstIterator<Edge>;

    template<IsTraversalDirection Direction>
    using AdjacentVertexConstIteratorType = AdjacentVertexConstIterator<Vertex, Edge, Direction>;
    template<IsTraversalDirection Direction>
    using AdjacentVertexIndexConstIteratorType = AdjacentVertexIndexConstIterator<Edge, Direction>;
    template<IsTraversalDirection Direction>
    using AdjacentEdgeConstIteratorType = AdjacentEdgeConstIterator<Edge, Direction>;
    template<IsTraversalDirection Direction>
    using AdjacentEdgeIndexConstIteratorType = AdjacentEdgeIndexConstIterator<Edge, Direction>;

    StaticGraph();

    /// @brief Reinitialize the graph to an empty graph.
    void reset();

    /**
     * Constructible functionality.
     */

    template<typename... Args>
    VertexIndex add_vertex(Args&&... args);

    template<typename... Args>
    EdgeIndex add_directed_edge(VertexIndex source, VertexIndex target, Args&&... args);

    template<typename... Args>
    std::pair<EdgeIndex, EdgeIndex> add_undirected_edge(VertexIndex source, VertexIndex target, Args&&... args);

    /**
     * Iterators
     */

    std::ranges::subrange<VertexIndexConstIteratorType> get_vertex_indices() const;
    std::ranges::subrange<EdgeIndexConstIteratorType> get_edge_indices() const;

    template<IsTraversalDirection Direction>
    std::ranges::subrange<AdjacentVertexConstIteratorType<Direction>> get_adjacent_vertices(VertexIndex vertex) const;
    template<IsTraversalDirection Direction>
    std::ranges::subrange<AdjacentVertexIndexConstIteratorType<Direction>> get_adjacent_vertex_indices(VertexIndex vertex) const;
    template<IsTraversalDirection Direction>
    std::ranges::subrange<AdjacentEdgeConstIteratorType<Direction>> get_adjacent_edges(VertexIndex vertex) const;
    template<IsTraversalDirection Direction>
    std::ranges::subrange<AdjacentEdgeIndexConstIteratorType<Direction>> get_adjacent_edge_indices(VertexIndex vertex) const;

    /**
     * Getters
     */

    const VertexList& get_vertices() const;
    const EdgeList& get_edges() const;
    size_t get_num_vertices() const;
    size_t get_num_edges() const;

    template<IsTraversalDirection>
    VertexIndex get_source(EdgeIndex edge) const;
    template<IsTraversalDirection>
    VertexIndex get_target(EdgeIndex edge) const;
    template<IsTraversalDirection Direction>
    const DegreeList& get_degrees() const;
    template<IsTraversalDirection Direction>
    Degree get_degree(VertexIndex vertex) const;

private:
    VertexList m_vertices;
    EdgeList m_edges;

    DegreeList m_in_degrees;
    DegreeList m_out_degrees;

    // Slice over all edges for using the iterators.
    EdgeIndexList m_slice;
};

/* StaticForwardGraph */

/// @brief StaticForwardGraph is a translated StaticGraph for more efficient forward traversal.
template<IsStaticGraph G>
class StaticForwardGraph
{
public:
    using GraphType = typename G::GraphType;
    using VertexType = typename G::VertexType;
    using EdgeType = typename G::EdgeType;
    using VertexList = std::vector<VertexType>;
    using EdgeList = std::vector<EdgeType>;

    using VertexIndexConstIteratorType = VertexIndexConstIterator<VertexType>;
    using EdgeIndexConstIteratorType = EdgeIndexConstIterator<EdgeType>;

    template<IsTraversalDirection Direction>
    using AdjacentVertexConstIteratorType = AdjacentVertexConstIterator<VertexType, EdgeType, Direction>;
    template<IsTraversalDirection Direction>
    using AdjacentVertexIndexConstIteratorType = AdjacentVertexIndexConstIterator<EdgeType, Direction>;
    template<IsTraversalDirection Direction>
    using AdjacentEdgeConstIteratorType = AdjacentEdgeConstIterator<EdgeType, Direction>;
    template<IsTraversalDirection Direction>
    using AdjacentEdgeIndexConstIteratorType = AdjacentEdgeIndexConstIterator<EdgeType, Direction>;

    explicit StaticForwardGraph(G graph);

    /**
     * Iterators
     */

    std::ranges::subrange<VertexIndexConstIteratorType> get_vertex_indices() const;
    std::ranges::subrange<EdgeIndexConstIteratorType> get_edge_indices() const;

    template<IsTraversalDirection Direction>
    std::ranges::subrange<AdjacentVertexConstIteratorType<Direction>> get_adjacent_vertices(VertexIndex vertex) const;
    template<IsTraversalDirection Direction>
    std::ranges::subrange<AdjacentVertexIndexConstIteratorType<Direction>> get_adjacent_vertex_indices(VertexIndex vertex) const;
    template<IsTraversalDirection Direction>
    std::ranges::subrange<AdjacentEdgeConstIteratorType<Direction>> get_adjacent_edges(VertexIndex vertex) const;
    template<IsTraversalDirection Direction>
    std::ranges::subrange<AdjacentEdgeIndexConstIteratorType<Direction>> get_adjacent_edge_indices(VertexIndex vertex) const;

    /**
     * Getters
     */

    const VertexList& get_vertices() const;
    const EdgeList& get_edges() const;
    size_t get_num_vertices() const;
    size_t get_num_edges() const;

    template<IsTraversalDirection>
    VertexIndex get_source(EdgeIndex edge) const;
    template<IsTraversalDirection>
    VertexIndex get_target(EdgeIndex edge) const;
    template<IsTraversalDirection Direction>
    const DegreeList& get_degrees() const;
    template<IsTraversalDirection Direction>
    Degree get_degree(VertexIndex vertex) const;

private:
    G m_graph;

    IndexGroupedVector<const EdgeIndex> m_edge_indices_grouped_by_source;
};

/* BidirectionalGraph */

/// @brief BidirectionalGraph is a translated StaticGraph for more efficient bidirectional traversal.
template<IsStaticGraph G>
class StaticBidirectionalGraph
{
public:
    using GraphType = typename G::GraphType;
    using VertexType = typename G::VertexType;
    using EdgeType = typename G::EdgeType;
    using VertexList = std::vector<VertexType>;
    using EdgeList = std::vector<EdgeType>;

    using VertexIndexConstIteratorType = VertexIndexConstIterator<VertexType>;
    using EdgeIndexConstIteratorType = EdgeIndexConstIterator<EdgeType>;

    template<IsTraversalDirection Direction>
    using AdjacentVertexConstIteratorType = AdjacentVertexConstIterator<VertexType, EdgeType, Direction>;
    template<IsTraversalDirection Direction>
    using AdjacentVertexIndexConstIteratorType = AdjacentVertexIndexConstIterator<EdgeType, Direction>;
    template<IsTraversalDirection Direction>
    using AdjacentEdgeConstIteratorType = AdjacentEdgeConstIterator<EdgeType, Direction>;
    template<IsTraversalDirection Direction>
    using AdjacentEdgeIndexConstIteratorType = AdjacentEdgeIndexConstIterator<EdgeType, Direction>;

    explicit StaticBidirectionalGraph(G graph);

    /**
     * Iterators
     */

    std::ranges::subrange<VertexIndexConstIteratorType> get_vertex_indices() const;
    std::ranges::subrange<EdgeIndexConstIteratorType> get_edge_indices() const;

    template<IsTraversalDirection Direction>
    std::ranges::subrange<AdjacentVertexConstIteratorType<Direction>> get_adjacent_vertices(VertexIndex vertex) const;
    template<IsTraversalDirection Direction>
    std::ranges::subrange<AdjacentVertexIndexConstIteratorType<Direction>> get_adjacent_vertex_indices(VertexIndex vertex) const;
    template<IsTraversalDirection Direction>
    std::ranges::subrange<AdjacentEdgeConstIteratorType<Direction>> get_adjacent_edges(VertexIndex vertex) const;
    template<IsTraversalDirection Direction>
    std::ranges::subrange<AdjacentEdgeIndexConstIteratorType<Direction>> get_adjacent_edge_indices(VertexIndex vertex) const;

    /**
     * Getters
     */

    const VertexList& get_vertices() const;
    const EdgeList& get_edges() const;
    size_t get_num_vertices() const;
    size_t get_num_edges() const;

    template<IsTraversalDirection>
    VertexIndex get_source(EdgeIndex edge) const;
    template<IsTraversalDirection>
    VertexIndex get_target(EdgeIndex edge) const;
    template<IsTraversalDirection Direction>
    const DegreeList& get_degrees() const;
    template<IsTraversalDirection Direction>
    Degree get_degree(VertexIndex vertex) const;

private:
    G m_graph;

    IndexGroupedVector<const EdgeIndex> m_edge_indices_grouped_by_source;
    IndexGroupedVector<const EdgeIndex> m_edge_indices_grouped_by_target;
};

/**
 * Implementations
 */

/* StaticGraph */

template<IsVertex Vertex, IsEdge Edge>
StaticGraph<Vertex, Edge>::StaticGraph() : m_vertices(), m_edges(), m_in_degrees(), m_out_degrees()
{
}

template<IsVertex Vertex, IsEdge Edge>
template<typename... Args>
VertexIndex StaticGraph<Vertex, Edge>::add_vertex(Args&&... args)
{
    const auto index = m_vertices.size();
    m_vertices.emplace_back(index, std::forward<Args>(args)...);
    m_in_degrees.resize(index + 1, 0);
    m_out_degrees.resize(index + 1, 0);
    return index;
}

template<IsVertex Vertex, IsEdge Edge>
template<typename... Args>
EdgeIndex StaticGraph<Vertex, Edge>::add_directed_edge(VertexIndex source, VertexIndex target, Args&&... args)
{
    if (source >= get_num_vertices() || target >= get_num_vertices() || source < 0 || target < 0)
    {
        throw std::out_of_range("StaticGraph<Vertex, Edge>::add_directed_edge(...): Source or destination vertex out of range");
    }
    const auto index = m_edges.size();
    m_edges.emplace_back(index, source, target, std::forward<Args>(args)...);
    ++m_in_degrees.at(target);
    ++m_out_degrees.at(source);
    m_slice.push_back(index);
    return index;
}

template<IsVertex Vertex, IsEdge Edge>
template<typename... Args>
std::pair<EdgeIndex, EdgeIndex> StaticGraph<Vertex, Edge>::add_undirected_edge(VertexIndex source, VertexIndex target, Args&&... args)
{
    if (source >= get_num_vertices() || target >= get_num_vertices() || source < 0 || target < 0)
    {
        throw std::out_of_range("StaticGraph<Vertex, Edge>::add_undirected_edge(...): Source or destination vertex out of range");
    }
    // Need to copy args to keep them in valid state.
    const auto forward_edge_index = add_directed_edge(source, target, args...);
    const auto backward_edge_index = add_directed_edge(target, source, std::forward<Args>(args)...);
    ++m_in_degrees.at(source);
    ++m_in_degrees.at(target);
    ++m_out_degrees.at(source);
    ++m_out_degrees.at(target);
    m_slice.push_back(forward_edge_index);
    m_slice.push_back(backward_edge_index);
    return std::make_pair(forward_edge_index, backward_edge_index);
}

template<IsVertex Vertex, IsEdge Edge>
void StaticGraph<Vertex, Edge>::reset()
{
    m_vertices.clear();
    m_edges.clear();
    m_in_degrees.clear();
    m_out_degrees.clear();
    m_slice.clear();
}

template<IsVertex Vertex, IsEdge Edge>
std::ranges::subrange<typename StaticGraph<Vertex, Edge>::VertexIndexConstIteratorType> StaticGraph<Vertex, Edge>::get_vertex_indices() const
{
    return std::ranges::subrange(typename StaticGraph<Vertex, Edge>::VertexIndexConstIteratorType(m_vertices, true),
                                 typename StaticGraph<Vertex, Edge>::VertexIndexConstIteratorType(m_vertices, false));
}

template<IsVertex Vertex, IsEdge Edge>
std::ranges::subrange<typename StaticGraph<Vertex, Edge>::EdgeIndexConstIteratorType> StaticGraph<Vertex, Edge>::get_edge_indices() const
{
    return std::ranges::subrange(typename StaticGraph<Vertex, Edge>::EdgeIndexConstIteratorType(m_edges, true),
                                 typename StaticGraph<Vertex, Edge>::EdgeIndexConstIteratorType(m_edges, false));
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
std::ranges::subrange<typename StaticGraph<Vertex, Edge>::template AdjacentVertexConstIteratorType<Direction>>
StaticGraph<Vertex, Edge>::get_adjacent_vertices(VertexIndex vertex) const
{
    return std::ranges::subrange(typename StaticGraph<Vertex, Edge>::AdjacentVertexConstIteratorType<Direction>(vertex, m_vertices, m_edges, m_slice, true),
                                 typename StaticGraph<Vertex, Edge>::AdjacentVertexConstIteratorType<Direction>(vertex, m_vertices, m_edges, m_slice, false));
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
std::ranges::subrange<typename StaticGraph<Vertex, Edge>::template AdjacentVertexIndexConstIteratorType<Direction>>
StaticGraph<Vertex, Edge>::get_adjacent_vertex_indices(VertexIndex vertex) const
{
    return std::ranges::subrange(typename StaticGraph<Vertex, Edge>::AdjacentVertexIndexConstIteratorType<Direction>(vertex, m_edges, m_slice, true),
                                 typename StaticGraph<Vertex, Edge>::AdjacentVertexIndexConstIteratorType<Direction>(vertex, m_edges, m_slice, false));
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
std::ranges::subrange<typename StaticGraph<Vertex, Edge>::template AdjacentEdgeConstIteratorType<Direction>>
StaticGraph<Vertex, Edge>::get_adjacent_edges(VertexIndex vertex) const
{
    return std::ranges::subrange(typename StaticGraph<Vertex, Edge>::AdjacentEdgeConstIteratorType<Direction>(vertex, m_edges, m_slice, true),
                                 typename StaticGraph<Vertex, Edge>::AdjacentEdgeConstIteratorType<Direction>(vertex, m_edges, m_slice, false));
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
std::ranges::subrange<typename StaticGraph<Vertex, Edge>::template AdjacentEdgeIndexConstIteratorType<Direction>>
StaticGraph<Vertex, Edge>::get_adjacent_edge_indices(VertexIndex vertex) const
{
    return std::ranges::subrange(typename StaticGraph<Vertex, Edge>::AdjacentEdgeIndexConstIteratorType<Direction>(vertex, m_edges, m_slice, true),
                                 typename StaticGraph<Vertex, Edge>::AdjacentEdgeIndexConstIteratorType<Direction>(vertex, m_edges, m_slice, false));
}

template<IsVertex Vertex, IsEdge Edge>
const StaticGraph<Vertex, Edge>::VertexList& StaticGraph<Vertex, Edge>::get_vertices() const
{
    return m_vertices;
}

template<IsVertex Vertex, IsEdge Edge>
const StaticGraph<Vertex, Edge>::EdgeList& StaticGraph<Vertex, Edge>::get_edges() const
{
    return m_edges;
}

template<IsVertex Vertex, IsEdge Edge>
size_t StaticGraph<Vertex, Edge>::get_num_vertices() const
{
    return m_vertices.size();
}

template<IsVertex Vertex, IsEdge Edge>
size_t StaticGraph<Vertex, Edge>::get_num_edges() const
{
    return m_edges.size();
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
VertexIndex StaticGraph<Vertex, Edge>::get_source(EdgeIndex edge) const
{
    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        return m_edges.at(edge).get_source();
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        return m_edges.at(edge).get_target();
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "StaticGraph<Vertex, Edge>::get_source(...): Missing implementation for IsTraversalDirection.");
    }
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
VertexIndex StaticGraph<Vertex, Edge>::get_target(EdgeIndex edge) const
{
    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        return m_edges.at(edge).get_target();
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        return m_edges.at(edge).get_source();
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "StaticGraph<Vertex, Edge>::get_target(...): Missing implementation for IsTraversalDirection.");
    }
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
const DegreeList& StaticGraph<Vertex, Edge>::get_degrees() const
{
    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        return m_out_degrees;
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        return m_in_degrees;
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "StaticGraph<Vertex, Edge>::get_degrees(): Missing implementation for IsTraversalDirection.");
    }
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
Degree StaticGraph<Vertex, Edge>::get_degree(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        return m_out_degrees.at(vertex);
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        return m_in_degrees.at(vertex);
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "StaticGraph<Vertex, Edge>::get_degree(...): Missing implementation for IsTraversalDirection.");
    }
}

/* StaticForwardGraph */

/// @brief Groups edge indices by source or target, depending on forward is true or false.
/// @tparam Vertex is the type of vertices in the graph.
/// @tparam Edge is the type of edges in the graph.
/// @param graph is the graph.
/// @param forward true will group by source and false will group by target.
/// @return
template<IsVertex Vertex, IsEdge Edge>
static IndexGroupedVector<const EdgeIndex> compute_index_grouped_edge_indices(const StaticGraph<Vertex, Edge>& graph, bool forward)
{
    auto grouped_by_source_data = std::vector<std::pair<VertexIndex, EdgeIndex>> {};
    for (const auto& edge : graph.get_edges())
    {
        grouped_by_source_data.emplace_back(((forward) ? edge.get_source() : edge.get_target()), edge.get_index());
    }

    std::sort(grouped_by_source_data.begin(), grouped_by_source_data.end(), [](const auto& l, const auto& r) { return l.first < r.first; });

    auto index_grouped_by_source = IndexGroupedVector<std::pair<VertexIndex, EdgeIndex>>::create(
        std::move(grouped_by_source_data),
        [](const auto& l, const auto& r) { return l.first != r.first; },
        [](const auto& e) { return e.first; },
        graph.get_num_vertices());

    auto edge_index_grouped_by_source_builder = IndexGroupedVectorBuilder<const EdgeIndex>();
    for (const auto& group : index_grouped_by_source)
    {
        edge_index_grouped_by_source_builder.start_group();

        for (const auto& element : group)
        {
            edge_index_grouped_by_source_builder.add_group_element(element.second);
        }
    }
    return edge_index_grouped_by_source_builder.get_result();
}

template<IsStaticGraph G>
StaticForwardGraph<G>::StaticForwardGraph(G graph) :
    m_graph(std::move(graph)),
    m_edge_indices_grouped_by_source(compute_index_grouped_edge_indices(m_graph, true))
{
}

template<IsStaticGraph G>
std::ranges::subrange<typename StaticForwardGraph<G>::VertexIndexConstIteratorType> StaticForwardGraph<G>::get_vertex_indices() const
{
    return m_graph.get_vertex_indices();
}

template<IsStaticGraph G>
std::ranges::subrange<typename StaticForwardGraph<G>::EdgeIndexConstIteratorType> StaticForwardGraph<G>::get_edge_indices() const
{
    return m_graph.get_edge_indices();
}

template<IsStaticGraph G>
template<IsTraversalDirection Direction>
std::ranges::subrange<typename StaticForwardGraph<G>::template AdjacentVertexConstIteratorType<Direction>>
StaticForwardGraph<G>::get_adjacent_vertices(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        return std::ranges::subrange(
            typename StaticForwardGraph<G>::AdjacentVertexConstIteratorType<ForwardTraversal>(vertex,
                                                                                              m_graph.get_vertices(),
                                                                                              m_graph.get_edges(),
                                                                                              m_edge_indices_grouped_by_source.at(vertex),
                                                                                              true),
            typename StaticForwardGraph<G>::AdjacentVertexConstIteratorType<ForwardTraversal>(vertex,
                                                                                              m_graph.get_vertices(),
                                                                                              m_graph.get_edges(),
                                                                                              m_edge_indices_grouped_by_source.at(vertex),
                                                                                              false));
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        return m_graph.template get_adjacent_vertices<BackwardTraversal>(vertex);
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "StaticForwardGraph<G>::get_adjacent_vertices: Missing implementation for IsTraversalDirection.");
    }
}

template<IsStaticGraph G>
template<IsTraversalDirection Direction>
std::ranges::subrange<typename StaticForwardGraph<G>::template AdjacentVertexIndexConstIteratorType<Direction>>
StaticForwardGraph<G>::get_adjacent_vertex_indices(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        return std::ranges::subrange(
            typename StaticForwardGraph<G>::AdjacentVertexIndexConstIteratorType<ForwardTraversal>(vertex,
                                                                                                   m_graph.get_edges(),
                                                                                                   m_edge_indices_grouped_by_source.at(vertex),
                                                                                                   true),
            typename StaticForwardGraph<G>::AdjacentVertexIndexConstIteratorType<ForwardTraversal>(vertex,
                                                                                                   m_graph.get_edges(),
                                                                                                   m_edge_indices_grouped_by_source.at(vertex),
                                                                                                   false));
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        return m_graph.template get_adjacent_vertex_indices<BackwardTraversal>(vertex);
    }
    else
    {
        static_assert(dependent_false<Direction>::value,
                      "StaticForwardGraph<G>::get_adjacent_vertex_indices: Missing implementation for IsTraversalDirection.");
    }
}

template<IsStaticGraph G>
template<IsTraversalDirection Direction>
std::ranges::subrange<typename StaticForwardGraph<G>::template AdjacentEdgeConstIteratorType<Direction>>
StaticForwardGraph<G>::get_adjacent_edges(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        return std::ranges::subrange(
            typename StaticForwardGraph<G>::AdjacentEdgeConstIteratorType<ForwardTraversal>(vertex,
                                                                                            m_graph.get_edges(),
                                                                                            m_edge_indices_grouped_by_source.at(vertex),
                                                                                            true),
            typename StaticForwardGraph<G>::AdjacentEdgeConstIteratorType<ForwardTraversal>(vertex,
                                                                                            m_graph.get_edges(),
                                                                                            m_edge_indices_grouped_by_source.at(vertex),
                                                                                            false));
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        return m_graph.template get_adjacent_edges<BackwardTraversal>(vertex);
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "StaticForwardGraph<G>::get_adjacent_edges: Missing implementation for IsTraversalDirection.");
    }
}

template<IsStaticGraph G>
template<IsTraversalDirection Direction>
std::ranges::subrange<typename StaticForwardGraph<G>::template AdjacentEdgeIndexConstIteratorType<Direction>>
StaticForwardGraph<G>::get_adjacent_edge_indices(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        return std::ranges::subrange(
            typename StaticForwardGraph<G>::AdjacentEdgeIndexConstIteratorType<ForwardTraversal>(vertex,
                                                                                                 m_graph.get_edges(),
                                                                                                 m_edge_indices_grouped_by_source.at(vertex),
                                                                                                 true),
            typename StaticForwardGraph<G>::AdjacentEdgeIndexConstIteratorType<ForwardTraversal>(vertex,
                                                                                                 m_graph.get_edges(),
                                                                                                 m_edge_indices_grouped_by_source.at(vertex),
                                                                                                 false));
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        return m_graph.template get_adjacent_edge_indices<BackwardTraversal>(vertex);
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "StaticForwardGraph<G>::get_adjacent_edge_indices: Missing implementation for IsTraversalDirection.");
    }
}

template<IsStaticGraph G>
const StaticForwardGraph<G>::VertexList& StaticForwardGraph<G>::get_vertices() const
{
    return m_graph.get_vertices();
}

template<IsStaticGraph G>
const StaticForwardGraph<G>::EdgeList& StaticForwardGraph<G>::get_edges() const
{
    return m_graph.get_edges();
}

template<IsStaticGraph G>
size_t StaticForwardGraph<G>::get_num_vertices() const
{
    return m_graph.get_num_vertices();
}

template<IsStaticGraph G>
size_t StaticForwardGraph<G>::get_num_edges() const
{
    return m_graph.get_num_edges();
}

template<IsStaticGraph G>
template<IsTraversalDirection Direction>
VertexIndex StaticForwardGraph<G>::get_source(EdgeIndex edge) const
{
    return m_graph.template get_source<Direction>(edge);
}

template<IsStaticGraph G>
template<IsTraversalDirection Direction>
VertexIndex StaticForwardGraph<G>::get_target(EdgeIndex edge) const
{
    return m_graph.template get_target<Direction>(edge);
}

template<IsStaticGraph G>
template<IsTraversalDirection Direction>
const DegreeList& StaticForwardGraph<G>::get_degrees() const
{
    return m_graph.template get_degrees<Direction>();
}

template<IsStaticGraph G>
template<IsTraversalDirection Direction>
Degree StaticForwardGraph<G>::get_degree(VertexIndex vertex) const
{
    return m_graph.template get_degree<Direction>(vertex);
}

/* BidirectionalGraph */

template<IsStaticGraph G>
StaticBidirectionalGraph<G>::StaticBidirectionalGraph(G graph) :
    m_graph(std::move(graph)),
    m_edge_indices_grouped_by_source(compute_index_grouped_edge_indices(m_graph, true)),
    m_edge_indices_grouped_by_target(compute_index_grouped_edge_indices(m_graph, false))
{
}

template<IsStaticGraph G>
std::ranges::subrange<typename StaticBidirectionalGraph<G>::VertexIndexConstIteratorType> StaticBidirectionalGraph<G>::get_vertex_indices() const
{
    return m_graph.get_vertex_indices();
}

template<IsStaticGraph G>
std::ranges::subrange<typename StaticBidirectionalGraph<G>::EdgeIndexConstIteratorType> StaticBidirectionalGraph<G>::get_edge_indices() const
{
    return m_graph.get_edge_indices();
}

template<IsStaticGraph G>
template<IsTraversalDirection Direction>
std::ranges::subrange<typename StaticBidirectionalGraph<G>::template AdjacentVertexConstIteratorType<Direction>>
StaticBidirectionalGraph<G>::get_adjacent_vertices(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        return std::ranges::subrange(
            typename StaticBidirectionalGraph<G>::AdjacentVertexConstIteratorType<ForwardTraversal>(vertex,
                                                                                                    m_graph.get_vertices(),
                                                                                                    m_graph.get_edges(),
                                                                                                    m_edge_indices_grouped_by_source.at(vertex),
                                                                                                    true),
            typename StaticBidirectionalGraph<G>::AdjacentVertexConstIteratorType<ForwardTraversal>(vertex,
                                                                                                    m_graph.get_vertices(),
                                                                                                    m_graph.get_edges(),
                                                                                                    m_edge_indices_grouped_by_source.at(vertex),
                                                                                                    false));
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        return std::ranges::subrange(
            typename StaticBidirectionalGraph<G>::AdjacentVertexConstIteratorType<BackwardTraversal>(vertex,
                                                                                                     m_graph.get_vertices(),
                                                                                                     m_graph.get_edges(),
                                                                                                     m_edge_indices_grouped_by_target.at(vertex),
                                                                                                     true),
            typename StaticBidirectionalGraph<G>::AdjacentVertexConstIteratorType<BackwardTraversal>(vertex,
                                                                                                     m_graph.get_vertices(),
                                                                                                     m_graph.get_edges(),
                                                                                                     m_edge_indices_grouped_by_target.at(vertex),
                                                                                                     false));
    }
    else
    {
        static_assert(dependent_false<Direction>::value,
                      "StaticBidirectionalGraph<G>::get_adjacent_vertices: Missing implementation for IsTraversalDirection.");
    }
}

template<IsStaticGraph G>
template<IsTraversalDirection Direction>
std::ranges::subrange<typename StaticBidirectionalGraph<G>::template AdjacentVertexIndexConstIteratorType<Direction>>
StaticBidirectionalGraph<G>::get_adjacent_vertex_indices(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        return std::ranges::subrange(
            typename StaticBidirectionalGraph<G>::AdjacentVertexIndexConstIteratorType<ForwardTraversal>(vertex,
                                                                                                         m_graph.get_edges(),
                                                                                                         m_edge_indices_grouped_by_source.at(vertex),
                                                                                                         true),
            typename StaticBidirectionalGraph<G>::AdjacentVertexIndexConstIteratorType<ForwardTraversal>(vertex,
                                                                                                         m_graph.get_edges(),
                                                                                                         m_edge_indices_grouped_by_source.at(vertex),
                                                                                                         false));
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        return std::ranges::subrange(
            typename StaticBidirectionalGraph<G>::AdjacentVertexIndexConstIteratorType<BackwardTraversal>(vertex,
                                                                                                          m_graph.get_edges(),
                                                                                                          m_edge_indices_grouped_by_target.at(vertex),
                                                                                                          true),
            typename StaticBidirectionalGraph<G>::AdjacentVertexIndexConstIteratorType<BackwardTraversal>(vertex,
                                                                                                          m_graph.get_edges(),
                                                                                                          m_edge_indices_grouped_by_target.at(vertex),
                                                                                                          false));
    }
    else
    {
        static_assert(dependent_false<Direction>::value,
                      "StaticBidirectionalGraph<G>::get_adjacent_vertex_indices: Missing implementation for IsTraversalDirection.");
    }
}

template<IsStaticGraph G>
template<IsTraversalDirection Direction>
std::ranges::subrange<typename StaticBidirectionalGraph<G>::template AdjacentEdgeConstIteratorType<Direction>>
StaticBidirectionalGraph<G>::get_adjacent_edges(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        return std::ranges::subrange(
            typename StaticBidirectionalGraph<G>::AdjacentEdgeConstIteratorType<ForwardTraversal>(vertex,
                                                                                                  m_graph.get_edges(),
                                                                                                  m_edge_indices_grouped_by_source.at(vertex),
                                                                                                  true),
            typename StaticBidirectionalGraph<G>::AdjacentEdgeConstIteratorType<ForwardTraversal>(vertex,
                                                                                                  m_graph.get_edges(),
                                                                                                  m_edge_indices_grouped_by_source.at(vertex),
                                                                                                  false));
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        return std::ranges::subrange(
            typename StaticBidirectionalGraph<G>::AdjacentEdgeConstIteratorType<BackwardTraversal>(vertex,
                                                                                                   m_graph.get_edges(),
                                                                                                   m_edge_indices_grouped_by_target.at(vertex),
                                                                                                   true),
            typename StaticBidirectionalGraph<G>::AdjacentEdgeConstIteratorType<BackwardTraversal>(vertex,
                                                                                                   m_graph.get_edges(),
                                                                                                   m_edge_indices_grouped_by_target.at(vertex),
                                                                                                   false));
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "StaticBidirectionalGraph<G>::get_adjacent_edges: Missing implementation for IsTraversalDirection.");
    }
}

template<IsStaticGraph G>
template<IsTraversalDirection Direction>
std::ranges::subrange<typename StaticBidirectionalGraph<G>::template AdjacentEdgeIndexConstIteratorType<Direction>>
StaticBidirectionalGraph<G>::get_adjacent_edge_indices(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        return std::ranges::subrange(
            typename StaticBidirectionalGraph<G>::AdjacentEdgeIndexConstIteratorType<ForwardTraversal>(vertex,
                                                                                                       m_graph.get_edges(),
                                                                                                       m_edge_indices_grouped_by_source.at(vertex),
                                                                                                       true),
            typename StaticBidirectionalGraph<G>::AdjacentEdgeIndexConstIteratorType<ForwardTraversal>(vertex,
                                                                                                       m_graph.get_edges(),
                                                                                                       m_edge_indices_grouped_by_source.at(vertex),
                                                                                                       false));
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        return std::ranges::subrange(
            typename StaticBidirectionalGraph<G>::AdjacentEdgeIndexConstIteratorType<BackwardTraversal>(vertex,
                                                                                                        m_graph.get_edges(),
                                                                                                        m_edge_indices_grouped_by_target.at(vertex),
                                                                                                        true),
            typename StaticBidirectionalGraph<G>::AdjacentEdgeIndexConstIteratorType<BackwardTraversal>(vertex,
                                                                                                        m_graph.get_edges(),
                                                                                                        m_edge_indices_grouped_by_target.at(vertex),
                                                                                                        false));
    }
    else
    {
        static_assert(dependent_false<Direction>::value,
                      "StaticBidirectionalGraph<G>::get_adjacent_edge_indices: Missing implementation for IsTraversalDirection.");
    }
}

template<IsStaticGraph G>
const StaticBidirectionalGraph<G>::VertexList& StaticBidirectionalGraph<G>::get_vertices() const
{
    return m_graph.get_vertices();
}

template<IsStaticGraph G>
const StaticBidirectionalGraph<G>::EdgeList& StaticBidirectionalGraph<G>::get_edges() const
{
    return m_graph.get_edges();
}

template<IsStaticGraph G>
size_t StaticBidirectionalGraph<G>::get_num_vertices() const
{
    return m_graph.get_num_vertices();
}

template<IsStaticGraph G>
size_t StaticBidirectionalGraph<G>::get_num_edges() const
{
    return m_graph.get_num_edges();
}

template<IsStaticGraph G>
template<IsTraversalDirection Direction>
VertexIndex StaticBidirectionalGraph<G>::get_source(EdgeIndex edge) const
{
    return m_graph.template get_source<Direction>(edge);
}

template<IsStaticGraph G>
template<IsTraversalDirection Direction>
VertexIndex StaticBidirectionalGraph<G>::get_target(EdgeIndex edge) const
{
    return m_graph.template get_target<Direction>(edge);
}

template<IsStaticGraph G>
template<IsTraversalDirection Direction>
const DegreeList& StaticBidirectionalGraph<G>::get_degrees() const
{
    return m_graph.template get_degrees<Direction>();
}

template<IsStaticGraph G>
template<IsTraversalDirection Direction>
Degree StaticBidirectionalGraph<G>::get_degree(VertexIndex vertex) const
{
    return m_graph.template get_degree<Direction>(vertex);
}

}
#endif
