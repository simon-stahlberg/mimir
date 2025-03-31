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

#ifndef MIMIR_GRAPHS_BGL_GRAPH_ALGORITHMS_HPP_
#define MIMIR_GRAPHS_BGL_GRAPH_ALGORITHMS_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/graphs/bgl/graph_adapters.hpp"
#include "mimir/graphs/bgl/graph_traits.hpp"
#include "mimir/graphs/bgl/property_maps.hpp"
#include "mimir/graphs/static_graph_interface.hpp"

#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/depth_first_search.hpp>
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
template<typename Graph, IsDirection Direction>
    requires IsVertexListGraph<Graph> && IsIncidenceGraph<Graph>
auto strong_components(const DirectionTaggedType<Graph, Direction>& g)
{
    using vertex_descriptor_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor;
    using vertices_size_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertices_size_type;
    using ComponentMap = typename PropertyMapTraits<std::decay_t<Graph>>::template ReadWritePropertyMap<vertex_descriptor_type, vertices_size_type>;

    auto vertex_index_map = TrivialReadPropertyMap<vertex_descriptor_type, vertex_descriptor_type>();
    auto c = ComponentMap::initialize_data(g.get().get_num_vertices());
    auto component_map = ComponentMap(c);

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
// boost::breadth_first_search
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

template<typename PredecessorMap, typename DistanceMap>
struct BFSBoostVisitor : public boost::bfs_visitor<>
{
    PredecessorMap predecessors;
    DistanceMap distances;

    BFSBoostVisitor(PredecessorMap p, DistanceMap d) : predecessors(p), distances(d) {}

    template<typename Edge, typename Graph>
    void tree_edge(Edge e, const Graph& g)
    {
        auto u = source(e, g);
        auto v = target(e, g);
        predecessors.set(v, u);
        distances.set(v, distances.get().at(u) + 1);
    }
};

template<typename Graph, IsDirection Direction, class SourceInputIter>
    requires IsVertexListGraph<Graph> && IsIncidenceGraph<Graph>
auto breadth_first_search(const DirectionTaggedType<Graph, Direction>& g, SourceInputIter s_begin, SourceInputIter s_end)
{
    using vertex_descriptor_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor;
    using PredecessorMap = typename PropertyMapTraits<std::decay_t<Graph>>::template ReadWritePropertyMap<vertex_descriptor_type, vertex_descriptor_type>;
    using DistanceMap = typename PropertyMapTraits<std::decay_t<Graph>>::template ReadWritePropertyMap<vertex_descriptor_type, DiscreteCost>;
    using ColorMap = typename PropertyMapTraits<std::decay_t<Graph>>::template ReadWritePropertyMap<vertex_descriptor_type, boost::default_color_type>;

    auto buffer = boost::queue<vertex_descriptor_type>();
    auto p = PredecessorMap::initialize_data(g.get().get_num_vertices());
    for (vertex_descriptor_type v = 0; v < g.get().get_num_vertices(); ++v)
    {
        p.at(v) = v;
    }
    auto inf = std::numeric_limits<DiscreteCost>::max();
    auto d = DistanceMap::initialize_data(g.get().get_num_vertices());
    std::fill(d.begin(), d.end(), inf);
    for (auto it = s_begin; it != s_end; ++it)
    {
        d.at(*it) = DiscreteCost(0);
    }
    auto visitor = BFSBoostVisitor(PredecessorMap(p), DistanceMap(d));

    auto color_vector = ColorMap::initialize_data(g.get().get_num_vertices());
    auto color_map = ColorMap(color_vector);
    boost::breadth_first_search(g, s_begin, s_end, buffer, visitor, color_map);

    return std::make_tuple(p, d);
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::depth_first_search
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

template<typename PredecessorMap>
struct DefaultDFSBoostVisitor : public boost::dfs_visitor<>
{
    PredecessorMap predecessors;

    DefaultDFSBoostVisitor(PredecessorMap p) : predecessors(p) {}

    template<typename Edge, typename Graph>
    void tree_edge(Edge e, const Graph& g)
    {
        auto u = source(e, g);
        auto v = target(e, g);
        predecessors.set(v, u);
    }
};

template<typename Graph, IsDirection Direction, class SourceInputIter>
    requires IsVertexListGraph<Graph> && IsIncidenceGraph<Graph>
auto depth_first_search(const DirectionTaggedType<Graph, Direction>& g, SourceInputIter s_begin, SourceInputIter s_end)
{
    using vertex_descriptor_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor;
    using ColorMap = typename PropertyMapTraits<std::decay_t<Graph>>::template ReadWritePropertyMap<vertex_descriptor_type, boost::default_color_type>;
    using PredecessorMap = typename PropertyMapTraits<std::decay_t<Graph>>::template ReadWritePropertyMap<vertex_descriptor_type, vertex_descriptor_type>;

    auto p = PredecessorMap::initialize_data(g.get().get_num_vertices());
    for (vertex_descriptor_type v = 0; v < g.get().get_num_vertices(); ++v)
    {
        p.at(v) = v;
    }
    auto visitor = DefaultDFSBoostVisitor(PredecessorMap(p));

    auto color_vector = ColorMap::initialize_data(g.get().get_num_vertices());
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
    requires IsVertexListGraph<Graph> && IsIncidenceGraph<Graph>
auto topological_sort(const DirectionTaggedType<Graph, Direction>& g)
{
    using vertex_descriptor_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor;

    using ColorMap = typename PropertyMapTraits<std::decay_t<Graph>>::template ReadWritePropertyMap<vertex_descriptor_type, boost::default_color_type>;

    auto d = std::vector<vertex_descriptor_type> {};
    auto color_vector = ColorMap::initialize_data(g.get().get_num_vertices());

    auto color_map = ColorMap(color_vector);

    boost::topological_sort(g, std::back_inserter(d), boost::color_map(color_map));

    return d;
}
}

#endif
