/*
 * Copyright (C) 2024 Till Hofmann
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

#ifndef MIMIR_GRAPHS_BOOST_ADAPTER_HPP_
#define MIMIR_GRAPHS_BOOST_ADAPTER_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/graphs/digraph.hpp"
#include "mimir/graphs/digraph_vertex_colored.hpp"
#include "mimir/graphs/graph.hpp"

#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/property_map/property_map.hpp>
#include <concepts>
#include <limits>
#include <map>
#include <ranges>

namespace mimir
{

struct ForwardTraversal
{
};
struct BackwardTraversal
{
};

template<typename T>
concept IsTraversalDirection = std::same_as<T, ForwardTraversal> || std::same_as<T, BackwardTraversal>;

template<IsGraph Graph, IsTraversalDirection Direction>
class GraphWithDirection
{
public:
    using VertexType = typename Graph::VertexType;
    using EdgeType = typename Graph::EdgeType;
    using VertexList = std::vector<VertexType>;
    using EdgeList = std::vector<EdgeType>;

private:
    std::reference_wrapper<const Graph> m_graph;

public:
    explicit GraphWithDirection(const Graph& graph, Direction) : m_graph(graph) {}

    const Graph& get_graph() const { return m_graph.get(); }
};
}

namespace boost
{

/// A tag for a graph that is both a vertex list graph and an incidence graph.
struct vertex_list_and_incidence_graph_tag : public vertex_list_graph_tag, public incidence_graph_tag
{
};

/// Traits for a graph that are needed for the boost graph library.
template<mimir::IsGraph Graph>
struct graph_traits<Graph>
{
    using VertexType = typename Graph::VertexType;
    using EdgeType = typename Graph::EdgeType;

    // boost::GraphConcept
    using vertex_descriptor = mimir::VertexIndex;
    using edge_descriptor = mimir::EdgeIndex;
    using directed_category = directed_tag;
    using edge_parallel_category = allow_parallel_edge_tag;
    using traversal_category = vertex_list_and_incidence_graph_tag;
    using edges_size_type = size_t;
    // boost::VertexListGraph
    using vertex_iterator = std::ranges::iterator_t<std::ranges::iota_view<vertex_descriptor, vertex_descriptor>>;
    using vertices_size_type = size_t;
    // boost::IncidenceGraph
    using out_edge_iterator = mimir::EdgeIndexIterator<EdgeType>::const_iterator;
    using degree_size_type = size_t;
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
template<IsGraph Graph>
std::pair<typename boost::graph_traits<Graph>::vertex_iterator, typename boost::graph_traits<Graph>::vertex_iterator> vertices(const Graph& g)
{
    std::ranges::iota_view<typename boost::graph_traits<Graph>::vertex_descriptor, typename boost::graph_traits<Graph>::vertex_descriptor> range(
        0,
        g.get_num_vertices());
    // Make sure we can return dangling iterators.
    static_assert(std::ranges::borrowed_range<decltype(range)>);
    return std::make_pair(range.begin(), range.end());
}

/// @brief Get the number of vertices in the graph.
/// @param g the graph.
/// @return the number of vertices in the graph.
template<IsGraph Graph>
boost::graph_traits<Graph>::vertices_size_type num_vertices(const Graph& g)
{
    return g.get_num_vertices();
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::IncidenceGraph
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

/// @brief Get the source vertex of an edge.
/// @param e the edge.
/// @param g the graph.
/// @return the source vertex of the edge.
template<IsGraph Graph>
typename boost::graph_traits<Graph>::vertex_descriptor source(const typename boost::graph_traits<Graph>::edge_descriptor& e, const Graph& g)
{
    return g.get_edges()[e].get_source();
}

/// @brief Get the target vertex of an edge.
/// @param e the edge.
/// @param g the graph.
/// @return the target vertex of the edge.
template<IsGraph Graph>
typename boost::graph_traits<Graph>::vertex_descriptor target(const typename boost::graph_traits<Graph>::edge_descriptor& e, const Graph& g)
{
    return g.get_edges()[e].get_target();
}

/// @brief Get the edges of the graph.
/// @param g the graph.
/// @return an iterator-range providing access to all the forward edges (i.e., the out edges) in the graph.
template<IsGraph Graph>
std::pair<typename boost::graph_traits<Graph>::out_edge_iterator, typename boost::graph_traits<Graph>::out_edge_iterator>
out_edges(typename boost::graph_traits<Graph>::vertex_descriptor const& u, const Graph& g)
{
    // std::cout << "Out edges: " << u << std::endl;
    // for (const auto e : g.get_adjacent_edge_indices(u, true))
    //{
    //     std::cout << e << std::endl;
    // }
    return { g.get_adjacent_edge_indices(u, true).begin(), g.get_adjacent_edge_indices(u, true).end() };
}

/// @brief Get the number of out edges of a vertex.
/// @param u the vertex.
/// @param g the graph.
/// @return the number of out edges of the vertex.
template<IsGraph Graph>
boost::graph_traits<Graph>::degree_size_type out_degree(typename boost::graph_traits<Graph>::vertex_descriptor const& u, const Graph& g)
{
    // TODO: this assumes that the graph was translated for forward or bidirectional accordingly.
    // We must add some assertion here.
    return std::distance(g.get_adjacent_edge_indices(u, true).begin(), g.get_adjacent_edge_indices(u, true).end());
}

}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::strong_components
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

// boost::strong_components requires a vertex_index that translates vertex->index, where the index is an index into the vertex list.
// In our case, the vertex is described with a StateIndex and so vertex and index are the same.
// To avoid storing a map of the size of the graph, we provide a custom index that just returns the key.
namespace mimir
{
/// @brief A property map that maps a vertex to its index.
struct VertexIndexMap
{
};
}

namespace boost
{

/// @brief Traits for the VertexIndexMap property map, required for boost::strong_components.
template<>
struct property_traits<mimir::VertexIndexMap>
{
    using value_type = mimir::VertexIndex;
    using key_type = mimir::VertexIndex;
    using reference = mimir::VertexIndex;
    using category = boost::readable_property_map_tag;
};

}

namespace mimir
{

/// @brief Get the index of a state.
/// @param key the state.
/// @return the index of the state, which is just the input key.
inline boost::property_traits<VertexIndexMap>::reference get(const VertexIndexMap&, boost::property_traits<VertexIndexMap>::key_type key) { return key; }

/// @brief Wrapper function for boost's strong_components algorithm.
/// @param g the transition system.
/// @return a pair of the number of strong components and a map from state to component.
template<IsGraph Graph>
std::pair<typename boost::graph_traits<Graph>::vertices_size_type,
          std::map<typename boost::graph_traits<Graph>::vertex_descriptor, typename boost::graph_traits<Graph>::vertices_size_type>>
strong_components(const Graph& g)
{
    std::map<StateIndex, size_t> component_map;
    boost::associative_property_map component_map_property(component_map);
    const auto num_components = boost::strong_components(g, component_map_property, boost::vertex_index_map(VertexIndexMap()));
    return std::make_pair(num_components, component_map);
}

template<IsGraph Graph>
IndexGroupedVector<std::pair<typename boost::graph_traits<Graph>::vertices_size_type, typename boost::graph_traits<Graph>::vertex_descriptor>>
get_partitioning(typename boost::graph_traits<Graph>::vertices_size_type num_components,
                 std::map<typename boost::graph_traits<Graph>::vertex_descriptor, typename boost::graph_traits<Graph>::vertices_size_type> component_map)
{
    using state_component_pair_t = std::pair<typename boost::graph_traits<Graph>::vertices_size_type, typename boost::graph_traits<Graph>::vertex_descriptor>;
    auto partitioning = std::vector<state_component_pair_t>();
    for (const auto& [state, component] : component_map)
    {
        partitioning.push_back({ component, state });
    }
    std::sort(std::begin(partitioning), std::end(partitioning));
    return IndexGroupedVector<state_component_pair_t>::create(std::move(partitioning),
                                                              [](const auto& prev, const auto& cur) { return prev.first != cur.first; });
}

/* Assert that the concepts are satisfied */
BOOST_CONCEPT_ASSERT((boost::GraphConcept<Digraph>) );
BOOST_CONCEPT_ASSERT((boost::VertexListGraphConcept<Digraph>) );
BOOST_CONCEPT_ASSERT((boost::IncidenceGraphConcept<Digraph>) );

