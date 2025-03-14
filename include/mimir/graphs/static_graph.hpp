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
#include "mimir/graphs/graph_edges.hpp"
#include "mimir/graphs/graph_vertex_interface.hpp"
#include "mimir/graphs/graph_vertices.hpp"
#include "mimir/graphs/static_graph_interface.hpp"
#include "mimir/graphs/static_graph_iterators.hpp"

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
/// and edges satisfying the graph concepts: `VertexListGraph`, `EdgeListGraph`, `IncidenceGraph`, and `AdjacencyGraph`.
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

    using VertexIndexConstIteratorType = std::ranges::iterator_t<std::ranges::iota_view<VertexIndex, VertexIndex>>;
    using EdgeIndexConstIteratorType = std::ranges::iterator_t<std::ranges::iota_view<EdgeIndex, EdgeIndex>>;

    template<IsDirection Direction>
    using AdjacentVertexConstIteratorType = StaticAdjacentVertexConstIterator<V, E, Direction>;
    template<IsDirection Direction>
    using AdjacentVertexIndexConstIteratorType = StaticAdjacentVertexIndexConstIterator<E, Direction>;
    template<IsDirection Direction>
    using AdjacentEdgeConstIteratorType = StaticAdjacentEdgeConstIterator<E, Direction>;
    template<IsDirection Direction>
    using AdjacentEdgeIndexConstIteratorType = StaticAdjacentEdgeIndexConstIterator<E, Direction>;

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

    std::ranges::subrange<VertexIndexConstIteratorType> get_vertex_indices() const;
    std::ranges::subrange<EdgeIndexConstIteratorType> get_edge_indices() const;

    template<IsDirection Direction>
    std::ranges::subrange<AdjacentVertexConstIteratorType<Direction>> get_adjacent_vertices(VertexIndex vertex) const;
    template<IsDirection Direction>
    std::ranges::subrange<AdjacentVertexIndexConstIteratorType<Direction>> get_adjacent_vertex_indices(VertexIndex vertex) const;
    template<IsDirection Direction>
    std::ranges::subrange<AdjacentEdgeConstIteratorType<Direction>> get_adjacent_edges(VertexIndex vertex) const;
    template<IsDirection Direction>
    std::ranges::subrange<AdjacentEdgeIndexConstIteratorType<Direction>> get_adjacent_edge_indices(VertexIndex vertex) const;

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
        boost::hana::map<boost::hana::pair<boost::hana::type<Forward>, DegreeList>, boost::hana::pair<boost::hana::type<Backward>, DegreeList>>;

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

    using VertexIndexConstIteratorType = typename G::VertexIndexConstIteratorType;
    using EdgeIndexConstIteratorType = typename G::EdgeIndexConstIteratorType;

    template<IsDirection Direction>
    using AdjacentVertexConstIteratorType = typename G::template AdjacentVertexConstIteratorType<Direction>;
    template<IsDirection Direction>
    using AdjacentVertexIndexConstIteratorType = typename G::template AdjacentVertexIndexConstIteratorType<Direction>;
    template<IsDirection Direction>
    using AdjacentEdgeConstIteratorType = typename G::template AdjacentEdgeConstIteratorType<Direction>;
    template<IsDirection Direction>
    using AdjacentEdgeIndexConstIteratorType = typename G::template AdjacentEdgeIndexConstIteratorType<Direction>;

    StaticForwardGraph();

    explicit StaticForwardGraph(G graph);

    /**
     * Iterators
     */

    std::ranges::subrange<VertexIndexConstIteratorType> get_vertex_indices() const;
    std::ranges::subrange<EdgeIndexConstIteratorType> get_edge_indices() const;

    template<IsDirection Direction>
    std::ranges::subrange<AdjacentVertexConstIteratorType<Direction>> get_adjacent_vertices(VertexIndex vertex) const;
    template<IsDirection Direction>
    std::ranges::subrange<AdjacentVertexIndexConstIteratorType<Direction>> get_adjacent_vertex_indices(VertexIndex vertex) const;
    template<IsDirection Direction>
    std::ranges::subrange<AdjacentEdgeConstIteratorType<Direction>> get_adjacent_edges(VertexIndex vertex) const;
    template<IsDirection Direction>
    std::ranges::subrange<AdjacentEdgeIndexConstIteratorType<Direction>> get_adjacent_edge_indices(VertexIndex vertex) const;

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

    using VertexIndexConstIteratorType = typename G::VertexIndexConstIteratorType;
    using EdgeIndexConstIteratorType = typename G::EdgeIndexConstIteratorType;

    template<IsDirection Direction>
    using AdjacentVertexConstIteratorType = typename G::template AdjacentVertexConstIteratorType<Direction>;
    template<IsDirection Direction>
    using AdjacentVertexIndexConstIteratorType = typename G::template AdjacentVertexIndexConstIteratorType<Direction>;
    template<IsDirection Direction>
    using AdjacentEdgeConstIteratorType = typename G::template AdjacentEdgeConstIteratorType<Direction>;
    template<IsDirection Direction>
    using AdjacentEdgeIndexConstIteratorType = typename G::template AdjacentEdgeIndexConstIteratorType<Direction>;

    StaticBidirectionalGraph();

    explicit StaticBidirectionalGraph(G graph);

    /**
     * Iterators
     */

    std::ranges::subrange<VertexIndexConstIteratorType> get_vertex_indices() const;
    std::ranges::subrange<EdgeIndexConstIteratorType> get_edge_indices() const;

    template<IsDirection Direction>
    std::ranges::subrange<AdjacentVertexConstIteratorType<Direction>> get_adjacent_vertices(VertexIndex vertex) const;
    template<IsDirection Direction>
    std::ranges::subrange<AdjacentVertexIndexConstIteratorType<Direction>> get_adjacent_vertex_indices(VertexIndex vertex) const;
    template<IsDirection Direction>
    std::ranges::subrange<AdjacentEdgeConstIteratorType<Direction>> get_adjacent_edges(VertexIndex vertex) const;
    template<IsDirection Direction>
    std::ranges::subrange<AdjacentEdgeIndexConstIteratorType<Direction>> get_adjacent_edge_indices(VertexIndex vertex) const;

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

    using TraversalDirectionToEdgesGroupedByVertex = boost::hana::map<boost::hana::pair<boost::hana::type<Forward>, IndexGroupedVector<const EdgeIndex>>,
                                                                      boost::hana::pair<boost::hana::type<Backward>, IndexGroupedVector<const EdgeIndex>>>;

    TraversalDirectionToEdgesGroupedByVertex m_edge_indices_grouped_by_vertex;
};

