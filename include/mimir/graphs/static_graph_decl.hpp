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

#ifndef MIMIR_GRAPHS_STATIC_GRAPH_DECL_HPP_
#define MIMIR_GRAPHS_STATIC_GRAPH_DECL_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/common/grouped_vector.hpp"
#include "mimir/graphs/graph_edge_interface.hpp"
#include "mimir/graphs/graph_edges.hpp"
#include "mimir/graphs/graph_vertex_interface.hpp"
#include "mimir/graphs/graph_vertices.hpp"
#include "mimir/graphs/static_graph_interface.hpp"

#include <boost/hana.hpp>
#include <ranges>
#include <span>
#include <vector>

namespace mimir::graphs
{

/**
 * Declarations
 */

/* StaticGraph */

/// @brief `StaticGraph` implements a directed graph with vertices of type V and edges of type E that supports the insertion but not the deletion of vertices
/// and edges satisfying the graph concepts: `IsVertexListGraph`, `IsEdgeListGraph`, `IsIncidenceGraph`, `IsAdjacencyGraph`, and `IsBidirectionalGraph`.
///
/// `StaticGraph` provides functionality for traversing adjacent vertices and edges in forward and backward directions. The iterators filter adjacent vertices
/// or edges of a vertex from all edges. A `StaticGraph` can be translated into a `StaticForwardGraph` or `StaticBidirectionalGraph` for efficient forward, or
/// respectively, bidirectional traversal.
/// @tparam V is the vertex type.
/// @tparam E is the edge type.
template<IsVertex V, IsEdge E>
class StaticGraph
{
public:
    using VertexType = V;
    using VertexList = std::vector<V>;
    using EdgeType = E;
    using EdgeList = std::vector<E>;

    using VertexIndexConstIterator = std::ranges::iterator_t<std::ranges::iota_view<VertexIndex, VertexIndex>>;
    using EdgeIndexConstIterator = std::ranges::iterator_t<std::ranges::iota_view<EdgeIndex, EdgeIndex>>;

    /* Iterator types. */

    template<IsDirection Direction>
    class AdjacentVertexConstIterator
    {
    private:
        size_t m_pos;
        VertexIndex m_vertex;
        const std::vector<V>* m_vertices;
        const std::vector<E>* m_edges;
        std::span<const EdgeIndex> m_slice;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = V;
        using pointer = value_type*;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        AdjacentVertexConstIterator();
        AdjacentVertexConstIterator(VertexIndex vertex,
                                    const std::vector<V>& vertices,
                                    const std::vector<E>& edges,
                                    std::span<const EdgeIndex> slice,
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
        VertexIndex m_vertex;
        const std::vector<E>* m_edges;
        std::span<const EdgeIndex> m_slice;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = VertexIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        AdjacentVertexIndexConstIterator();
        AdjacentVertexIndexConstIterator(VertexIndex vertex, const std::vector<E>& edges, std::span<const EdgeIndex> slice, bool begin);
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
        VertexIndex m_vertex;
        const std::vector<E>* m_edges;
        std::span<const EdgeIndex> m_slice;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = E;
        using pointer = value_type*;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        AdjacentEdgeConstIterator();
        AdjacentEdgeConstIterator(VertexIndex vertex, const std::vector<E>& edges, std::span<const EdgeIndex> slice, bool begin);
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
        VertexIndex m_vertex;
        const std::vector<E>* m_edges;
        std::span<const EdgeIndex> m_slice;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = EdgeIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        AdjacentEdgeIndexConstIterator();
        AdjacentEdgeIndexConstIterator(VertexIndex vertex, const std::vector<E>& edges, std::span<const EdgeIndex> slice, bool begin);
        value_type operator*() const;
        AdjacentEdgeIndexConstIterator& operator++();
        AdjacentEdgeIndexConstIterator operator++(int);
        bool operator==(const AdjacentEdgeIndexConstIterator& other) const;
        bool operator!=(const AdjacentEdgeIndexConstIterator& other) const;
    };

    /// @brief Construct an empty graph.
    StaticGraph();

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
    /// @tparam ...EdgeProperties the types of the edge properties. Must match the properties mentioned in the edge constructor.
    /// @param source the source vertex.
    /// @param target the target vertex.
    /// @param ...properties the edge properties.
    /// @return the index pair of the two newly created edges.
    template<typename... EdgeProperties>
        requires HasEdgeProperties<E, EdgeProperties...>
    std::pair<EdgeIndex, EdgeIndex> add_undirected_edge(VertexIndex source, VertexIndex target, EdgeProperties&&... properties);

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

    const VertexList& get_vertices() const;
    const V& get_vertex(VertexIndex vertex) const;
    const EdgeList& get_edges() const;
    const E& get_edge(EdgeIndex edge) const;
    size_t get_num_vertices() const;
    size_t get_num_edges() const;

    template<IsDirection>
    VertexIndex get_source(EdgeIndex edge) const;
    template<IsDirection>
    VertexIndex get_target(EdgeIndex edge) const;
    template<IsDirection Direction>
    const DegreeList& get_degrees() const;
    template<IsDirection Direction>
    Degree get_degree(VertexIndex vertex) const;

private:
    VertexList m_vertices;
    EdgeList m_edges;

