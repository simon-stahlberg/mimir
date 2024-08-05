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

#ifndef MIMIR_GRAPHS_DYNAMIC_GRAPH_HPP_
#define MIMIR_GRAPHS_DYNAMIC_GRAPH_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/graphs/dynamic_graph_interface.hpp"
#include "mimir/graphs/dynamic_graph_iterators.hpp"
#include "mimir/graphs/graph_edge_interface.hpp"
#include "mimir/graphs/graph_vertex_interface.hpp"

#include <ranges>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mimir
{

/**
 * Declarations
 */

/* DynamicGraph */

/// @brief A `DynamicGraph` G consists of vertices V and edges E that allows for vertex and edge removal
/// and as a result requires less efficient hash sets and hash map.
///
/// A `DynamicGraph` provides functionality for adding or removing vertices and edges, and iterating over vertices and edges.
/// Iterating over the outgoing or incoming edges, or the ingoing or outgoing adjacent vertices requires time O(|E|),
/// which is usually much smaller due to storing and exploiting adjacency list in forward and backward direction.
/// @tparam Vertex is vertex type.
/// @tparam Edge is the edge type.
template<IsVertex Vertex, IsEdge Edge>
class DynamicGraph
{
public:
    using GraphType = DynamicGraphTag;
    using VertexType = Vertex;
    using VertexMap = std::unordered_map<VertexIndex, Vertex>;
    using EdgeType = Edge;
    using EdgeMap = std::unordered_map<EdgeIndex, Edge>;

    using VertexIndexConstIteratorType = DynamicVertexIndexConstIterator<VertexType>;
    using EdgeIndexConstIteratorType = DynamicEdgeIndexConstIterator<EdgeType>;

    template<IsTraversalDirection Direction>
    using AdjacentVertexConstIteratorType = DynamicAdjacentVertexConstIterator<VertexType, EdgeType, Direction>;
    template<IsTraversalDirection Direction>
    using AdjacentVertexIndexConstIteratorType = DynamicAdjacentVertexIndexConstIterator<EdgeType, Direction>;
    template<IsTraversalDirection Direction>
    using AdjacentEdgeConstIteratorType = DynamicAdjacentEdgeConstIterator<EdgeType, Direction>;
    template<IsTraversalDirection Direction>
    using AdjacentEdgeIndexConstIteratorType = DynamicAdjacentEdgeIndexConstIterator<EdgeType, Direction>;

    DynamicGraph();

    /// @brief Reinitialize the graph to an empty graph.
    void clear();

    /**
     * Constructible functionality.
     */

    /// @brief Add a vertex to the graph with vertex properties args.
    /// @tparam ...Args the types of the vertex properties. Must match the properties mentioned in the vertex constructor.
    /// @param ...args the properties.
    /// @return the index of the newly created vertex.
    template<typename... Args>
    VertexIndex add_vertex(Args&&... args);

    /// @brief Add a directed edge from source to target to the graph with edge properties args.
    /// @tparam ...Args the types of the edge properties. Must match the properties mentioned in the edge constructor.
    /// @param source the source vertex.
    /// @param target the target vertex.
    /// @param ...args the properties.
    /// @return the index of the newly created edge.
    template<typename... Args>
    EdgeIndex add_directed_edge(VertexIndex source, VertexIndex target, Args&&... args);

    /// @brief Add two anti-parallel directed edges to the graph with the identical edge properties, representing the undirected edge.
    /// If the edge properties are heavy weight, we suggest externalizing the properties and storing an index to the properties instead.
    /// @tparam ...Args the types of the edge properties. Must match the properties mentioned in the edge constructor.
    /// @param source the source vertex.
    /// @param target the target vertex.
    /// @param ...args the properties.
    /// @return the index pair of the two newly created edges.
    template<typename... Args>
    std::pair<EdgeIndex, EdgeIndex> add_undirected_edge(VertexIndex source, VertexIndex target, Args&&... args);

    /**
     * Destructible functionality.
     */

    /// @brief Remove the given vertex and all its adjacent edges from the graph.
    /// @param vertex the vertex to be removed.
    void remove_vertex(VertexIndex vertex);

    /// @brief Remove the given edge from the graph.
    /// @param edge
    void remove_edge(EdgeIndex edge);

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

    const VertexMap& get_vertices() const;
    const EdgeMap& get_edges() const;
    size_t get_num_vertices() const;
    size_t get_num_edges() const;

    template<IsTraversalDirection>
    VertexIndex get_source(EdgeIndex edge) const;
    template<IsTraversalDirection>
    VertexIndex get_target(EdgeIndex edge) const;
    template<IsTraversalDirection Direction>
    const DegreeMap& get_degrees() const;
    template<IsTraversalDirection Direction>
    Degree get_degree(VertexIndex vertex) const;

private:
    VertexMap m_vertices;
    VertexIndexList m_free_vertices;
    size_t m_next_vertex_index;
    EdgeMap m_edges;
    EdgeIndexList m_free_edges;
    size_t m_next_edge_index;

    TraversalDirectionStorage<std::unordered_map<VertexIndex, EdgeIndexSet>> m_adjacent_edges;

    TraversalDirectionStorage<DegreeMap> m_degrees;
};

/**
 * Implementations
 */

/* DynamicGraph */

template<IsVertex Vertex, IsEdge Edge>
DynamicGraph<Vertex, Edge>::DynamicGraph() :
    m_vertices(),
    m_free_vertices(),
    m_next_vertex_index(0),
    m_edges(),
    m_free_edges(),
    m_next_edge_index(0),
    m_adjacent_edges(),
    m_degrees()
{
}

template<IsVertex Vertex, IsEdge Edge>
void DynamicGraph<Vertex, Edge>::clear()
{
    m_vertices.clear();
    m_free_edges.clear();
    m_next_vertex_index = 0;
    m_edges.clear();
    m_free_edges.clear();
    m_next_edge_index = 0;
    m_adjacent_edges.get<ForwardTraversal>().clear();
    m_adjacent_edges.get<BackwardTraversal>().clear();
    m_degrees.get<ForwardTraversal>().clear();
    m_degrees.get<BackwardTraversal>().clear();
}

template<IsVertex Vertex, IsEdge Edge>
template<typename... Args>
VertexIndex DynamicGraph<Vertex, Edge>::add_vertex(Args&&... args)
{
    /* Get the vertex index. */
    auto index = m_free_vertices.empty() ? m_next_vertex_index++ : m_free_vertices.back();
    if (!m_free_vertices.empty())
    {
        // If m_free_vertices was non-empty, we additionally need to pop_back the used index.
        m_free_vertices.pop_back();
    }

    /* Create the vertex. */
    m_vertices.emplace_back(index, std::forward<Args>(args)...);

    /* Initialize the data structures. */
    if (m_free_vertices.empty())
    {
        m_adjacent_edges.get<ForwardTraversal>().emplace(index, EdgeIndexSet());
        m_adjacent_edges.get<BackwardTraversal>().emplace(index, EdgeIndexSet());
        m_degrees.get<ForwardTraversal>().emplace(index, 0);
        m_degrees.get<BackwardTraversal>().emplace(index, 0);
    }
    else
    {
        m_adjacent_edges.get<ForwardTraversal>().at(index).clear();
        m_adjacent_edges.get<ForwardTraversal>().at(index).clear();
        m_degrees.get<ForwardTraversal>().at(index) = 0;
        m_degrees.get<ForwardTraversal>().at(index) = 0;
    }

    return index;
}

template<IsVertex Vertex, IsEdge Edge>
template<typename... Args>
EdgeIndex DynamicGraph<Vertex, Edge>::add_directed_edge(VertexIndex source, VertexIndex target, Args&&... args)
{
    if (!m_vertices.contains(source) || !m_vertices.contains(target))
    {
        throw std::out_of_range("DynamicGraph<Vertex, Edge>::add_directed_edge(...): Source or destination vertex out of range");
    }

    /* Get the edge index */
    const auto index = m_free_edges.empty() ? m_next_edge_index++ : m_free_edges.back();
    if (!m_free_edges.empty())
    {
        // If m_free_edges was non-empty, we additionally need to pop_back the used index.
        m_free_edges.pop_back();
    }

    /* Create the edge */
    m_edges.emplace_back(index, source, target, std::forward<Args>(args)...);

    /* Initialize the data structures. */
    m_adjacent_edges.get<ForwardTraversal>().at(source).insert(index);
    m_adjacent_edges.get<BackwardTraversal>().at(target).insert(index);
    ++m_degrees.get<ForwardTraversal>().at(source);
    ++m_degrees.get<BackwardTraversal>().at(target);

    return index;
}

template<IsVertex Vertex, IsEdge Edge>
template<typename... Args>
std::pair<EdgeIndex, EdgeIndex> DynamicGraph<Vertex, Edge>::add_undirected_edge(VertexIndex source, VertexIndex target, Args&&... args)
{
    if (!m_vertices.contains(source) || !m_vertices.contains(target))
    {
        throw std::out_of_range("DynamicGraph<Vertex, Edge>::add_undirected_edge(...): Source or destination vertex out of range");
    }
    // Need to copy args to keep them in valid state.
    const auto forward_edge_index = add_directed_edge(source, target, args...);
    const auto backward_edge_index = add_directed_edge(target, source, std::forward<Args>(args)...);

    return std::make_pair(forward_edge_index, backward_edge_index);
}

template<IsVertex Vertex, IsEdge Edge>
void DynamicGraph<Vertex, Edge>::remove_vertex(VertexIndex vertex)
{
    if (!m_vertices.contains(vertex))
    {
        throw std::out_of_range("DynamicGraph<Vertex, Edge>::remove_vertex(...): Tried to remove non-existing vertex.");
    }

    /* Remove backward adjacent edges from vertex of adjacent vertices */
    for (const auto& edge : get_adjacent_edge_indices<ForwardTraversal>(vertex))
    {
        const auto source = get_source<ForwardTraversal>(edge);
        if (source == vertex)
        {
            // Ignore loops over vertex.
            continue;
        }
        const auto target = get_target<ForwardTraversal>(edge);

        m_adjacent_edges.get<BackwardTraversal>().at(target).erase(edge);
        --m_degrees.get<BackwardTraversal>().at(target);
        m_edges.erase(edge);
        m_free_edges.push_back(edge);
    }
    /* Remove forward adjacent edges to vertex of adjacent vertices */
    for (const auto& edge : get_adjacent_edge_indices<BackwardTraversal>(vertex))
    {
        const auto source = get_source<BackwardTraversal>(edge);
        if (source == vertex)
        {
            // Ignore loops over vertex.
            continue;
        }
        const auto target = get_target<BackwardTraversal>(edge);

        m_adjacent_edges.get<ForwardTraversal>().at(target).erase(edge);
        --m_degrees.get<ForwardTraversal>().at(target);
        m_edges.erase(edge);
        m_free_edges.push_back(edge);
    }

    m_vertices.erase(vertex);
    m_free_vertices.push_back(vertex);
}

template<IsVertex Vertex, IsEdge Edge>
void DynamicGraph<Vertex, Edge>::remove_edge(EdgeIndex edge)
{
    const auto source = get_source<ForwardTraversal>(edge);
    const auto target = get_target<ForwardTraversal>(edge);

    m_adjacent_edges.get<ForwardTraversal>().at(source).erase(edge);
    m_adjacent_edges.get<BackwardTraversal>().at(target).erase(edge);
    --m_degrees.get<ForwardTraversal>().at(source);
    --m_degrees.get<BackwardTraversal>().at(target);
    m_edges.erase(edge);
    m_free_edges.push_back(edge);
}

template<IsVertex Vertex, IsEdge Edge>
std::ranges::subrange<typename DynamicGraph<Vertex, Edge>::VertexIndexConstIteratorType> DynamicGraph<Vertex, Edge>::get_vertex_indices() const
{
    return std::ranges::subrange(typename DynamicGraph<Vertex, Edge>::VertexIndexConstIteratorType(m_vertices, true),
                                 typename DynamicGraph<Vertex, Edge>::VertexIndexConstIteratorType(m_vertices, false));
}

template<IsVertex Vertex, IsEdge Edge>
std::ranges::subrange<typename DynamicGraph<Vertex, Edge>::EdgeIndexConstIteratorType> DynamicGraph<Vertex, Edge>::get_edge_indices() const
{
    return std::ranges::subrange(typename DynamicGraph<Vertex, Edge>::EdgeIndexConstIteratorType(m_edges, true),
                                 typename DynamicGraph<Vertex, Edge>::EdgeIndexConstIteratorType(m_edges, false));
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
std::ranges::subrange<typename DynamicGraph<Vertex, Edge>::template AdjacentVertexConstIteratorType<Direction>>
DynamicGraph<Vertex, Edge>::get_adjacent_vertices(VertexIndex vertex) const
{
    return std::ranges::subrange(
        typename DynamicGraph<Vertex, Edge>::AdjacentVertexConstIteratorType<ForwardTraversal>(m_vertices,
                                                                                               m_edges,
                                                                                               m_adjacent_edges.get<Direction>().at(vertex),
                                                                                               true),
        typename DynamicGraph<Vertex, Edge>::AdjacentVertexConstIteratorType<ForwardTraversal>(m_vertices,
                                                                                               m_edges,
                                                                                               m_adjacent_edges.get<Direction>().at(vertex),
                                                                                               false));
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
std::ranges::subrange<typename DynamicGraph<Vertex, Edge>::template AdjacentVertexIndexConstIteratorType<Direction>>
DynamicGraph<Vertex, Edge>::get_adjacent_vertex_indices(VertexIndex vertex) const
{
    return std::ranges::subrange(
        typename DynamicGraph<Vertex, Edge>::AdjacentVertexIndexConstIteratorType<ForwardTraversal>(m_edges,
                                                                                                    m_adjacent_edges.get<Direction>().at(vertex),
                                                                                                    true),
        typename DynamicGraph<Vertex, Edge>::AdjacentVertexIndexConstIteratorType<ForwardTraversal>(m_edges,
                                                                                                    m_adjacent_edges.get<Direction>().at(vertex),
                                                                                                    false));
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
std::ranges::subrange<typename DynamicGraph<Vertex, Edge>::template AdjacentEdgeConstIteratorType<Direction>>
DynamicGraph<Vertex, Edge>::get_adjacent_edges(VertexIndex vertex) const
{
    return std::ranges::subrange(
        typename DynamicGraph<Vertex, Edge>::AdjacentEdgeConstIteratorType<ForwardTraversal>(m_edges, m_adjacent_edges.get<Direction>().at(vertex), true),
        typename DynamicGraph<Vertex, Edge>::AdjacentEdgeConstIteratorType<ForwardTraversal>(m_edges, m_adjacent_edges.get<Direction>().at(vertex), false));
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
std::ranges::subrange<typename DynamicGraph<Vertex, Edge>::template AdjacentEdgeIndexConstIteratorType<Direction>>
DynamicGraph<Vertex, Edge>::get_adjacent_edge_indices(VertexIndex vertex) const
{
    return std::ranges::subrange(
        typename DynamicGraph<Vertex, Edge>::AdjacentEdgeIndexConstIteratorType<ForwardTraversal>(m_edges, m_adjacent_edges.get<Direction>().at(vertex), true),
        typename DynamicGraph<Vertex, Edge>::AdjacentEdgeIndexConstIteratorType<ForwardTraversal>(m_edges,
                                                                                                  m_adjacent_edges.get<Direction>().at(vertex),
                                                                                                  false));
}

template<IsVertex Vertex, IsEdge Edge>
const typename DynamicGraph<Vertex, Edge>::VertexMap& DynamicGraph<Vertex, Edge>::get_vertices() const
{
    return m_vertices;
}

template<IsVertex Vertex, IsEdge Edge>
const typename DynamicGraph<Vertex, Edge>::EdgeMap& DynamicGraph<Vertex, Edge>::get_edges() const
{
    return m_edges;
}

template<IsVertex Vertex, IsEdge Edge>
size_t DynamicGraph<Vertex, Edge>::get_num_vertices() const
{
    return m_vertices.size();
}

template<IsVertex Vertex, IsEdge Edge>
size_t DynamicGraph<Vertex, Edge>::get_num_edges() const
{
    return m_edges.size();
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
VertexIndex DynamicGraph<Vertex, Edge>::get_source(EdgeIndex edge) const
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
        static_assert(dependent_false<Direction>::value, "DynamicGraph<Vertex, Edge>::get_source(...): Missing implementation for IsTraversalDirection.");
    }
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
VertexIndex DynamicGraph<Vertex, Edge>::get_target(EdgeIndex edge) const
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
        static_assert(dependent_false<Direction>::value, "DynamicGraph<Vertex, Edge>::get_target(...): Missing implementation for IsTraversalDirection.");
    }
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
const DegreeMap& DynamicGraph<Vertex, Edge>::get_degrees() const
{
    return m_degrees.get<Direction>();
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
Degree DynamicGraph<Vertex, Edge>::get_degree(VertexIndex vertex) const
{
    return m_degrees.get<Direction>().at(vertex);
}

}

#endif
