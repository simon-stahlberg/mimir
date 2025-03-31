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

#ifndef MIMIR_GRAPHS_DYNAMIC_GRAPH_DECL_HPP_
#define MIMIR_GRAPHS_DYNAMIC_GRAPH_DECL_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/graphs/dynamic_graph_interface.hpp"
#include "mimir/graphs/graph_edge_interface.hpp"
#include "mimir/graphs/graph_edges.hpp"
#include "mimir/graphs/graph_vertex_interface.hpp"
#include "mimir/graphs/graph_vertices.hpp"

#include <boost/hana.hpp>
#include <iostream>
#include <ranges>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mimir::graphs
{

/* DynamicGraph */

/// @brief `DynamicGraph` implements a directed graph with vertices of type V and edges of type E that supports the insertion and deletion of vertices
/// and edges satisfying the graph concepts: `IsVertexListGraph`, `IsEdgeListGraph`, `IsIncidenceGraph`, `IsAdjacencyGraph`, and `IsBidirectionalGraph`.
///
/// `DynamicGraph` supports efficient traversal of adjacent vertices and edges in forward and backward directions.
/// @tparam V is the vertex type.
/// @tparam E is the edge type.
template<IsVertex V, IsEdge E>
class DynamicGraph
{
public:
    using VertexType = V;
    using VertexMap = std::unordered_map<VertexIndex, V>;
    using EdgeType = E;
    using EdgeMap = std::unordered_map<EdgeIndex, E>;

    /* Iterator types. */

    class VertexIndexConstIterator
    {
    private:
        size_t m_pos;
        const std::unordered_map<VertexIndex, V>* m_vertices;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = VertexIndex;
        using pointer = value_type*;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        VertexIndexConstIterator();
        VertexIndexConstIterator(const std::unordered_map<VertexIndex, V>& vertices, bool begin);
        value_type operator*() const;
        VertexIndexConstIterator& operator++();
        VertexIndexConstIterator operator++(int);
        bool operator==(const VertexIndexConstIterator& other) const;
        bool operator!=(const VertexIndexConstIterator& other) const;
    };

    class EdgeIndexConstIterator
    {
    private:
        size_t m_pos;
        const std::unordered_map<EdgeIndex, E>* m_edges;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = EdgeIndex;
        using pointer = value_type*;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        EdgeIndexConstIterator();
        EdgeIndexConstIterator(const std::unordered_map<EdgeIndex, E>& edges, bool begin);
        value_type operator*() const;
        EdgeIndexConstIterator& operator++();
        EdgeIndexConstIterator operator++(int);
        bool operator==(const EdgeIndexConstIterator& other) const;
        bool operator!=(const EdgeIndexConstIterator& other) const;
    };

    template<IsDirection Direction>
    class AdjacentVertexConstIterator
    {
    private:
        const std::unordered_map<VertexIndex, V>* m_vertices;
        const std::unordered_map<EdgeIndex, E>* m_edges;
        std::unordered_set<EdgeIndex>::const_iterator m_slice_iter;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = const V;
        using pointer = value_type*;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        AdjacentVertexConstIterator();
        AdjacentVertexConstIterator(const std::unordered_map<VertexIndex, V>& vertices,
                                    const std::unordered_map<EdgeIndex, E>& edges,
                                    const std::unordered_set<EdgeIndex>& slice,
                                    bool begin);
        reference operator*() const;
        AdjacentVertexConstIterator& operator++();
        AdjacentVertexConstIterator operator++(int);
        bool operator==(const AdjacentVertexConstIterator& other) const;
        bool operator!=(const AdjacentVertexConstIterator& other) const;
    };

    template<IsDirection Direction>
    class AdjacentVertexIndexConstIterator
    {
    private:
        size_t m_pos;
        const std::unordered_map<EdgeIndex, E>* m_edges;
        std::unordered_set<EdgeIndex>::const_iterator m_slice_iter;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = VertexIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        AdjacentVertexIndexConstIterator();
        AdjacentVertexIndexConstIterator(const std::unordered_map<EdgeIndex, E>& edges, const std::unordered_set<EdgeIndex>& slice, bool begin);
        value_type operator*() const;
        AdjacentVertexIndexConstIterator& operator++();
        AdjacentVertexIndexConstIterator operator++(int);
        bool operator==(const AdjacentVertexIndexConstIterator& other) const;
        bool operator!=(const AdjacentVertexIndexConstIterator& other) const;
    };

    template<IsDirection Direction>
    class AdjacentEdgeConstIterator
    {
    private:
        size_t m_pos;
        const std::unordered_map<EdgeIndex, E>* m_edges;
        std::unordered_set<EdgeIndex>::const_iterator m_slice_iter;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = E;
        using pointer = value_type*;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        AdjacentEdgeConstIterator();
        AdjacentEdgeConstIterator(const std::unordered_map<EdgeIndex, E>& edges, const std::unordered_set<EdgeIndex>& slice, bool begin);
        reference operator*() const;
        AdjacentEdgeConstIterator& operator++();
        AdjacentEdgeConstIterator operator++(int);
        bool operator==(const AdjacentEdgeConstIterator& other) const;
        bool operator!=(const AdjacentEdgeConstIterator& other) const;
    };

    template<IsDirection Direction>
    class AdjacentEdgeIndexConstIterator
    {
    private:
        size_t m_pos;
        const std::unordered_map<EdgeIndex, E>* m_edges;
        std::unordered_set<EdgeIndex>::const_iterator m_slice_iter;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = EdgeIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        AdjacentEdgeIndexConstIterator();
        AdjacentEdgeIndexConstIterator(const std::unordered_map<EdgeIndex, E>& edges, const std::unordered_set<EdgeIndex>& slice, bool begin);
        value_type operator*() const;
        AdjacentEdgeIndexConstIterator& operator++();
        AdjacentEdgeIndexConstIterator operator++(int);
        bool operator==(const AdjacentEdgeIndexConstIterator& other) const;
        bool operator!=(const AdjacentEdgeIndexConstIterator& other) const;
    };

    /// @brief Construct an empty graph.
    DynamicGraph();

    /// @brief Reinitialize the graph to an empty graph.
    void clear();

    /**
     * Constructible functionality.
     */

    /// @brief Add a vertex to the graph with vertex properties args.
    /// @tparam ...VertexProperties the types of the vertex properties. Must match the properties mentioned in the vertex constructor.
    /// @param ...properties the vertex properties.
    /// @return the index of the newly created vertex.
    template<typename... VertexProperties>
        requires HasVertexProperties<V, VertexProperties...>
    VertexIndex add_vertex(VertexProperties&&... properties);

    /// @brief Add a vertex to the graph that has the same properties as the given vertex.
    /// @tparam ...VertexProperties the types of the vertex properties. Must match the properties mentioned in the vertex constructor.
    /// @param vertex is the given vertex.
    /// @return the index of the newly created vertex.
    template<typename... VertexProperties>
        requires HasVertexProperties<V, VertexProperties...>
    VertexIndex add_vertex(const Vertex<VertexProperties...>& vertex);

    /// @brief Add a directed edge from source to target to the graph with edge properties args.
    /// @tparam ...EdgeProperties the types of the edge properties. Must match the properties mentioned in the edge constructor.
    /// @param source the source vertex.
    /// @param target the target vertex.
    /// @param ...properties the edge properties.
    /// @return the index of the newly created edge.
    template<typename... EdgeProperties>
        requires HasEdgeProperties<E, EdgeProperties...>
    EdgeIndex add_directed_edge(VertexIndex source, VertexIndex target, EdgeProperties&&... properties);

    /// @brief Add an edge to the graph that has the same properties as the given edge.
    /// @tparam ...EdgeProperties the types of the edge properties. Must match the properties mentioned in the edge constructor.
    /// @param source the source vertex.
    /// @param target the target vertex.
    /// @param edge is the given edge.
    /// @return the index of the newly created edge.
    template<typename... EdgeProperties>
        requires HasEdgeProperties<E, EdgeProperties...>
    EdgeIndex add_directed_edge(VertexIndex source, VertexIndex target, const Edge<EdgeProperties...>& edge);

    /// @brief Add two anti-parallel directed edges to the graph with the identical edge properties, representing the undirected edge.
    ///
    /// Semantics depending on the value category of a `EdgeProperties` property:
    ///   - lvalue: property is copied twice.
    ///   - rvalue: property is copied once.
    /// Therefore, if an `EdgeProperties` is heavy weight, we suggest externalizing the properties and storing an index to the properties instead.
    /// @tparam ...EdgeProperties the types of the edge properties. Must match the properties mentioned in the edge constructor.
    /// @param source the source vertex.
    /// @param target the target vertex.
    /// @param ...properties the edge properties.
    /// @return the index pair of the two newly created edges.
    template<typename... EdgeProperties>
        requires HasEdgeProperties<E, EdgeProperties...>
    std::pair<EdgeIndex, EdgeIndex> add_undirected_edge(VertexIndex source, VertexIndex target, EdgeProperties&&... properties);

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
     * Subgraph
     */

    DynamicGraph compute_induced_subgraph(const VertexIndexList& vertices) const;

    /**
     * Iterators
     */

    std::ranges::subrange<VertexIndexConstIterator> get_vertex_indices() const;
    std::ranges::subrange<EdgeIndexConstIterator> get_edge_indices() const;

    template<IsDirection Direction>
    std::ranges::subrange<AdjacentVertexConstIterator<Direction>> get_adjacent_vertices(VertexIndex vertex) const;
    template<IsDirection Direction>
    std::ranges::subrange<AdjacentVertexIndexConstIterator<Direction>> get_adjacent_vertex_indices(VertexIndex vertex) const;
    template<IsDirection Direction>
    std::ranges::subrange<AdjacentEdgeConstIterator<Direction>> get_adjacent_edges(VertexIndex vertex) const;
    template<IsDirection Direction>
    std::ranges::subrange<AdjacentEdgeIndexConstIterator<Direction>> get_adjacent_edge_indices(VertexIndex vertex) const;

    /**
     * Getters
     */

    const VertexMap& get_vertices() const;
    const V& get_vertex(VertexIndex vertex) const;
    const EdgeMap& get_edges() const;
    const E& get_edge(EdgeIndex edge) const;
    size_t get_num_vertices() const;
    size_t get_num_edges() const;

    template<IsDirection>
    VertexIndex get_source(EdgeIndex edge) const;
    template<IsDirection>
    VertexIndex get_target(EdgeIndex edge) const;
    template<IsDirection Direction>
    const DegreeMap& get_degrees() const;
    template<IsDirection Direction>
    Degree get_degree(VertexIndex vertex) const;

private:
    VertexMap m_vertices;
    VertexIndexList m_free_vertices;
    size_t m_next_vertex_index;
    EdgeMap m_edges;
    EdgeIndexList m_free_edges;
    size_t m_next_edge_index;

    using TraversalDirectionToAdjacentEdges =
        boost::hana::map<boost::hana::pair<boost::hana::type<ForwardTag>, std::unordered_map<VertexIndex, EdgeIndexSet>>,
                         boost::hana::pair<boost::hana::type<BackwardTag>, std::unordered_map<VertexIndex, EdgeIndexSet>>>;

    TraversalDirectionToAdjacentEdges m_adjacent_edges;

    using TraversalDirectionToDegrees =
        boost::hana::map<boost::hana::pair<boost::hana::type<ForwardTag>, DegreeMap>, boost::hana::pair<boost::hana::type<BackwardTag>, DegreeMap>>;

    TraversalDirectionToDegrees m_degrees;

    /**
     * Error handling
     */

    void vertex_index_check(VertexIndex vertex, const std::string& error_message) const;
    void edge_index_check(EdgeIndex edge, const std::string& error_message) const;
};

template<IsVertex V, IsEdge E>
std::ostream& operator<<(std::ostream& out, const DynamicGraph<V, E>& graph);

}

#endif