BOOST_CONCEPT_ASSERT((boost::GraphConcept<VertexColoredDigraph>) );
BOOST_CONCEPT_ASSERT((boost::VertexListGraphConcept<VertexColoredDigraph>) );
BOOST_CONCEPT_ASSERT((boost::IncidenceGraphConcept<VertexColoredDigraph>) );

}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::dijkstra_shortest_path
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

namespace mimir
{
class WeightMap
{
private:
    std::reference_wrapper<const std::vector<EdgeCost>> m_edge_costs;

public:
    explicit WeightMap(const std::vector<EdgeCost>& edge_costs) : m_edge_costs(edge_costs) {}

    EdgeCost get(EdgeIndex key) const
    {
        // std::cout << "WeightMap::get: key=" << key << " " << m_edge_costs.get().at(key) << std::endl;
        return m_edge_costs.get().at(key);
    }
};

class PredecessorMap
{
private:
    std::reference_wrapper<std::vector<VertexIndex>> m_predecessors;

public:
    explicit PredecessorMap(std::vector<VertexIndex>& predecessors) : m_predecessors(predecessors) {}

    VertexIndex get(VertexIndex key) const { return m_predecessors.get().at(key); }
    void set(VertexIndex key, VertexIndex value)
    {
        // std::cout << "PredecessorMap::set: key=" << key << " value=" << value << std::endl;
        m_predecessors.get().at(key) = value;
    }
};

class DistanceMap
{
private:
    std::reference_wrapper<std::vector<EdgeCost>> m_distances;

public:
    explicit DistanceMap(std::vector<EdgeCost>& distances) : m_distances(distances) {}

