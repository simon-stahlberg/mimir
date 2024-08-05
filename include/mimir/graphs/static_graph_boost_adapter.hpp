/*
 * Copyright (C) 2024 Dominik Drexler and Till Hofmann
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

#ifndef MIMIR_GRAPHS_STATIC_GRAPH_BOOST_ADAPTER_HPP_
#define MIMIR_GRAPHS_STATIC_GRAPH_BOOST_ADAPTER_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/graphs/digraph.hpp"
#include "mimir/graphs/digraph_vertex_colored.hpp"
#include "mimir/graphs/static_graph.hpp"

#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/property_map/property_map.hpp>
#include <concepts>
#include <limits>
#include <ranges>

namespace boost
{

/// A tag for a graph that is both a vertex list graph and an incidence graph.
struct vertex_list_and_incidence_graph_and_edge_list_graph_and_adjacency_graph_and_bidirectional_graph_tag :
    public vertex_list_graph_tag,
    // public incidence_graph_tag,  // is included in bidirectional_graph_tag
    public edge_list_graph_tag,
    public adjacency_graph_tag,
    public bidirectional_graph_tag
{
};

/// Traits for a graph that are needed for the boost graph library.
template<mimir::IsStaticGraph Graph, mimir::IsTraversalDirection Direction>
struct graph_traits<mimir::TraversalDirectionTaggedType<Graph, Direction>>
{
    using VertexType = typename Graph::VertexType;
    using EdgeType = typename Graph::EdgeType;
    using EdgeList = typename Graph::EdgeList;

    // boost::GraphConcept
    using vertex_descriptor = mimir::VertexIndex;
    using edge_descriptor = mimir::EdgeIndex;
    using directed_category = directed_tag;
    using edge_parallel_category = allow_parallel_edge_tag;
    using traversal_category = vertex_list_and_incidence_graph_and_edge_list_graph_and_adjacency_graph_and_bidirectional_graph_tag;
    // boost::VertexListGraph
    using vertex_iterator = std::ranges::iterator_t<std::ranges::iota_view<vertex_descriptor, vertex_descriptor>>;
    using vertices_size_type = size_t;
    // boost::IncidenceGraph
    using out_edge_iterator = typename Graph::template AdjacentEdgeIndexConstIteratorType<Direction>;
    using degree_size_type = size_t;
    // boost::EdgeListGraph
    using edge_iterator = typename Graph::EdgeIndexConstIteratorType;
    using edges_size_type = size_t;
    // boost::AdjacencyGraph
    using adjacency_iterator = typename Graph::template AdjacentVertexIndexConstIteratorType<Direction>;
    // boost::BidirectionalGraph
    using inverse_direction = typename mimir::InverseTraversalDirection<Direction>::type;
    using in_edge_iterator = typename Graph::template AdjacentEdgeIndexConstIteratorType<inverse_direction>;
    // boost::strong_components
    constexpr static vertex_descriptor null_vertex() { return std::numeric_limits<vertex_descriptor>::max(); }
};

}

namespace mimir
{

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::VertexListGraph
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

/// @brief Get the vertices of the graph.
/// @param g the graph.
/// @return an iterator-range providing access to all the vertices in the graph.
template<IsStaticGraph Graph, IsTraversalDirection Direction>
std::pair<typename boost::graph_traits<mimir::TraversalDirectionTaggedType<Graph, Direction>>::vertex_iterator,
          typename boost::graph_traits<mimir::TraversalDirectionTaggedType<Graph, Direction>>::vertex_iterator>
vertices(const mimir::TraversalDirectionTaggedType<Graph, Direction>& g)
{
    std::ranges::iota_view<typename boost::graph_traits<mimir::TraversalDirectionTaggedType<Graph, Direction>>::vertex_descriptor,
                           typename boost::graph_traits<mimir::TraversalDirectionTaggedType<Graph, Direction>>::vertex_descriptor>
        range(0, g.get().get_num_vertices());
    // Make sure we can return dangling iterators.
    static_assert(std::ranges::borrowed_range<decltype(range)>);
    return std::make_pair(range.begin(), range.end());
}

/// @brief Get the number of vertices in the graph.
/// @param g the graph.
/// @return the number of vertices in the graph.
template<IsStaticGraph Graph, IsTraversalDirection Direction>
boost::graph_traits<mimir::TraversalDirectionTaggedType<Graph, Direction>>::vertices_size_type
num_vertices(const mimir::TraversalDirectionTaggedType<Graph, Direction>& g)
{
    return g.get().get_num_vertices();
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::IncidenceGraph
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

/// @brief Get the source vertex of an edge.
/// @param e the edge.
/// @param g the graph.
/// @return the source vertex of the edge.
template<IsStaticGraph Graph, IsTraversalDirection Direction>
typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::vertex_descriptor
source(const typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::edge_descriptor& e,
       const TraversalDirectionTaggedType<Graph, Direction>& g)
{
    return g.get().template get_source<Direction>(e);
}

/// @brief Get the target vertex of an edge.
/// @param e the edge.
/// @param g the graph.
/// @return the target vertex of the edge.
template<IsStaticGraph Graph, IsTraversalDirection Direction>
typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::vertex_descriptor
target(const typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::edge_descriptor& e,
       const TraversalDirectionTaggedType<Graph, Direction>& g)
{
    return g.get().template get_target<Direction>(e);
}

/// @brief Get the out edges of a vertex in the graph.
/// @param u the vertex.
/// @param g the graph.
/// @return an iterator-range providing access to all the ou edges in the graph.
template<IsStaticGraph Graph, IsTraversalDirection Direction>
std::pair<typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::out_edge_iterator,
          typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::out_edge_iterator>
out_edges(typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::vertex_descriptor const& u,
          const TraversalDirectionTaggedType<Graph, Direction>& g)
{
    return { g.get().template get_adjacent_edge_indices<Direction>(u).begin(), g.get().template get_adjacent_edge_indices<Direction>(u).end() };
}

/// @brief Get the out degree of a vertex in the graph.
/// @param u the vertex.
/// @param g the graph.
/// @return the number of out edges of the vertex.
template<IsStaticGraph Graph, IsTraversalDirection Direction>
boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::degree_size_type
out_degree(typename boost::graph_traits<mimir::TraversalDirectionTaggedType<Graph, Direction>>::vertex_descriptor const& u,
           const TraversalDirectionTaggedType<Graph, Direction>& g)
{
    return g.get().template get_degree<Direction>(u);
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::EdgeListGraph
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

template<IsStaticGraph Graph, IsTraversalDirection Direction>
std::pair<typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::edge_iterator,
          typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::edge_iterator>
edges(const TraversalDirectionTaggedType<Graph, Direction>& g)
{
    return { g.get().get_edge_indices().begin(), g.get().get_edge_indices().end() };
}

template<IsStaticGraph Graph, IsTraversalDirection Direction>
boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::edges_size_type num_edges(const TraversalDirectionTaggedType<Graph, Direction>& g)
{
    return g.get().get_num_edges();
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::AdjacencyGraph
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

template<IsStaticGraph Graph, IsTraversalDirection Direction>
std::pair<typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::adjacency_iterator,
          typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::adjacency_iterator>
adjacent_vertices(typename boost::graph_traits<mimir::TraversalDirectionTaggedType<Graph, Direction>>::vertex_descriptor const& u,
                  const TraversalDirectionTaggedType<Graph, Direction>& g)
{
    return { g.get().template get_adjacent_vertex_indices<Direction>(u).begin(), g.get().template get_adjacent_vertex_indices<Direction>(u).end() };
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::BidirectionalGraph
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

/// @brief Get the in edges of a vertex in the graph.
/// @param u the vertex.
/// @param g the graph.
/// @return an iterator-range providing access to all the in edges in the graph.
template<IsStaticGraph Graph, IsTraversalDirection Direction>
std::pair<typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::in_edge_iterator,
          typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::in_edge_iterator>
in_edges(typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::vertex_descriptor const& u,
         const TraversalDirectionTaggedType<Graph, Direction>& g)
{
    using InverseDirection = boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::inverse_direction;

    return { g.get().template get_adjacent_edge_indices<InverseDirection>(u).begin(), g.get().template get_adjacent_edge_indices<InverseDirection>(u).end() };
}

/// @brief Get the in degree of a vertex in the graph.
/// @param u the vertex.
/// @param g the graph.
/// @return the number of in edges of the vertex.
template<IsStaticGraph Graph, IsTraversalDirection Direction>
boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::degree_size_type
in_degree(typename boost::graph_traits<mimir::TraversalDirectionTaggedType<Graph, Direction>>::vertex_descriptor const& u,
          const TraversalDirectionTaggedType<Graph, Direction>& g)
{
    using InverseDirection = boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::inverse_direction;

    return g.get().template get_degree<InverseDirection>(u);
}

/// @brief Get the toal degree (in + out) of a vertex in the graph.
/// @param u the vertex.
/// @param g the graph.
/// @return the number of in and out edges of the vertex.
template<IsStaticGraph Graph, IsTraversalDirection Direction>
boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::degree_size_type
degree(typename boost::graph_traits<mimir::TraversalDirectionTaggedType<Graph, Direction>>::vertex_descriptor const& u,
       const TraversalDirectionTaggedType<Graph, Direction>& g)
{
    return g.get().template get_degree<ForwardTraversal>(u) + g.get().template get_degree<BackwardTraversal>(u);
}

/* Assert that the concepts are satisfied */
BOOST_CONCEPT_ASSERT((boost::GraphConcept<TraversalDirectionTaggedType<StaticDigraph, ForwardTraversal>>) );
BOOST_CONCEPT_ASSERT((boost::GraphConcept<TraversalDirectionTaggedType<StaticDigraph, BackwardTraversal>>) );
BOOST_CONCEPT_ASSERT((boost::VertexListGraphConcept<TraversalDirectionTaggedType<StaticDigraph, ForwardTraversal>>) );
BOOST_CONCEPT_ASSERT((boost::VertexListGraphConcept<TraversalDirectionTaggedType<StaticDigraph, BackwardTraversal>>) );
BOOST_CONCEPT_ASSERT((boost::IncidenceGraphConcept<TraversalDirectionTaggedType<StaticDigraph, ForwardTraversal>>) );
BOOST_CONCEPT_ASSERT((boost::IncidenceGraphConcept<TraversalDirectionTaggedType<StaticDigraph, BackwardTraversal>>) );
BOOST_CONCEPT_ASSERT((boost::EdgeListGraphConcept<TraversalDirectionTaggedType<StaticDigraph, ForwardTraversal>>) );
BOOST_CONCEPT_ASSERT((boost::EdgeListGraphConcept<TraversalDirectionTaggedType<StaticDigraph, BackwardTraversal>>) );
BOOST_CONCEPT_ASSERT((boost::AdjacencyGraphConcept<TraversalDirectionTaggedType<StaticDigraph, ForwardTraversal>>) );
BOOST_CONCEPT_ASSERT((boost::AdjacencyGraphConcept<TraversalDirectionTaggedType<StaticDigraph, BackwardTraversal>>) );
BOOST_CONCEPT_ASSERT((boost::BidirectionalGraphConcept<TraversalDirectionTaggedType<StaticDigraph, ForwardTraversal>>) );
BOOST_CONCEPT_ASSERT((boost::BidirectionalGraphConcept<TraversalDirectionTaggedType<StaticDigraph, BackwardTraversal>>) );

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// BasicMatrix
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

