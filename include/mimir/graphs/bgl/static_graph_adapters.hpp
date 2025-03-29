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

#ifndef MIMIR_GRAPHS_BGL_STATIC_GRAPH_ADAPTERS_HPP_
#define MIMIR_GRAPHS_BGL_STATIC_GRAPH_ADAPTERS_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/graphs/bgl/graph_adapters.hpp"
#include "mimir/graphs/bgl/graph_traits.hpp"
#include "mimir/graphs/bgl/property_maps.hpp"
#include "mimir/graphs/static_graph_interface.hpp"

#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/property_map/property_map.hpp>
#include <concepts>
#include <limits>
#include <ranges>

namespace mimir::graphs::bgl
{

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::strong_components
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

/// @brief Wrapper function for boost's strong_components algorithm.
/// @param g the graph.
/// @return a pair of the number of strong components and a map from state to component.
template<IsStaticGraph Graph, IsDirection Direction>
    requires IsVertexListGraph<Graph> && IsIncidenceGraph<Graph>
std::pair<typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertices_size_type,
          std::vector<typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertices_size_type>>
strong_components(const DirectionTaggedType<Graph, Direction>& g)
{
    using vertex_descriptor_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor;
    using vertices_size_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertices_size_type;

    auto vertex_index_map = TrivialReadPropertyMap<vertex_descriptor_type, vertex_descriptor_type>();
    auto c = std::vector<vertices_size_type>(g.get().get_num_vertices());
    auto component_map = VectorReadWritePropertyMap<vertex_descriptor_type, vertices_size_type>(c);

    const auto num_components = boost::strong_components(g, component_map, boost::vertex_index_map(vertex_index_map));
    return std::make_pair(num_components, c);
}

/*
template<IsStaticGraph Graph, IsDirection Direction>
IndexGroupedVector<std::pair<typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertices_size_type,
                             typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor>>
get_partitioning(typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertices_size_type num_components,
                 std::vector<typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertices_size_type> component_map)
{
    using vertex_descriptor_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor;
    using vertices_size_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertices_size_type;
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
*/

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::dijkstra_shortest_path
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

template<IsStaticGraph Graph, IsDirection Direction, class SourceInputIter>
    requires IsVertexListGraph<Graph> && IsIncidenceGraph<Graph>
std::tuple<std::vector<typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor>, ContinuousCostList>
dijkstra_shortest_paths(const DirectionTaggedType<Graph, Direction>& g, const ContinuousCostList& w, SourceInputIter s_begin, SourceInputIter s_end)
{
    using vertex_descriptor_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor;
    using edge_descriptor_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::edge_descriptor;

    auto p = std::vector<vertex_descriptor_type>(g.get().get_num_vertices());
    auto predecessor_map = VectorReadWritePropertyMap<vertex_descriptor_type, vertex_descriptor_type>(p);
    auto d = ContinuousCostList(g.get().get_num_vertices());
    auto distance_map = VectorReadWritePropertyMap<vertex_descriptor_type, ContinuousCost>(d);
    auto weight_map = VectorReadPropertyMap<edge_descriptor_type, ContinuousCost>(w);
    auto vertex_index_map = TrivialReadPropertyMap<vertex_descriptor_type, vertex_descriptor_type>();
    auto compare = std::less<ContinuousCost>();
    auto combine = std::plus<ContinuousCost>();
    auto inf = std::numeric_limits<ContinuousCost>::infinity();
    auto zero = ContinuousCost();

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
struct BFSBoostVisitor : public boost::bfs_visitor<>
{
    std::reference_wrapper<std::vector<VertexDescriptor>> predecessors;
    std::reference_wrapper<DiscreteCostList> distances;

    BFSBoostVisitor(std::vector<VertexDescriptor>& p, DiscreteCostList& d) : predecessors(p), distances(d) {}

    template<typename Edge, typename Graph>
    void tree_edge(Edge e, const Graph& g) const
    {
        auto u = source(e, g);
        auto v = target(e, g);
        predecessors.get().at(v) = u;
        distances.get().at(v) = distances.get().at(u) + 1;
    }
};

template<IsStaticGraph Graph, IsDirection Direction, class SourceInputIter>
std::tuple<std::vector<typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor>, DiscreteCostList>
breadth_first_search(const DirectionTaggedType<Graph, Direction>& g, SourceInputIter s_begin, SourceInputIter s_end)
{
    using vertex_descriptor_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor;
    using ColorMap = boost::iterator_property_map<std::vector<boost::default_color_type>::iterator, boost::identity_property_map>;

    auto buffer = boost::queue<vertex_descriptor_type>();
    auto p = std::vector<vertex_descriptor_type>(g.get().get_num_vertices());
    for (vertex_descriptor_type v = 0; v < g.get().get_num_vertices(); ++v)
    {
        p.at(v) = v;
    }
    auto inf = std::numeric_limits<DiscreteCost>::max();
    auto d = DiscreteCostList(g.get().get_num_vertices(), inf);
    for (auto it = s_begin; it != s_end; ++it)
    {
        d.at(*it) = DiscreteCost(0);
    }
    auto visitor = BFSBoostVisitor(p, d);
    auto color_vector = std::vector<boost::default_color_type>(g.get().get_num_vertices(), boost::white_color);
    auto color_map = ColorMap(color_vector.begin(), boost::identity_property_map());
    boost::breadth_first_search(g, s_begin, s_end, buffer, visitor, color_map);

    return std::make_tuple(p, d);
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::depth_first_search
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

template<typename VertexDescriptor>
struct DFSBoostVisitor : public boost::dfs_visitor<>
{
    std::reference_wrapper<std::vector<VertexDescriptor>> predecessors;
    std::reference_wrapper<DiscreteCostList> distances;

    DFSBoostVisitor(std::vector<VertexDescriptor>& p, DiscreteCostList& d) : predecessors(p), distances(d) {}

    template<typename Edge, typename Graph>
    void tree_edge(Edge e, const Graph& g) const
    {
        auto u = source(e, g);
        auto v = target(e, g);
        predecessors.get().at(v) = u;
        distances.get().at(v) = distances.get().at(u) + 1;
    }
};

template<IsStaticGraph Graph, IsDirection Direction, class SourceInputIter>
std::tuple<std::vector<typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor>, DiscreteCostList>
depth_first_search(const DirectionTaggedType<Graph, Direction>& g, SourceInputIter s_begin, SourceInputIter s_end)
{
    using vertex_descriptor_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor;
    using ColorMap = boost::iterator_property_map<std::vector<boost::default_color_type>::iterator, boost::identity_property_map>;

    auto p = std::vector<vertex_descriptor_type>(g.get().get_num_vertices());
    for (vertex_descriptor_type v = 0; v < g.get().get_num_vertices(); ++v)
    {
        p.at(v) = v;
    }
    auto inf = std::numeric_limits<DiscreteCost>::max();
    auto d = DiscreteCostList(g.get().get_num_vertices(), inf);
    for (auto it = s_begin; it != s_end; ++it)
    {
        d.at(*it) = DiscreteCost(0);
    }
    auto visitor = DFSBoostVisitor(p, d);
    auto color_vector = std::vector<boost::default_color_type>(g.get().get_num_vertices(), boost::white_color);
    auto color_map = ColorMap(color_vector.begin(), boost::identity_property_map());

    for (; s_begin != s_end; ++s_begin)
    {
        if (color_vector[*s_begin] == boost::white_color)
        {
            boost::depth_first_search(g, visitor, color_map, *s_begin);
        }
    }

    return std::make_tuple(p, d);
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::floyd_warshall_all_pairs_shortest_paths
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

template<typename Graph, IsDirection Direction>
    requires IsVertexListGraph<Graph> && IsEdgeListGraph<Graph>
auto floyd_warshall_all_pairs_shortest_paths(const DirectionTaggedType<Graph, Direction>& g, const ContinuousCostList& w)
{
    using vertex_descriptor_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor;
    using edge_descriptor_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::edge_descriptor;

    using BasicMatrix = typename PropertyMapTraits<Graph>::template BasicMatrix<vertex_descriptor_type, ContinuousCost>;
    using WeightPropertyMap = typename PropertyMapTraits<Graph>::template ReadPropertyMap<edge_descriptor_type, ContinuousCost>;

    auto matrix = BasicMatrix::initialize_data(g.get().get_num_vertices());

    auto d = BasicMatrix(matrix);
    auto weight_map = WeightPropertyMap(w);

    boost::floyd_warshall_all_pairs_shortest_paths(g, d, boost::weight_map(weight_map));

    return matrix;
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::topological_sort
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

template<IsStaticGraph Graph, IsDirection Direction>
std::vector<typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor>
topological_sort(const DirectionTaggedType<Graph, Direction>& g)
{
    using vertex_descriptor_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor;
    using ColorMap = boost::iterator_property_map<std::vector<boost::default_color_type>::iterator, boost::identity_property_map>;

    auto d = std::vector<vertex_descriptor_type> {};
    auto color_vector = std::vector<boost::default_color_type>(g.get().get_num_vertices(), boost::white_color);
    auto color_map = ColorMap(color_vector.begin(), boost::identity_property_map());

    boost::topological_sort(g, std::back_inserter(d), boost::color_map(color_map));

    return d;
}
}

#endif