/**
 * Implementations
 */

/* StaticGraph */

template<IsVertex V, IsEdge E>
StaticGraph<V, E>::StaticGraph() : m_vertices(), m_edges(), m_degrees()
{
}

template<IsVertex V, IsEdge E>
template<typename... VertexProperties>
    requires HasVertexProperties<V, VertexProperties...>
VertexIndex StaticGraph<V, E>::add_vertex(VertexProperties&&... properties)
{
    const auto index = m_vertices.size();
    m_vertices.emplace_back(index, std::forward<VertexProperties>(properties)...);

    boost::hana::at_key(m_degrees, boost::hana::type<Forward> {}).resize(index + 1, 0);
    boost::hana::at_key(m_degrees, boost::hana::type<Backward> {}).resize(index + 1, 0);

    return index;
}

template<IsVertex V, IsEdge E>
template<typename... VertexProperties>
    requires HasVertexProperties<V, VertexProperties...>
VertexIndex StaticGraph<V, E>::add_vertex(const Vertex<VertexProperties...>& vertex)
{
    return std::apply([this](auto&&... properties) { return this->add_vertex(std::forward<decltype(properties)>(properties)...); }, vertex.get_properties());
}

template<IsVertex V, IsEdge E>
template<typename... EdgeProperties>
    requires HasEdgeProperties<E, EdgeProperties...>