template<typename I, typename V>
class VectorBasicMatrix
{
public:
    VectorBasicMatrix(std::size_t num_vertices, V init_value = std::numeric_limits<V>::max()) : m_matrix(num_vertices, std::vector<V>(num_vertices, init_value))
    {
    }

    std::vector<V>& operator[](I i) { return m_matrix[i]; }

    const std::vector<V>& operator[](I i) const { return m_matrix[i]; }

    const std::vector<std::vector<V>>& get_matrix() const { return m_matrix; }

private:
    std::vector<std::vector<V>> m_matrix;
};

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// PropertyMaps
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

/// @brief A property map that returns its key as value.
///
/// boost requires a vertex_index that translates vertex->index, where the index is an index into the vertex list.
/// In our case, the vertex is described with a StateIndex and so vertex and index are the same.
/// To avoid storing a map of the size of the graph, we use TrivialReadPropertyMap that just returns the key.
template<typename Key, typename Value>
struct TrivialReadPropertyMap
{
    using value_type = Value;
    using key_type = Key;
    using reference = Value;
    using category = boost::read_write_property_map_tag;
};

/// @brief Get the index of a state.
/// @param key the state.
/// @return the index of the state, which is just the input key.
template<typename Key, typename Value>
inline Value get(const TrivialReadPropertyMap<Key, Value>&, Key key)
{
    return key;
}

