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

#ifndef MIMIR_GRAPHS_GRAPH_HPP_
#define MIMIR_GRAPHS_GRAPH_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/common/grouped_vector.hpp"
#include "mimir/graphs/graph_edge_interface.hpp"
#include "mimir/graphs/graph_interface.hpp"
#include "mimir/graphs/graph_iterators.hpp"
#include "mimir/graphs/graph_vertex_interface.hpp"

#include <ranges>
#include <span>
#include <vector>

namespace mimir
{

/**
 * Declarations
 */

/* Graph */

/// @brief A `Graph` G consists of vertices V and edges E.
/// A `Graph` provides functionality for adding vertices and edges, and iterating over vertices and edges.
/// Iterating over the outgoing or incoming edges,
/// as well as the ingoing or outgoing adjacent vertices requires time O(|E|).
///
/// A `Graph` can be translated into an equivalent ForwardGraph or Bidirectional Graph in O(|V|+|E|*Log2(|E|))
/// to iterate more efficiently over exactly the outgoing or incoming edges,
/// as well as the ingoing or outgoing adjacent vertices.
/// @tparam Vertex is vertex type.
/// @tparam Edge is the edge type.
template<IsVertex Vertex, IsEdge Edge>
class Graph
{
public:
    using VertexType = Vertex;
    using VertexList = std::vector<Vertex>;
    using EdgeType = Edge;
    using EdgeList = std::vector<Edge>;

private:
    VertexList m_vertices;
    EdgeList m_edges;

    // Slice over all edges for iteration.
    std::vector<EdgeIndex> m_slice;

public:
    Graph();

    template<typename... Args>
    VertexIndex add_vertex(Args&&... args);

    template<typename... Args>
    EdgeIndex add_directed_edge(VertexIndex source, VertexIndex target, Args&&... args);

    template<typename... Args>
    std::pair<EdgeIndex, EdgeIndex> add_undirected_edge(VertexIndex source, VertexIndex target, Args&&... args);

    /// @brief Reinitialize the graph to an empty graph.
    void reset();

    /**
     * Iterators
     */

    template<IsTraversalDirection Direction>
    VertexIterator<Vertex, Edge, Direction> get_adjacent_vertices(VertexIndex vertex) const;
    template<IsTraversalDirection Direction>
    VertexIndexIterator<Edge, Direction> get_adjacent_vertex_indices(VertexIndex vertex) const;
    template<IsTraversalDirection Direction>
    EdgeIterator<Edge, Direction> get_adjacent_edges(VertexIndex vertex) const;
    template<IsTraversalDirection Direction>
    EdgeIndexIterator<Edge, Direction> get_adjacent_edge_indices(VertexIndex vertex) const;

    /**
     * Getters
     */

    const VertexList& get_vertices() const;
    const EdgeList& get_edges() const;
    size_t get_num_vertices() const;
    size_t get_num_edges() const;
    const std::vector<EdgeIndex>& get_slice() const;
};

/* ForwardGraph */

/// @brief ForwardGraph is a translated Graph for more efficient forward traversal.
template<IsGraph G>
class ForwardGraph
{
public:
    using VertexType = typename G::VertexType;
    using EdgeType = typename G::EdgeType;
    using VertexList = std::vector<VertexType>;
    using EdgeList = std::vector<EdgeType>;

private:
    G m_graph;

    IndexGroupedVector<const EdgeIndex> m_edge_indices_grouped_by_source;

public:
    explicit ForwardGraph(G graph);

    /**
     * Iterators
     */

    template<IsTraversalDirection Direction>
    VertexIterator<VertexType, EdgeType, Direction> get_adjacent_vertices(VertexIndex vertex) const;
    template<IsTraversalDirection Direction>
    VertexIndexIterator<EdgeType, Direction> get_adjacent_vertex_indices(VertexIndex vertex) const;
    template<IsTraversalDirection Direction>
    EdgeIterator<EdgeType, Direction> get_adjacent_edges(VertexIndex vertex) const;
    template<IsTraversalDirection Direction>
    EdgeIndexIterator<EdgeType, Direction> get_adjacent_edge_indices(VertexIndex vertex) const;

    /**
     * Getters
     */

    const VertexList& get_vertices() const;
    const EdgeList& get_edges() const;
    size_t get_num_vertices() const;
    size_t get_num_edges() const;
    const std::vector<EdgeIndex>& get_slice() const;
};

/* BidirectionalGraph */

/// @brief BidirectionalGraph is a translated Graph for more efficient bidirectional traversal.
template<IsGraph G>
class BidirectionalGraph
{
public:
    using VertexType = typename G::VertexType;
    using EdgeType = typename G::EdgeType;
    using VertexList = std::vector<VertexType>;
    using EdgeList = std::vector<EdgeType>;

private:
    G m_graph;