    EdgeCost get(VertexIndex key) const
    {
        // std::cout << "DistanceMap::get: key=" << key << " value=" << m_distances.get().at(key) << std::endl;
        return m_distances.get().at(key);
    }
    void set(VertexIndex key, EdgeCost value)
    {
        // std::cout << "DistanceMap::set: key=" << key << " value=" << value << std::endl;
        m_distances.get().at(key) = value;
    }
};
}

namespace boost
{
/// @brief Traits for the WeightMap property map, required for boost::dijkstra_shortest_path.
template<>
struct property_traits<mimir::WeightMap>
{
    using value_type = mimir::EdgeCost;
    using key_type = mimir::EdgeIndex;
    using reference = mimir::EdgeCost;
    using category = boost::readable_property_map_tag;
};

/// @brief Traits for the PredecessorMap property map, required for boost::dijkstra_shortest_path.
template<>
struct property_traits<mimir::PredecessorMap>
{
    using value_type = mimir::VertexIndex;
    using key_type = mimir::VertexIndex;
    using reference = mimir::VertexIndex;
    using category = boost::read_write_property_map_tag;
};

/// @brief Traits for the DistanceMap property map, required for boost::dijkstra_shortest_path.
template<>
struct property_traits<mimir::DistanceMap>
{
    using value_type = mimir::EdgeCost;
    using key_type = mimir::VertexIndex;
    using reference = mimir::EdgeCost;
    using category = boost::read_write_property_map_tag;
};

}

namespace mimir
{
/// @brief Get the weight of the edge.
/// @param WeightMap the weight map.
/// @param key the edge.
/// @return the weight of the edge.
inline boost::property_traits<WeightMap>::reference get(const WeightMap& m, boost::property_traits<WeightMap>::key_type key) { return m.get(key); }

inline boost::property_traits<PredecessorMap>::reference get(const PredecessorMap& m, boost::property_traits<PredecessorMap>::key_type key)
{
    return m.get(key);
}

inline void put(PredecessorMap& m, boost::property_traits<PredecessorMap>::key_type key, boost::property_traits<PredecessorMap>::value_type value)
{
    m.set(key, value);
}

inline boost::property_traits<DistanceMap>::reference get(const DistanceMap& m, boost::property_traits<DistanceMap>::key_type key) { return m.get(key); }

inline void put(DistanceMap& m, boost::property_traits<DistanceMap>::key_type key, boost::property_traits<DistanceMap>::value_type value) { m.set(key, value); }

template<IsGraph Graph, class SourceInputIter>
std::tuple<std::vector<typename boost::graph_traits<Graph>::vertex_descriptor>, std::vector<double>>
dijkstra_shortest_paths(const Graph& g, const std::vector<EdgeCost>& w, SourceInputIter s_begin, SourceInputIter s_end)
{
    auto p = std::vector<typename boost::graph_traits<Graph>::vertex_descriptor>(g.get_num_vertices());
    auto d = std::vector<double>(g.get_num_vertices());
    auto compare = std::less<EdgeCost>();
    auto combine = std::plus<EdgeCost>();
    auto inf = std::numeric_limits<EdgeCost>::max();
    auto zero = EdgeCost();

    // Custom visitor to add debug information
    class DebugVisitor : public boost::default_dijkstra_visitor
    {
    private:
        DistanceMap d;
        WeightMap w;

    public:
        DebugVisitor(DistanceMap d, WeightMap w) : d(d), w(w) {}

        void edge_relaxed(EdgeIndex e, const Graph& g) { std::cout << "Edge relaxed: (" << source(e, g) << ", " << target(e, g) << ")" << std::endl; }

        void edge_not_relaxed(EdgeIndex e, const Graph& g)
        {
            std::cout << "Edge not relaxed: (" << source(e, g) << ", " << target(e, g) << ") " << d.get(source(e, g)) << " " << get(w, e) << " "
                      << d.get(target(e, g)) << std::endl;
        }
    };

    // multiple source shortest path
    boost::dijkstra_shortest_paths(g,  //
                                   s_begin,
                                   s_end,
                                   PredecessorMap(p),
                                   DistanceMap(d),
                                   WeightMap(w),
                                   VertexIndexMap(),
                                   compare,
                                   combine,
                                   inf,
                                   zero,
                                   DebugVisitor(DistanceMap(d), WeightMap(w)));

    std::cout << p << std::endl;
    std::cout << d << std::endl;

    return std::make_tuple(p, d);
};
}

#endif