template<IsUnsignedIntegral Key, typename Value>
class VectorReadPropertyMap
{
public:
    using value_type = Value;
    using key_type = Key;
    using reference = Value;
    using category = boost::read_write_property_map_tag;

    explicit VectorReadPropertyMap(const std::vector<Value>& distances) : m_distances(distances) {}

    Value get(Key key) const { return m_distances.get().at(key); }

private:
    std::reference_wrapper<const std::vector<Value>> m_distances;
};

template<typename Key, typename Value>
inline Value get(const VectorReadPropertyMap<Key, Value>& m, Key key)
{
    return m.get(key);
}

template<IsUnsignedIntegral Key, typename Value>
class VectorReadWritePropertyMap
{
public:
    using value_type = Value;
    using key_type = Key;
    using reference = Value;
    using category = boost::read_write_property_map_tag;

    explicit VectorReadWritePropertyMap(std::vector<Value>& distances) : m_distances(distances) {}

    Value get(Key key) const { return m_distances.get().at(key); }
    void set(Key key, Value value) { m_distances.get().at(key) = value; }

private:
    std::reference_wrapper<std::vector<Value>> m_distances;
};

template<typename Key, typename Value>
inline Value get(const VectorReadWritePropertyMap<Key, Value>& m, Key key)
{
    return m.get(key);
}