    IndexGroupedVector<const EdgeIndex> m_edge_indices_grouped_by_source;
    IndexGroupedVector<const EdgeIndex> m_edge_indices_grouped_by_target;

public:
    explicit BidirectionalGraph(G graph);

    /**
     * Iterators
     */

    template<IsTraversalDirection Direction>
    VertexIterator<VertexType, EdgeType, Direction> get_adjacent_vertices(VertexIndex vertex) const;
    template<IsTraversalDirection Direction>
    VertexIndexIterator<EdgeType, Direction> get_adjacent_vertex_indices(VertexIndex vertex) const;
    template<IsTraversalDirection Direction>
    EdgeIterator<EdgeType, Direction> get_adjacent_edges(VertexIndex vertex) const;
    template<IsTraversalDirection Direction>
    EdgeIndexIterator<EdgeType, Direction> get_adjacent_edge_indices(VertexIndex vertex) const;

    /**
     * Getters
     */

    const VertexList& get_vertices() const;
    const EdgeList& get_edges() const;
    size_t get_num_vertices() const;
    size_t get_num_edges() const;
    const std::vector<EdgeIndex>& get_slice() const;
};

/**
 * Implementations
 */

/* Graph */

template<IsVertex Vertex, IsEdge Edge>
Graph<Vertex, Edge>::Graph() : m_vertices(), m_edges()
{
}

template<IsVertex Vertex, IsEdge Edge>
template<typename... Args>
VertexIndex Graph<Vertex, Edge>::add_vertex(Args&&... args)
{
    const auto index = m_vertices.size();
    m_vertices.emplace_back(index, std::forward<Args>(args)...);
    return index;
}

template<IsVertex Vertex, IsEdge Edge>
template<typename... Args>
EdgeIndex Graph<Vertex, Edge>::add_directed_edge(VertexIndex source, VertexIndex target, Args&&... args)
{
    if (source >= get_num_vertices() || target >= get_num_vertices() || source < 0 || target < 0)
    {
        throw std::out_of_range("Source or destination vertex out of range");
    }
    const auto index = m_edges.size();
    m_edges.emplace_back(index, source, target, std::forward<Args>(args)...);
    m_slice.push_back(index);
    return index;
}

template<IsVertex Vertex, IsEdge Edge>
template<typename... Args>
std::pair<EdgeIndex, EdgeIndex> Graph<Vertex, Edge>::add_undirected_edge(VertexIndex source, VertexIndex target, Args&&... args)
{
    if (source >= get_num_vertices() || target >= get_num_vertices() || source < 0 || target < 0)
    {
        throw std::out_of_range("Source or destination vertex out of range");
    }
    // Need to copy args to keep them in valid state.
    const auto forward_edge_index = add_directed_edge(source, target, args...);
    const auto backward_edge_index = add_directed_edge(target, source, std::forward<Args>(args)...);
    m_slice.push_back(forward_edge_index);
    m_slice.push_back(backward_edge_index);
    return std::make_pair(forward_edge_index, backward_edge_index);
}

template<IsVertex Vertex, IsEdge Edge>
void Graph<Vertex, Edge>::reset()
{
    m_vertices.clear();
    m_edges.clear();
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
VertexIterator<Vertex, Edge, Direction> Graph<Vertex, Edge>::get_adjacent_vertices(VertexIndex vertex) const
{
    return VertexIterator<Vertex, Edge, Direction>(vertex, m_vertices, m_edges, m_slice);
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
VertexIndexIterator<Edge, Direction> Graph<Vertex, Edge>::get_adjacent_vertex_indices(VertexIndex vertex) const
{
    return VertexIndexIterator<Edge, Direction>(vertex, m_edges, m_slice);
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
EdgeIterator<Edge, Direction> Graph<Vertex, Edge>::get_adjacent_edges(VertexIndex vertex) const
{
    return EdgeIterator<Edge, Direction>(vertex, m_edges, m_slice);
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
EdgeIndexIterator<Edge, Direction> Graph<Vertex, Edge>::get_adjacent_edge_indices(VertexIndex vertex) const
{
    return EdgeIndexIterator<Edge, Direction>(vertex, m_edges, m_slice);
}

template<IsVertex Vertex, IsEdge Edge>
const Graph<Vertex, Edge>::VertexList& Graph<Vertex, Edge>::get_vertices() const
{
    return m_vertices;
}

template<IsVertex Vertex, IsEdge Edge>
const Graph<Vertex, Edge>::EdgeList& Graph<Vertex, Edge>::get_edges() const
{
    return m_edges;
}

template<IsVertex Vertex, IsEdge Edge>
size_t Graph<Vertex, Edge>::get_num_vertices() const
{
    return m_vertices.size();
}

template<IsVertex Vertex, IsEdge Edge>
size_t Graph<Vertex, Edge>::get_num_edges() const
{
    return m_edges.size();
}

template<IsVertex Vertex, IsEdge Edge>
const std::vector<EdgeIndex>& Graph<Vertex, Edge>::get_slice() const
{
    return m_slice;
}

/* ForwardGraph */

/// @brief Groups edge indices by source or target, depending on forward is true or false.
/// @tparam Vertex is the type of vertices in the graph.
/// @tparam Edge is the type of edges in the graph.
/// @param graph is the graph.
/// @param forward true will group by source and false will group by target.
/// @return
template<IsVertex Vertex, IsEdge Edge>
static IndexGroupedVector<const EdgeIndex> compute_index_grouped_edge_indices(const Graph<Vertex, Edge>& graph, bool forward)
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

template<IsGraph G>
ForwardGraph<G>::ForwardGraph(G graph) : m_graph(std::move(graph)), m_edge_indices_grouped_by_source(compute_index_grouped_edge_indices(m_graph, true))
{
}

template<IsGraph G>
template<IsTraversalDirection Direction>
VertexIterator<typename ForwardGraph<G>::VertexType, typename ForwardGraph<G>::EdgeType, Direction>
ForwardGraph<G>::get_adjacent_vertices(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        return VertexIterator<typename ForwardGraph<G>::VertexType, typename ForwardGraph<G>::EdgeType, ForwardTraversal>(
            vertex,
            m_graph.get_vertices(),
            m_graph.get_edges(),
            m_edge_indices_grouped_by_source.at(vertex));
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        return m_graph.template get_adjacent_vertices<BackwardTraversal>(vertex);
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "ForwardGraph<G>::get_adjacent_vertices: Missing implementation for IsTraversalDirection.");
    }
}

template<IsGraph G>
template<IsTraversalDirection Direction>
VertexIndexIterator<typename ForwardGraph<G>::EdgeType, Direction> ForwardGraph<G>::get_adjacent_vertex_indices(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        return VertexIndexIterator<typename ForwardGraph<G>::EdgeType, ForwardTraversal>(vertex,
                                                                                         m_graph.get_edges(),
                                                                                         m_edge_indices_grouped_by_source.at(vertex));
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        return m_graph.template get_adjacent_vertex_indices<BackwardTraversal>(vertex);
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "ForwardGraph<G>::get_adjacent_vertex_indices: Missing implementation for IsTraversalDirection.");
    }
}

template<IsGraph G>
template<IsTraversalDirection Direction>
EdgeIterator<typename ForwardGraph<G>::EdgeType, Direction> ForwardGraph<G>::get_adjacent_edges(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        return EdgeIterator<typename ForwardGraph<G>::EdgeType, Direction>(vertex, m_graph.get_edges(), m_edge_indices_grouped_by_source.at(vertex));
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        return m_graph.template get_backward_edges<BackwardTraversal>(vertex);
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "ForwardGraph<G>::get_adjacent_edges: Missing implementation for IsTraversalDirection.");
    }
}

