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

#ifndef MIMIR_GRAPHS_BGL_STATIC_GRAPH_ALGORITHMS_HPP_
#define MIMIR_GRAPHS_BGL_STATIC_GRAPH_ALGORITHMS_HPP_

#include "mimir/common/declarations.hpp"
#include "mimir/graphs/bgl/graph_adapters.hpp"
#include "mimir/graphs/bgl/graph_traits.hpp"
#include "mimir/graphs/bgl/property_maps.hpp"
#include "mimir/graphs/graph_properties.hpp"

#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/graph_traits.hpp>
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
template<typename Graph, IsDirection Direction>
    requires IsStaticGraph<Graph> && IsVertexListGraph<Graph> && IsIncidenceGraph<Graph>
auto strong_components(const DirectionTaggedType<Graph, Direction>& g)
{
    using vertex_descriptor_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor;
    using vertices_size_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertices_size_type;
    using ComponentMap = VectorReadWritePropertyMap<vertex_descriptor_type, vertices_size_type>;

    auto vertex_index_map = TrivialReadPropertyMap<vertex_descriptor_type, vertex_descriptor_type>();
    auto c = std::vector<vertices_size_type>(g.get().get_num_vertices());
    auto component_map = ComponentMap(c);

    const auto num_components = boost::strong_components(g, component_map, boost::vertex_index_map(vertex_index_map));
    return std::make_pair(num_components, c);
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::breadth_first_search
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

template<typename Graph, IsDirection Direction, class SourceInputIter>
    requires IsStaticGraph<Graph> && IsVertexListGraph<Graph> && IsIncidenceGraph<Graph>
auto breadth_first_search(const DirectionTaggedType<Graph, Direction>& g, SourceInputIter s_begin, SourceInputIter s_end)
{
    using vertex_descriptor_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor;
    using edge_descriptor_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::edge_descriptor;
    using PredecessorMap = VectorReadWritePropertyMap<vertex_descriptor_type, vertex_descriptor_type>;
    using DistanceMap = VectorReadWritePropertyMap<vertex_descriptor_type, DiscreteCost>;
    using ColorMap = VectorReadWritePropertyMap<vertex_descriptor_type, boost::default_color_type>;

    auto buffer = boost::queue<vertex_descriptor_type>();
    auto p = std::vector<vertex_descriptor_type>(g.get().get_num_vertices());
    for (vertex_descriptor_type v = 0; v < g.get().get_num_vertices(); ++v)
    {
        p.at(v) = v;
    }
    auto inf = std::numeric_limits<DiscreteCost>::max();
    auto d = std::vector<DiscreteCost>(g.get().get_num_vertices());
    std::fill(d.begin(), d.end(), inf);
    for (auto it = s_begin; it != s_end; ++it)
    {
        d.at(*it) = DiscreteCost(0);
    }

    struct BFSBoostVisitor : public boost::bfs_visitor<>
    {
        PredecessorMap predecessors;
        DistanceMap distances;

        BFSBoostVisitor(PredecessorMap predecessors, DistanceMap distances) : predecessors(predecessors), distances(distances) {}

        void tree_edge(edge_descriptor_type edge, const DirectionTaggedType<Graph, Direction>& graph)
        {
            auto u = mimir::graphs::source(edge, graph);
            auto v = mimir::graphs::target(edge, graph);
            predecessors.set(v, u);
            distances.set(v, distances.get().at(u) + 1);
        }
    };
    auto visitor = BFSBoostVisitor(PredecessorMap(p), DistanceMap(d));

    auto c = std::vector<boost::default_color_type>(g.get().get_num_vertices());
    auto color_map = ColorMap(c);
    boost::breadth_first_search(g, s_begin, s_end, buffer, visitor, color_map);

    return std::make_tuple(p, d);
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::depth_first_search
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

template<typename Graph, IsDirection Direction, class SourceInputIter>
    requires IsStaticGraph<Graph> && IsVertexListGraph<Graph> && IsIncidenceGraph<Graph>
auto depth_first_search(const DirectionTaggedType<Graph, Direction>& g, SourceInputIter s_begin, SourceInputIter s_end)
{
    using vertex_descriptor_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor;
    using edge_descriptor_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::edge_descriptor;
    using ColorMap = VectorReadWritePropertyMap<vertex_descriptor_type, boost::default_color_type>;
    using PredecessorMap = VectorReadWritePropertyMap<vertex_descriptor_type, vertex_descriptor_type>;

    auto p = std::vector<vertex_descriptor_type>(g.get().get_num_vertices());
    for (vertex_descriptor_type v = 0; v < g.get().get_num_vertices(); ++v)
    {
        p.at(v) = v;
    }

    struct DefaultDFSBoostVisitor : public boost::dfs_visitor<>
    {
        PredecessorMap predecessors;

        DefaultDFSBoostVisitor(PredecessorMap predecessors) : predecessors(predecessors) {}

        void tree_edge(edge_descriptor_type edge, const DirectionTaggedType<Graph, Direction>& graph)
        {
            auto u = mimir::graphs::source(edge, graph);
            auto v = mimir::graphs::target(edge, graph);
            predecessors.set(v, u);
        }
    };
    auto visitor = DefaultDFSBoostVisitor(PredecessorMap(p));

    auto color_vector = std::vector<boost::default_color_type>(g.get().get_num_vertices());
    auto color_map = ColorMap(color_vector);

    for (; s_begin != s_end; ++s_begin)
    {
        if (color_vector[*s_begin] == boost::white_color)
        {
            boost::depth_first_search(g, visitor, color_map, *s_begin);
        }
    }

    return p;
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::topological_sort
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

template<typename Graph, IsDirection Direction>
    requires IsStaticGraph<Graph> && IsVertexListGraph<Graph> && IsIncidenceGraph<Graph>
auto topological_sort(const DirectionTaggedType<Graph, Direction>& g)
{
    using vertex_descriptor_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor;

    using ColorMap = VectorReadWritePropertyMap<vertex_descriptor_type, boost::default_color_type>;

    auto d = std::vector<vertex_descriptor_type> {};
    auto c = std::vector<boost::default_color_type>(g.get().get_num_vertices());
    auto color_map = ColorMap(c);

    boost::topological_sort(g, std::back_inserter(d), boost::color_map(color_map));

    return d;
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::dijkstra_shortest_path
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

template<typename Graph, IsDirection Direction, class SourceInputIter>
    requires IsStaticGraph<Graph> && IsVertexListGraph<Graph> && IsIncidenceGraph<Graph>
auto dijkstra_shortest_paths(const DirectionTaggedType<Graph, Direction>& g, const ContinuousCostList& w, SourceInputIter s_begin, SourceInputIter s_end)
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
// boost::floyd_warshall_all_pairs_shortest_paths
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

template<typename Graph, IsDirection Direction>
    requires IsStaticGraph<Graph> && IsVertexListGraph<Graph> && IsEdgeListGraph<Graph>
auto floyd_warshall_all_pairs_shortest_paths(const DirectionTaggedType<Graph, Direction>& g, const ContinuousCostList& w)
{
    using vertex_descriptor_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor;
    using edge_descriptor_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::edge_descriptor;

    using BasicMatrix = VectorBasicMatrix<vertex_descriptor_type, ContinuousCost>;
    using WeightPropertyMap = VectorReadPropertyMap<edge_descriptor_type, ContinuousCost>;

    auto m = std::vector<std::vector<ContinuousCost>>(g.get().get_num_vertices(), std::vector<ContinuousCost>(g.get().get_num_vertices()));
    auto distance_matrix = BasicMatrix(m);
    auto weight_map = WeightPropertyMap(w);

    boost::floyd_warshall_all_pairs_shortest_paths(g, distance_matrix, boost::weight_map(weight_map));

    return m;
}

}

#endif