template<typename Key, typename Value>
inline void put(VectorReadWritePropertyMap<Key, Value>& m, Key key, Value value)
{
    m.set(key, value);
}

}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::strong_components
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

namespace mimir
{

/// @brief Wrapper function for boost's strong_components algorithm.
/// @param g the graph.
/// @return a pair of the number of strong components and a map from state to component.
template<IsStaticGraph Graph, IsTraversalDirection Direction>
std::pair<typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::vertices_size_type,
          std::vector<typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::vertices_size_type>>
strong_components(const TraversalDirectionTaggedType<Graph, Direction>& g)
{
    using vertex_descriptor_type = typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::vertex_descriptor;
    using vertices_size_type = typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::vertices_size_type;

    auto vertex_index_map = TrivialReadPropertyMap<vertex_descriptor_type, vertex_descriptor_type>();
    auto c = std::vector<vertices_size_type>(g.get().get_num_vertices());
    auto component_map = VectorReadWritePropertyMap<vertex_descriptor_type, vertices_size_type>(c);

    const auto num_components = boost::strong_components(g, component_map, boost::vertex_index_map(vertex_index_map));
    return std::make_pair(num_components, c);
}

template<IsStaticGraph Graph, IsTraversalDirection Direction>
IndexGroupedVector<std::pair<typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::vertices_size_type,
                             typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::vertex_descriptor>>
get_partitioning(typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::vertices_size_type num_components,
                 std::vector<typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::vertices_size_type> component_map)
{
    using vertex_descriptor_type = typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::vertex_descriptor;
    using vertices_size_type = typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::vertices_size_type;
    using state_component_pair_t = std::pair<vertices_size_type, vertex_descriptor_type>;

    auto partitioning = std::vector<state_component_pair_t>();
    for (vertex_descriptor_type v = 0; v < component_map.size(); ++v)
    {
        partitioning.push_back({ component_map.at(v), v });
    }
    std::sort(std::begin(partitioning), std::end(partitioning));
    return IndexGroupedVector<state_component_pair_t>::create(std::move(partitioning),
                                                              [](const auto& prev, const auto& cur) { return prev.first != cur.first; });
}

}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::dijkstra_shortest_path
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