template<IsGraph G>
template<IsTraversalDirection Direction>
EdgeIndexIterator<typename ForwardGraph<G>::EdgeType, Direction> ForwardGraph<G>::get_adjacent_edge_indices(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        return EdgeIndexIterator<typename ForwardGraph<G>::EdgeType, Direction>(vertex,
                                                                                m_graph.get_vertices(),
                                                                                m_graph.get_edges(),
                                                                                m_edge_indices_grouped_by_source.at(vertex));
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        return m_graph.template get_backward_edge_indices<BackwardTraversal>(vertex);
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "ForwardGraph<G>::get_adjacent_edge_indices: Missing implementation for IsTraversalDirection.");
    }
}

template<IsGraph G>
const ForwardGraph<G>::VertexList& ForwardGraph<G>::get_vertices() const
{
    return m_graph.get_vertices();
}

template<IsGraph G>
const ForwardGraph<G>::EdgeList& ForwardGraph<G>::get_edges() const
{
    return m_graph.get_edges();
}

template<IsGraph G>
size_t ForwardGraph<G>::get_num_vertices() const
{
    return m_graph.get_num_vertices();
}

template<IsGraph G>
size_t ForwardGraph<G>::get_num_edges() const
{
    return m_graph.get_num_edges();
}

template<IsGraph G>
const std::vector<EdgeIndex>& ForwardGraph<G>::get_slice() const
{
    return m_graph.get_slice();
}