EdgeIndex StaticGraph<V, E>::add_directed_edge(VertexIndex source, VertexIndex target, EdgeProperties&&... properties)
{
    vertex_index_check(source, "StaticGraph<V, E>::add_directed_edge(...): Source vertex out of range");
    vertex_index_check(target, "StaticGraph<V, E>::add_directed_edge(...): Source vertex out of range");

    const auto index = m_edges.size();
    m_edges.emplace_back(index, source, target, std::forward<EdgeProperties>(properties)...);
    ++boost::hana::at_key(m_degrees, boost::hana::type<Forward> {}).at(source);
    ++boost::hana::at_key(m_degrees, boost::hana::type<Backward> {}).at(target);
    m_slice.push_back(index);

    return index;
}

template<IsVertex V, IsEdge E>
template<typename... EdgeProperties>
    requires HasEdgeProperties<E, EdgeProperties...>
EdgeIndex StaticGraph<V, E>::add_directed_edge(VertexIndex source, VertexIndex target, const Edge<EdgeProperties...>& edge)
{
    return std::apply([this, source, target](auto&&... properties)
                      { return this->add_directed_edge(source, target, std::forward<decltype(properties)>(properties)...); },
                      edge.get_properties());
}

template<IsVertex V, IsEdge E>
template<typename... EdgeProperties>
    requires HasEdgeProperties<E, EdgeProperties...>
std::pair<EdgeIndex, EdgeIndex> StaticGraph<V, E>::add_undirected_edge(VertexIndex source, VertexIndex target, EdgeProperties&&... properties)
{
    auto properties_tuple = std::make_tuple(std::forward<EdgeProperties>(properties)...);
    auto properties_tuple_copy = properties_tuple;

    const auto forward_edge_index =
        std::apply([this, source, target](auto&&... args) { return this->add_directed_edge(source, target, std::forward<decltype(args)>(args)...); },
                   std::move(properties_tuple_copy));
    const auto backward_edge_index =
        std::apply([this, source, target](auto&&... args) { return this->add_directed_edge(target, source, std::forward<decltype(args)>(args)...); },
                   std::move(properties_tuple));

    return std::make_pair(forward_edge_index, backward_edge_index);
}

template<IsVertex V, IsEdge E>
void StaticGraph<V, E>::clear()
{
    m_vertices.clear();
    m_edges.clear();
    boost::hana::at_key(m_degrees, boost::hana::type<Forward> {}).clear();
    boost::hana::at_key(m_degrees, boost::hana::type<Backward> {}).clear();
    m_slice.clear();
}

template<IsVertex V, IsEdge E>
std::ranges::subrange<typename StaticGraph<V, E>::VertexIndexConstIteratorType> StaticGraph<V, E>::get_vertex_indices() const
{
    auto range = std::ranges::iota_view<VertexIndex, VertexIndex>(0, get_num_vertices());
    static_assert(std::ranges::borrowed_range<decltype(range)>);
    return std::ranges::subrange<VertexIndexConstIteratorType>(range.begin(), range.end());
}

