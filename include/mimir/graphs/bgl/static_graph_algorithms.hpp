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

#include "mimir/common/concepts.hpp"
#include "mimir/graphs/bgl/graph_adapters.hpp"
#include "mimir/graphs/bgl/graph_traits.hpp"
#include "mimir/graphs/bgl/property_maps.hpp"
#include "mimir/graphs/static_graph_interface.hpp"

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
// boost::dijkstra_shortest_path
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

template<IsStaticGraph Graph, IsDirection Direction, class SourceInputIter>
    requires IsVertexListGraph<Graph> && IsIncidenceGraph<Graph>
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

template<IsStaticGraph Graph, IsDirection Direction>
    requires IsVertexListGraph<Graph> && IsEdgeListGraph<Graph>
auto floyd_warshall_all_pairs_shortest_paths(const DirectionTaggedType<Graph, Direction>& g, const ContinuousCostList& w)
{
    using vertex_descriptor_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor;
    using edge_descriptor_type = typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::edge_descriptor;

    using BasicMatrix = typename PropertyMapTraits<std::decay_t<Graph>>::template BasicMatrix<vertex_descriptor_type, ContinuousCost>;
    using WeightPropertyMap = typename PropertyMapTraits<std::decay_t<Graph>>::template ReadPropertyMap<edge_descriptor_type, ContinuousCost>;

    auto matrix = BasicMatrix::initialize_data(g.get().get_num_vertices());

    auto d = BasicMatrix(matrix);
    auto weight_map = WeightPropertyMap(w);

    boost::floyd_warshall_all_pairs_shortest_paths(g, d, boost::weight_map(weight_map));

    return matrix;
}

}

#endif