/* BidirectionalGraph */

template<IsGraph G>
BidirectionalGraph<G>::BidirectionalGraph(G graph) :
    m_graph(std::move(graph)),
    m_edge_indices_grouped_by_source(compute_index_grouped_edge_indices(m_graph, true)),
    m_edge_indices_grouped_by_target(compute_index_grouped_edge_indices(m_graph, false))
{
}

template<IsGraph G>
template<IsTraversalDirection Direction>
VertexIterator<typename BidirectionalGraph<G>::VertexType, typename BidirectionalGraph<G>::EdgeType, Direction>
BidirectionalGraph<G>::get_adjacent_vertices(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        return VertexIterator<typename ForwardGraph<G>::VertexType, typename ForwardGraph<G>::EdgeType, ForwardTraversal>(
            vertex,
            m_graph.get_vertices(),
            m_graph.get_edges(),
            m_edge_indices_grouped_by_source.at(vertex));
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        return VertexIterator<typename ForwardGraph<G>::VertexType, typename ForwardGraph<G>::EdgeType, BackwardTraversal>(
            vertex,
            m_graph.get_vertices(),
            m_graph.get_edges(),
            m_edge_indices_grouped_by_target.at(vertex));
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "BidirectionalGraph<G>::get_adjacent_vertices: Missing implementation for IsTraversalDirection.");
    }
}

template<IsGraph G>
template<IsTraversalDirection Direction>
VertexIndexIterator<typename BidirectionalGraph<G>::EdgeType, Direction> BidirectionalGraph<G>::get_adjacent_vertex_indices(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        return VertexIndexIterator<typename ForwardGraph<G>::EdgeType, ForwardTraversal>(vertex,
                                                                                         m_graph.get_edges(),
                                                                                         m_edge_indices_grouped_by_source.at(vertex));
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        return VertexIndexIterator<typename ForwardGraph<G>::EdgeType, BackwardTraversal>(vertex,
                                                                                          m_graph.get_edges(),
                                                                                          m_edge_indices_grouped_by_target.at(vertex));
    }
    else
    {
        static_assert(dependent_false<Direction>::value,
                      "BidirectionalGraph<G>::get_adjacent_vertex_indices: Missing implementation for IsTraversalDirection.");
    }
}

template<IsGraph G>
template<IsTraversalDirection Direction>
EdgeIterator<typename BidirectionalGraph<G>::EdgeType, Direction> BidirectionalGraph<G>::get_adjacent_edges(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        return EdgeIterator<typename ForwardGraph<G>::EdgeType, ForwardTraversal>(vertex, m_graph.get_edges(), m_edge_indices_grouped_by_source.at(vertex));
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        return EdgeIterator<typename ForwardGraph<G>::EdgeType, BackwardTraversal>(vertex, m_graph.get_edges(), m_edge_indices_grouped_by_target.at(vertex));
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "BidirectionalGraph<G>::get_adjacent_edges: Missing implementation for IsTraversalDirection.");
    }
}

template<IsGraph G>
template<IsTraversalDirection Direction>
EdgeIndexIterator<typename BidirectionalGraph<G>::EdgeType, Direction> BidirectionalGraph<G>::get_adjacent_edge_indices(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        return EdgeIndexIterator<typename ForwardGraph<G>::EdgeType, ForwardTraversal>(vertex,
                                                                                       m_graph.get_edges(),
                                                                                       m_edge_indices_grouped_by_source.at(vertex));
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        return EdgeIndexIterator<typename ForwardGraph<G>::EdgeType, BackwardTraversal>(vertex,
                                                                                        m_graph.get_edges(),
                                                                                        m_edge_indices_grouped_by_target.at(vertex));
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "BidirectionalGraph<G>::get_adjacent_edge_indices: Missing implementation for IsTraversalDirection.");
    }
}

template<IsGraph G>
const BidirectionalGraph<G>::VertexList& BidirectionalGraph<G>::get_vertices() const
{
    return m_graph.get_vertices();
}

template<IsGraph G>
const BidirectionalGraph<G>::EdgeList& BidirectionalGraph<G>::get_edges() const
{
    return m_graph.get_edges();
}

template<IsGraph G>
size_t BidirectionalGraph<G>::get_num_vertices() const
{
    return m_graph.get_num_vertices();
}

template<IsGraph G>
size_t BidirectionalGraph<G>::get_num_edges() const
{
    return m_graph.get_num_edges();
}

template<IsGraph G>
const std::vector<EdgeIndex>& BidirectionalGraph<G>::get_slice() const
{
    return m_graph.get_slice();
}

}
#endif
