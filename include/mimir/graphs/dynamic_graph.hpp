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

#include "mimir/graphs/dynamic_graph_iterators.hpp"
#include "mimir/graphs/graph_edge_interface.hpp"
#include "mimir/graphs/graph_interface.hpp"
#include "mimir/graphs/graph_vertex_interface.hpp"

#include <ranges>
#include <span>
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
    using VertexList = std::vector<Vertex>;
    using EdgeType = Edge;
    using EdgeList = std::vector<Edge>;

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

    const VertexList& get_vertices() const;
    const EdgeList& get_edges() const;
    size_t get_num_vertices() const;
    size_t get_num_edges() const;
    template<IsTraversalDirection Direction>
    const DegreeList& get_degrees() const;
    template<IsTraversalDirection Direction>
    Degree get_degree(VertexIndex vertex) const;

private:
    std::unordered_map<VertexIndex, Vertex> m_vertices;
    std::stack<VertexIndex> m_free_vertices;
    std::unordered_map<EdgeIndex, Edge> m_edges;
    std::stack<EdgeIndex> m_free_edges;

    std::unordered_map<VertexIndex, std::unordered_set<EdgeIndex>> m_forward_adjacent_edges;
    std::unordered_map<VertexIndex, std::unordered_set<EdgeIndex>> m_backward_adjacent_edges;

    std::unordered_map<VertexIndex, Degree> m_in_degrees;
    std::unordered_map<VertexIndex, Degree> m_out_degrees;
};

/**
 * Implementations
 */

/* DynamicGraph */

}

#endif