namespace mimir
{

template<IsStaticGraph Graph, IsTraversalDirection Direction, class SourceInputIter>
std::tuple<std::vector<typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::vertex_descriptor>, DistanceList>
dijkstra_shortest_paths(const TraversalDirectionTaggedType<Graph, Direction>& g, const std::vector<EdgeCost>& w, SourceInputIter s_begin, SourceInputIter s_end)
{
    using vertex_descriptor_type = typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::vertex_descriptor;
    using edge_descriptor_type = typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::edge_descriptor;

    auto p = std::vector<vertex_descriptor_type>(g.get().get_num_vertices());
    auto predecessor_map = VectorReadWritePropertyMap<vertex_descriptor_type, vertex_descriptor_type>(p);
    auto d = DistanceList(g.get().get_num_vertices());
    auto distance_map = VectorReadWritePropertyMap<vertex_descriptor_type, Distance>(d);
    auto weight_map = VectorReadPropertyMap<edge_descriptor_type, EdgeCost>(w);
    auto vertex_index_map = TrivialReadPropertyMap<vertex_descriptor_type, vertex_descriptor_type>();
    auto compare = std::less<Distance>();
    auto combine = std::plus<Distance>();
    auto inf = DISTANCE_INFINITY;
    auto zero = Distance();

    // multiple source shortest path version.
    boost::dijkstra_shortest_paths(g,  //
                                   s_begin,
                                   s_end,
                                   predecessor_map,
                                   distance_map,
                                   weight_map,
                                   vertex_index_map,
                                   compare,
                                   combine,
                                   inf,
                                   zero,
                                   boost::default_dijkstra_visitor());

    return std::make_tuple(p, d);
};

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::breadth_first_search
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

template<typename VertexDescriptor>
struct CustomBFSVisitor : public boost::bfs_visitor<>
{
    std::reference_wrapper<std::vector<VertexDescriptor>> predecessors;
    std::reference_wrapper<DistanceList> distances;

    CustomBFSVisitor(std::vector<VertexDescriptor>& p, DistanceList& d) : predecessors(p), distances(d) {}

    template<typename Edge, typename Graph>
    void tree_edge(Edge e, const Graph& g) const
    {
        auto u = source(e, g);
        auto v = target(e, g);
        predecessors.get().at(v) = u;
        distances.get().at(v) = distances.get().at(u) + 1;
    }
};

template<IsStaticGraph Graph, IsTraversalDirection Direction, class SourceInputIter>
std::tuple<std::vector<typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::vertex_descriptor>, DistanceList>
breadth_first_search(const TraversalDirectionTaggedType<Graph, Direction>& g, SourceInputIter s_begin, SourceInputIter s_end)
{
    using vertex_descriptor_type = typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::vertex_descriptor;
    using ColorMap = boost::iterator_property_map<std::vector<boost::default_color_type>::iterator, boost::identity_property_map>;

    auto buffer = boost::queue<vertex_descriptor_type>();
    auto p = std::vector<vertex_descriptor_type>(g.get().get_num_vertices());
    for (vertex_descriptor_type v = 0; v < g.get().get_num_vertices(); ++v)
    {
        p.at(v) = v;
    }
    auto inf = DISTANCE_INFINITY;
    auto d = DistanceList(g.get().get_num_vertices(), inf);
    for (auto it = s_begin; it != s_end; ++it)
    {
        d.at(*it) = 0;
    }
    auto visitor = CustomBFSVisitor(p, d);
    auto color_vector = std::vector<boost::default_color_type>(g.get().get_num_vertices(), boost::white_color);
    auto color_map = ColorMap(color_vector.begin(), boost::identity_property_map());
    boost::breadth_first_search(g, s_begin, s_end, buffer, visitor, color_map);

    return std::make_tuple(p, d);
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::floyd_warshall_all_pairs_shortest_paths
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

template<IsStaticGraph Graph, IsTraversalDirection Direction>
VectorBasicMatrix<typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::vertex_descriptor, Distance>
floyd_warshall_all_pairs_shortest_paths(const TraversalDirectionTaggedType<Graph, Direction>& g, const std::vector<EdgeCost>& w)
{
    using vertex_descriptor_type = typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::vertex_descriptor;
    using edge_descriptor_type = typename boost::graph_traits<TraversalDirectionTaggedType<Graph, Direction>>::edge_descriptor;

    auto d = VectorBasicMatrix<vertex_descriptor_type, Distance>(g.get().get_num_vertices());
    auto weight_map = VectorReadPropertyMap<edge_descriptor_type, EdgeCost>(w);

    boost::floyd_warshall_all_pairs_shortest_paths(g, d, boost::weight_map(weight_map));

    return d;
}
}

#endif