template<IsVertex V, IsEdge E>
std::ranges::subrange<typename StaticGraph<V, E>::EdgeIndexConstIteratorType> StaticGraph<V, E>::get_edge_indices() const
{
    auto range = std::ranges::iota_view<EdgeIndex, EdgeIndex>(0, get_num_edges());
    static_assert(std::ranges::borrowed_range<decltype(range)>);
    return std::ranges::subrange<EdgeIndexConstIteratorType>(range.begin(), range.end());
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
std::ranges::subrange<typename StaticGraph<V, E>::template AdjacentVertexConstIteratorType<Direction>>
StaticGraph<V, E>::get_adjacent_vertices(VertexIndex vertex) const
{
    vertex_index_check(vertex, "StaticGraph<V, E>::get_adjacent_vertices(...): Vertex out of range");

    return std::ranges::subrange(typename StaticGraph<V, E>::AdjacentVertexConstIteratorType<Direction>(vertex, m_vertices, m_edges, m_slice, true),
                                 typename StaticGraph<V, E>::AdjacentVertexConstIteratorType<Direction>(vertex, m_vertices, m_edges, m_slice, false));
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
std::ranges::subrange<typename StaticGraph<V, E>::template AdjacentVertexIndexConstIteratorType<Direction>>
StaticGraph<V, E>::get_adjacent_vertex_indices(VertexIndex vertex) const
{
    vertex_index_check(vertex, "StaticGraph<V, E>::get_adjacent_vertex_indices(...): Vertex out of range");

    return std::ranges::subrange(typename StaticGraph<V, E>::AdjacentVertexIndexConstIteratorType<Direction>(vertex, m_edges, m_slice, true),
                                 typename StaticGraph<V, E>::AdjacentVertexIndexConstIteratorType<Direction>(vertex, m_edges, m_slice, false));
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
std::ranges::subrange<typename StaticGraph<V, E>::template AdjacentEdgeConstIteratorType<Direction>>
StaticGraph<V, E>::get_adjacent_edges(VertexIndex vertex) const
{
    vertex_index_check(vertex, "StaticGraph<V, E>::get_adjacent_edges(...): Vertex out of range");

    return std::ranges::subrange(typename StaticGraph<V, E>::AdjacentEdgeConstIteratorType<Direction>(vertex, m_edges, m_slice, true),
                                 typename StaticGraph<V, E>::AdjacentEdgeConstIteratorType<Direction>(vertex, m_edges, m_slice, false));
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
std::ranges::subrange<typename StaticGraph<V, E>::template AdjacentEdgeIndexConstIteratorType<Direction>>
StaticGraph<V, E>::get_adjacent_edge_indices(VertexIndex vertex) const
{
    vertex_index_check(vertex, "StaticGraph<V, E>::get_adjacent_edge_indices(...): Vertex out of range");

    return std::ranges::subrange(typename StaticGraph<V, E>::AdjacentEdgeIndexConstIteratorType<Direction>(vertex, m_edges, m_slice, true),
                                 typename StaticGraph<V, E>::AdjacentEdgeIndexConstIteratorType<Direction>(vertex, m_edges, m_slice, false));
}

template<IsVertex V, IsEdge E>
const StaticGraph<V, E>::VertexList& StaticGraph<V, E>::get_vertices() const
{
    return m_vertices;
}

template<IsVertex V, IsEdge E>
const V& StaticGraph<V, E>::get_vertex(VertexIndex vertex) const
{
    vertex_index_check(vertex, "StaticGraph<V, E>::get_vertex(...): Vertex does not exist.");

    return m_vertices[vertex];
}

template<IsVertex V, IsEdge E>
const StaticGraph<V, E>::EdgeList& StaticGraph<V, E>::get_edges() const
{
    return m_edges;
}

template<IsVertex V, IsEdge E>
const E& StaticGraph<V, E>::get_edge(EdgeIndex edge) const
{
    edge_index_check(edge, "StaticGraph<V, E>::get_edge(...): Edge does not exist.");

    return m_edges[edge];
}

template<IsVertex V, IsEdge E>
size_t StaticGraph<V, E>::get_num_vertices() const
{
    return m_vertices.size();
}

template<IsVertex V, IsEdge E>
size_t StaticGraph<V, E>::get_num_edges() const
{
    return m_edges.size();
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
VertexIndex StaticGraph<V, E>::get_source(EdgeIndex edge) const
{
    edge_index_check(edge, "StaticGraph<V, E>::get_source(...): Edge out of range");

    if constexpr (std::is_same_v<Direction, Forward>)
    {
        return m_edges[edge].get_source();
    }
    else if constexpr (std::is_same_v<Direction, Backward>)
    {
        return m_edges[edge].get_target();
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "StaticGraph<V, E>::get_source(...): Missing implementation for IsDirection.");
    }
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
VertexIndex StaticGraph<V, E>::get_target(EdgeIndex edge) const
{
    edge_index_check(edge, "StaticGraph<V, E>::get_target(...): Edge out of range");

    if constexpr (std::is_same_v<Direction, Forward>)
    {
        return m_edges[edge].get_target();
    }
    else if constexpr (std::is_same_v<Direction, Backward>)
    {
        return m_edges[edge].get_source();
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "StaticGraph<V, E>::get_target(...): Missing implementation for IsDirection.");
    }
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
const DegreeList& StaticGraph<V, E>::get_degrees() const
{
    return boost::hana::at_key(m_degrees, boost::hana::type<Direction> {});
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
Degree StaticGraph<V, E>::get_degree(VertexIndex vertex) const
{
    vertex_index_check(vertex, "StaticGraph<V, E>::get_degree(...): Vertex out of range");

    return boost::hana::at_key(m_degrees, boost::hana::type<Direction> {}).at(vertex);
}

template<IsVertex V, IsEdge E>
void StaticGraph<V, E>::vertex_index_check(VertexIndex vertex, const std::string& error_message) const
{
    if (vertex >= get_num_vertices() || vertex < 0)
    {
        throw std::out_of_range(error_message);
    }
}

template<IsVertex V, IsEdge E>
void StaticGraph<V, E>::edge_index_check(EdgeIndex edge, const std::string& error_message) const
{
    if (edge >= get_num_edges() || edge < 0)
    {
        throw std::out_of_range(error_message);
    }
}

/* StaticForwardGraph */

/// @brief Groups edge indices by source or target, depending on forward is true or false.
/// @tparam Vertex is the type of vertices in the graph.
/// @tparam Edge is the type of edges in the graph.
/// @param graph is the graph.
/// @param forward true will group by source and false will group by target.
/// @return
template<IsVertex V, IsEdge E>
static IndexGroupedVector<const EdgeIndex> compute_index_grouped_edge_indices(const StaticGraph<V, E>& graph, bool forward)
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
StaticForwardGraph<G>::StaticForwardGraph() : m_graph(G()), m_edge_indices_grouped_by_source()
{
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
template<IsDirection Direction>
std::ranges::subrange<typename StaticForwardGraph<G>::template AdjacentVertexConstIteratorType<Direction>>
StaticForwardGraph<G>::get_adjacent_vertices(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, Forward>)
    {
        return std::ranges::subrange(typename StaticForwardGraph<G>::AdjacentVertexConstIteratorType<Forward>(vertex,
                                                                                                              m_graph.get_vertices(),
                                                                                                              m_graph.get_edges(),
                                                                                                              m_edge_indices_grouped_by_source.at(vertex),
                                                                                                              true),
                                     typename StaticForwardGraph<G>::AdjacentVertexConstIteratorType<Forward>(vertex,
                                                                                                              m_graph.get_vertices(),
                                                                                                              m_graph.get_edges(),
                                                                                                              m_edge_indices_grouped_by_source.at(vertex),
                                                                                                              false));
    }
    else if constexpr (std::is_same_v<Direction, Backward>)
    {
        return m_graph.template get_adjacent_vertices<Backward>(vertex);
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "StaticForwardGraph<G>::get_adjacent_vertices: Missing implementation for IsDirection.");
    }
}

template<IsStaticGraph G>
template<IsDirection Direction>
std::ranges::subrange<typename StaticForwardGraph<G>::template AdjacentVertexIndexConstIteratorType<Direction>>
StaticForwardGraph<G>::get_adjacent_vertex_indices(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, Forward>)
    {
        return std::ranges::subrange(typename StaticForwardGraph<G>::AdjacentVertexIndexConstIteratorType<Forward>(vertex,
                                                                                                                   m_graph.get_edges(),
                                                                                                                   m_edge_indices_grouped_by_source.at(vertex),
                                                                                                                   true),
                                     typename StaticForwardGraph<G>::AdjacentVertexIndexConstIteratorType<Forward>(vertex,
                                                                                                                   m_graph.get_edges(),
                                                                                                                   m_edge_indices_grouped_by_source.at(vertex),
                                                                                                                   false));
    }
    else if constexpr (std::is_same_v<Direction, Backward>)
    {
        return m_graph.template get_adjacent_vertex_indices<Backward>(vertex);
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "StaticForwardGraph<G>::get_adjacent_vertex_indices: Missing implementation for IsDirection.");
    }
}

template<IsStaticGraph G>
template<IsDirection Direction>
std::ranges::subrange<typename StaticForwardGraph<G>::template AdjacentEdgeConstIteratorType<Direction>>
StaticForwardGraph<G>::get_adjacent_edges(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, Forward>)
    {
        return std::ranges::subrange(typename StaticForwardGraph<G>::AdjacentEdgeConstIteratorType<Forward>(vertex,
                                                                                                            m_graph.get_edges(),
                                                                                                            m_edge_indices_grouped_by_source.at(vertex),
                                                                                                            true),
                                     typename StaticForwardGraph<G>::AdjacentEdgeConstIteratorType<Forward>(vertex,
                                                                                                            m_graph.get_edges(),
                                                                                                            m_edge_indices_grouped_by_source.at(vertex),
                                                                                                            false));
    }
    else if constexpr (std::is_same_v<Direction, Backward>)
    {
        return m_graph.template get_adjacent_edges<Backward>(vertex);
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "StaticForwardGraph<G>::get_adjacent_edges: Missing implementation for IsDirection.");
    }
}

template<IsStaticGraph G>
template<IsDirection Direction>
std::ranges::subrange<typename StaticForwardGraph<G>::template AdjacentEdgeIndexConstIteratorType<Direction>>
StaticForwardGraph<G>::get_adjacent_edge_indices(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, Forward>)
    {
        return std::ranges::subrange(typename StaticForwardGraph<G>::AdjacentEdgeIndexConstIteratorType<Forward>(vertex,
                                                                                                                 m_graph.get_edges(),
                                                                                                                 m_edge_indices_grouped_by_source.at(vertex),
                                                                                                                 true),
                                     typename StaticForwardGraph<G>::AdjacentEdgeIndexConstIteratorType<Forward>(vertex,
                                                                                                                 m_graph.get_edges(),
                                                                                                                 m_edge_indices_grouped_by_source.at(vertex),
                                                                                                                 false));
    }
    else if constexpr (std::is_same_v<Direction, Backward>)
    {
        return m_graph.template get_adjacent_edge_indices<Backward>(vertex);
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "StaticForwardGraph<G>::get_adjacent_edge_indices: Missing implementation for IsDirection.");
    }
}

