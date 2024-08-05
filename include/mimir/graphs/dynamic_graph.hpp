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
     * Destructible functionality.
     */

    void remove_vertex(VertexIndex vertex);

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

    std::unordered_map<VertexIndex, EdgeIndexSet> m_forward_adjacent_edges;
    std::unordered_map<VertexIndex, EdgeIndexSet> m_backward_adjacent_edges;

    DegreeMap m_in_degrees;
    DegreeMap m_out_degrees;
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
    m_forward_adjacent_edges(),
    m_backward_adjacent_edges(),
    m_in_degrees(),
    m_out_degrees()
{
}

template<IsVertex Vertex, IsEdge Edge>
void DynamicGraph<Vertex, Edge>::reset()
{
    m_vertices.clear();
    m_free_edges.clear();
    m_next_vertex_index = 0;
    m_edges.clear();
    m_free_edges.clear();
    m_next_edge_index = 0;
    m_forward_adjacent_edges.clear();
    m_backward_adjacent_edges.clear();
    m_in_degrees.clear();
    m_out_degrees.clear();
}

template<IsVertex Vertex, IsEdge Edge>
template<typename... Args>
VertexIndex DynamicGraph<Vertex, Edge>::add_vertex(Args&&... args)
{
    /* Get the vertex index. */
    auto index = m_free_vertices.empty() ? m_next_vertex_index++ : m_free_vertices.back();

    /* Create the vertex. */
    m_vertices.emplace_back(index, std::forward<Args>(args)...);

    /* Initialize the data structures. */
    if (m_free_vertices.empty())
    {
        m_forward_adjacent_edges.emplace(index, EdgeIndexSet());
        m_backward_adjacent_edges.emplace(index, EdgeIndexSet());
        m_in_degrees.emplace(index, 0);
        m_out_degrees.emplace(index, 0);
    }
    else
    {
        // If free_vertices was non empty, we additionally need to pop_back the used index.
        m_free_vertices.pop_back();
        m_forward_adjacent_edges.at(index).clear();
        m_backward_adjacent_edges.at(index).clear();
        m_in_degrees.at(index) = 0;
        m_out_degrees.at(index) = 0;
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

    /* Create the edge */
    m_edges.emplace_back(index, source, target, std::forward<Args>(args)...);

    if (m_free_edges.empty()) {}
    else
    {
        m_free_edges.pop_back();
    }

    return index;
}

template<IsVertex Vertex, IsEdge Edge>
template<typename... Args>
std::pair<EdgeIndex, EdgeIndex> DynamicGraph<Vertex, Edge>::add_undirected_edge(VertexIndex source, VertexIndex target, Args&&... args)
{
    throw std::runtime_error("Not implemented");
}

template<IsVertex Vertex, IsEdge Edge>
void DynamicGraph<Vertex, Edge>::remove_vertex(VertexIndex vertex)
{
    throw std::runtime_error("Not implemented");
}

template<IsVertex Vertex, IsEdge Edge>
void DynamicGraph<Vertex, Edge>::remove_edge(EdgeIndex edge)
{
    throw std::runtime_error("Not implemented");
}

template<IsVertex Vertex, IsEdge Edge>
std::ranges::subrange<typename DynamicGraph<Vertex, Edge>::VertexIndexConstIteratorType> DynamicGraph<Vertex, Edge>::get_vertex_indices() const
{
    throw std::runtime_error("Not implemented");
}

template<IsVertex Vertex, IsEdge Edge>
std::ranges::subrange<typename DynamicGraph<Vertex, Edge>::EdgeIndexConstIteratorType> DynamicGraph<Vertex, Edge>::get_edge_indices() const
{
    throw std::runtime_error("Not implemented");
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
std::ranges::subrange<typename DynamicGraph<Vertex, Edge>::template AdjacentVertexConstIteratorType<Direction>>
DynamicGraph<Vertex, Edge>::get_adjacent_vertices(VertexIndex vertex) const
{
    throw std::runtime_error("Not implemented");
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
std::ranges::subrange<typename DynamicGraph<Vertex, Edge>::template AdjacentVertexIndexConstIteratorType<Direction>>
DynamicGraph<Vertex, Edge>::get_adjacent_vertex_indices(VertexIndex vertex) const
{
    throw std::runtime_error("Not implemented");
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
std::ranges::subrange<typename DynamicGraph<Vertex, Edge>::template AdjacentEdgeConstIteratorType<Direction>>
DynamicGraph<Vertex, Edge>::get_adjacent_edges(VertexIndex vertex) const
{
    throw std::runtime_error("Not implemented");
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
std::ranges::subrange<typename DynamicGraph<Vertex, Edge>::template AdjacentEdgeIndexConstIteratorType<Direction>>
DynamicGraph<Vertex, Edge>::get_adjacent_edge_indices(VertexIndex vertex) const
{
    throw std::runtime_error("Not implemented");
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
        static_assert(dependent_false<Direction>::value, "DynamicGraph<Vertex, Edge>::get_degrees(): Missing implementation for IsTraversalDirection.");
    }
}

template<IsVertex Vertex, IsEdge Edge>
template<IsTraversalDirection Direction>
Degree DynamicGraph<Vertex, Edge>::get_degree(VertexIndex vertex) const
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
        static_assert(dependent_false<Direction>::value, "DynamicGraph<Vertex, Edge>::get_degree(...): Missing implementation for IsTraversalDirection.");
    }
}

}

#endif