    using TraversalDirectionToDegrees =
        boost::hana::map<boost::hana::pair<boost::hana::type<ForwardTag>, DegreeList>, boost::hana::pair<boost::hana::type<BackwardTag>, DegreeList>>;

    TraversalDirectionToDegrees m_degrees;

    // Slice over all edge indices for using the iterators.
    EdgeIndexList m_slice;

    /**
     * Error handling
     */

    void vertex_index_check(VertexIndex vertex, const std::string& error_message) const;
    void edge_index_check(EdgeIndex edge, const std::string& error_message) const;
};

/* StaticForwardGraph */

/// @brief `StaticForwardGraph` is a translated `StaticGraph` extended with efficient forward traversal.
template<IsStaticGraph G>
class StaticForwardGraph
{
public:
    using VertexType = typename G::VertexType;
    using EdgeType = typename G::EdgeType;
    using VertexList = std::vector<VertexType>;
    using EdgeList = std::vector<EdgeType>;

    using VertexIndexConstIterator = typename G::VertexIndexConstIterator;
    using EdgeIndexConstIterator = typename G::EdgeIndexConstIterator;

    template<IsDirection Direction>
    using AdjacentVertexConstIterator = typename G::template AdjacentVertexConstIterator<Direction>;
    template<IsDirection Direction>
    using AdjacentVertexIndexConstIterator = typename G::template AdjacentVertexIndexConstIterator<Direction>;
    template<IsDirection Direction>
    using AdjacentEdgeConstIterator = typename G::template AdjacentEdgeConstIterator<Direction>;
    template<IsDirection Direction>
    using AdjacentEdgeIndexConstIterator = typename G::template AdjacentEdgeIndexConstIterator<Direction>;

    StaticForwardGraph();

    explicit StaticForwardGraph(G graph);

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

    const G& get_graph() const;

    const VertexList& get_vertices() const;
    const VertexType& get_vertex(VertexIndex vertex) const;
    const EdgeList& get_edges() const;
    const EdgeType& get_edge(EdgeIndex edge) const;
    size_t get_num_vertices() const;
    size_t get_num_edges() const;

    template<IsDirection>
    VertexIndex get_source(EdgeIndex edge) const;
    template<IsDirection>
    VertexIndex get_target(EdgeIndex edge) const;
    template<IsDirection Direction>
    const DegreeList& get_degrees() const;
    template<IsDirection Direction>
    Degree get_degree(VertexIndex vertex) const;

private:
    G m_graph;

    IndexGroupedVector<const EdgeIndex> m_edge_indices_grouped_by_source;
};

/* BidirectionalGraph */

/// @brief `StaticBidirectionalGraph` is a translated `StaticGraph` extended with efficient bidirectional traversal.
template<IsStaticGraph G>
class StaticBidirectionalGraph
{
public:
    using VertexType = typename G::VertexType;
    using EdgeType = typename G::EdgeType;
    using VertexList = std::vector<VertexType>;
    using EdgeList = std::vector<EdgeType>;

    using VertexIndexConstIterator = typename G::VertexIndexConstIterator;
    using EdgeIndexConstIterator = typename G::EdgeIndexConstIterator;

    template<IsDirection Direction>
    using AdjacentVertexConstIterator = typename G::template AdjacentVertexConstIterator<Direction>;
    template<IsDirection Direction>
    using AdjacentVertexIndexConstIterator = typename G::template AdjacentVertexIndexConstIterator<Direction>;
    template<IsDirection Direction>
    using AdjacentEdgeConstIterator = typename G::template AdjacentEdgeConstIterator<Direction>;
    template<IsDirection Direction>
    using AdjacentEdgeIndexConstIterator = typename G::template AdjacentEdgeIndexConstIterator<Direction>;

    StaticBidirectionalGraph();

    explicit StaticBidirectionalGraph(G graph);

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

    const G& get_graph() const;

    const VertexList& get_vertices() const;
    const VertexType& get_vertex(VertexIndex vertex) const;
    const EdgeList& get_edges() const;
    const EdgeType& get_edge(EdgeIndex edge) const;
    size_t get_num_vertices() const;
    size_t get_num_edges() const;

    template<IsDirection>
    VertexIndex get_source(EdgeIndex edge) const;
    template<IsDirection>
    VertexIndex get_target(EdgeIndex edge) const;
    template<IsDirection Direction>
    const DegreeList& get_degrees() const;
    template<IsDirection Direction>
    Degree get_degree(VertexIndex vertex) const;

private:
    G m_graph;

    using TraversalDirectionToEdgesGroupedByVertex = boost::hana::map<boost::hana::pair<boost::hana::type<ForwardTag>, IndexGroupedVector<const EdgeIndex>>,
                                                                      boost::hana::pair<boost::hana::type<BackwardTag>, IndexGroupedVector<const EdgeIndex>>>;

    TraversalDirectionToEdgesGroupedByVertex m_edge_indices_grouped_by_vertex;
};

template<IsStaticGraph G>
std::ostream& operator<<(std::ostream& out, const StaticForwardGraph<G>& graph);

template<IsStaticGraph G>
std::ostream& operator<<(std::ostream& out, const StaticBidirectionalGraph<G>& graph);

}

#endif