template<IsStaticGraph G>
const G& StaticForwardGraph<G>::get_graph() const
{
    return m_graph;
}

template<IsStaticGraph G>
const StaticForwardGraph<G>::VertexList& StaticForwardGraph<G>::get_vertices() const
{
    return m_graph.get_vertices();
}

template<IsStaticGraph G>
const StaticForwardGraph<G>::VertexType& StaticForwardGraph<G>::get_vertex(VertexIndex vertex) const
{
    return m_graph.get_vertex(vertex);
}

template<IsStaticGraph G>
const StaticForwardGraph<G>::EdgeList& StaticForwardGraph<G>::get_edges() const
{
    return m_graph.get_edges();
}

template<IsStaticGraph G>
const StaticForwardGraph<G>::EdgeType& StaticForwardGraph<G>::get_edge(EdgeIndex edge) const
{
    return m_graph.get_edge(edge);
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
template<IsDirection Direction>
VertexIndex StaticForwardGraph<G>::get_source(EdgeIndex edge) const
{
    return m_graph.template get_source<Direction>(edge);
}

template<IsStaticGraph G>
template<IsDirection Direction>
VertexIndex StaticForwardGraph<G>::get_target(EdgeIndex edge) const
{
    return m_graph.template get_target<Direction>(edge);
}

template<IsStaticGraph G>
template<IsDirection Direction>
const DegreeList& StaticForwardGraph<G>::get_degrees() const
{
    return m_graph.template get_degrees<Direction>();
}

template<IsStaticGraph G>
template<IsDirection Direction>
Degree StaticForwardGraph<G>::get_degree(VertexIndex vertex) const
{
    return m_graph.template get_degree<Direction>(vertex);
}

/* BidirectionalGraph */

template<IsStaticGraph G>
StaticBidirectionalGraph<G>::StaticBidirectionalGraph() : m_graph(G()), m_edge_indices_grouped_by_vertex()
{
}

template<IsStaticGraph G>
StaticBidirectionalGraph<G>::StaticBidirectionalGraph(G graph) : m_graph(std::move(graph)), m_edge_indices_grouped_by_vertex()
{
    boost::hana::at_key(m_edge_indices_grouped_by_vertex, boost::hana::type<Forward> {}) = std::move(compute_index_grouped_edge_indices(m_graph, true));
    boost::hana::at_key(m_edge_indices_grouped_by_vertex, boost::hana::type<Backward> {}) = std::move(compute_index_grouped_edge_indices(m_graph, false));
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
template<IsDirection Direction>
std::ranges::subrange<typename StaticBidirectionalGraph<G>::template AdjacentVertexConstIteratorType<Direction>>
StaticBidirectionalGraph<G>::get_adjacent_vertices(VertexIndex vertex) const
{
    return std::ranges::subrange(typename StaticBidirectionalGraph<G>::AdjacentVertexConstIteratorType<Direction>(
                                     vertex,
                                     m_graph.get_vertices(),
                                     m_graph.get_edges(),
                                     boost::hana::at_key(m_edge_indices_grouped_by_vertex, boost::hana::type<Direction> {}).at(vertex),
                                     true),
                                 typename StaticBidirectionalGraph<G>::AdjacentVertexConstIteratorType<Direction>(
                                     vertex,
                                     m_graph.get_vertices(),
                                     m_graph.get_edges(),
                                     boost::hana::at_key(m_edge_indices_grouped_by_vertex, boost::hana::type<Direction> {}).at(vertex),
                                     false));
}

template<IsStaticGraph G>
template<IsDirection Direction>
std::ranges::subrange<typename StaticBidirectionalGraph<G>::template AdjacentVertexIndexConstIteratorType<Direction>>
StaticBidirectionalGraph<G>::get_adjacent_vertex_indices(VertexIndex vertex) const
{
    return std::ranges::subrange(typename StaticBidirectionalGraph<G>::AdjacentVertexIndexConstIteratorType<Direction>(
                                     vertex,
                                     m_graph.get_edges(),
                                     boost::hana::at_key(m_edge_indices_grouped_by_vertex, boost::hana::type<Direction> {}).at(vertex),
                                     true),
                                 typename StaticBidirectionalGraph<G>::AdjacentVertexIndexConstIteratorType<Direction>(
                                     vertex,
                                     m_graph.get_edges(),
                                     boost::hana::at_key(m_edge_indices_grouped_by_vertex, boost::hana::type<Direction> {}).at(vertex),
                                     false));
}

template<IsStaticGraph G>
template<IsDirection Direction>
std::ranges::subrange<typename StaticBidirectionalGraph<G>::template AdjacentEdgeConstIteratorType<Direction>>
StaticBidirectionalGraph<G>::get_adjacent_edges(VertexIndex vertex) const
{
    return std::ranges::subrange(typename StaticBidirectionalGraph<G>::AdjacentEdgeConstIteratorType<Direction>(
                                     vertex,
                                     m_graph.get_edges(),
                                     boost::hana::at_key(m_edge_indices_grouped_by_vertex, boost::hana::type<Direction> {}).at(vertex),
                                     true),
                                 typename StaticBidirectionalGraph<G>::AdjacentEdgeConstIteratorType<Direction>(
                                     vertex,
                                     m_graph.get_edges(),
                                     boost::hana::at_key(m_edge_indices_grouped_by_vertex, boost::hana::type<Direction> {}).at(vertex),
                                     false));
}

template<IsStaticGraph G>
template<IsDirection Direction>
std::ranges::subrange<typename StaticBidirectionalGraph<G>::template AdjacentEdgeIndexConstIteratorType<Direction>>
StaticBidirectionalGraph<G>::get_adjacent_edge_indices(VertexIndex vertex) const
{
    return std::ranges::subrange(typename StaticBidirectionalGraph<G>::AdjacentEdgeIndexConstIteratorType<Direction>(
                                     vertex,
                                     m_graph.get_edges(),
                                     boost::hana::at_key(m_edge_indices_grouped_by_vertex, boost::hana::type<Direction> {}).at(vertex),
                                     true),
                                 typename StaticBidirectionalGraph<G>::AdjacentEdgeIndexConstIteratorType<Direction>(
                                     vertex,
                                     m_graph.get_edges(),
                                     boost::hana::at_key(m_edge_indices_grouped_by_vertex, boost::hana::type<Direction> {}).at(vertex),
                                     false));
}

template<IsStaticGraph G>
const G& StaticBidirectionalGraph<G>::get_graph() const
{
    return m_graph;
}

template<IsStaticGraph G>
const StaticBidirectionalGraph<G>::VertexList& StaticBidirectionalGraph<G>::get_vertices() const
{
    return m_graph.get_vertices();
}

template<IsStaticGraph G>
const StaticBidirectionalGraph<G>::VertexType& StaticBidirectionalGraph<G>::get_vertex(VertexIndex vertex) const
{
    return m_graph.get_vertex(vertex);
}

template<IsStaticGraph G>
const StaticBidirectionalGraph<G>::EdgeList& StaticBidirectionalGraph<G>::get_edges() const
{
    return m_graph.get_edges();
}

template<IsStaticGraph G>
const StaticBidirectionalGraph<G>::EdgeType& StaticBidirectionalGraph<G>::get_edge(EdgeIndex edge) const
{
    return m_graph.get_edge(edge);
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
template<IsDirection Direction>
VertexIndex StaticBidirectionalGraph<G>::get_source(EdgeIndex edge) const
{
    return m_graph.template get_source<Direction>(edge);
}

template<IsStaticGraph G>
template<IsDirection Direction>
VertexIndex StaticBidirectionalGraph<G>::get_target(EdgeIndex edge) const
{
    return m_graph.template get_target<Direction>(edge);
}

template<IsStaticGraph G>
template<IsDirection Direction>
const DegreeList& StaticBidirectionalGraph<G>::get_degrees() const
{
    return m_graph.template get_degrees<Direction>();
}

template<IsStaticGraph G>
template<IsDirection Direction>
Degree StaticBidirectionalGraph<G>::get_degree(VertexIndex vertex) const
{
    return m_graph.template get_degree<Direction>(vertex);
}

template<IsVertex V, IsEdge E>
std::ostream& operator<<(std::ostream& out, const StaticGraph<V, E>& graph)
{
    out << "digraph Tree {\n"
           "rankdir=TB;\n\n";

    /* Node definitions */
    for (const auto& vertex : graph.get_vertices())
    {
        out << "n" << vertex.get_index() << " [label=\"" << vertex << "\"];\n";
    }
    out << "\n";

    /* Edge definitions */
    for (const auto& edge : graph.get_edges())
    {
        out << "n" << edge.get_source() << " -> " << "n" << edge.get_target() << " [label=\"" << edge << "\"];\n";
    }
    out << "\n";

    out << "}\n";  // end graph

    return out;
}

template<IsStaticGraph G>
std::ostream& operator<<(std::ostream& out, const StaticForwardGraph<G>& graph)
{
    out << graph.get_graph();
    return out;
}

template<IsStaticGraph G>
std::ostream& operator<<(std::ostream& out, const StaticBidirectionalGraph<G>& graph)
{
    out << graph.get_graph();
    return out;
}

}

#endif
