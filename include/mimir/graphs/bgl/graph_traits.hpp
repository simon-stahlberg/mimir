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

#ifndef MIMIR_GRAPHS_BGL_GRAPH_TRAITS_HPP_
#define MIMIR_GRAPHS_BGL_GRAPH_TRAITS_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/graphs/graph_interface.hpp"
#include "mimir/graphs/graph_traversal_interface.hpp"

#include <boost/graph/breadth_first_search.hpp>
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

namespace boost
{

/// Traits for a graph that are needed for the boost graph library.
template<typename Graph, mimir::graphs::IsDirection Direction>
    requires mimir::graphs::IsVertexListGraph<Graph>    //
             && mimir::graphs::IsIncidenceGraph<Graph>  //
             && mimir::graphs::IsEdgeListGraph<Graph>   //
             && mimir::graphs::IsAdjacencyGraph<Graph>  //
             && mimir::graphs::IsBidirectionalGraph<Graph>
struct graph_traits<mimir::graphs::DirectionTaggedType<Graph, Direction>>
{
    struct vertex_list_and_incidence_and_edge_list_and_adjacency_and_bidirectional_graph_tag :
        public vertex_list_graph_tag,
        // public incidence_graph_tag,  // is included in bidirectional_graph_tag
        public edge_list_graph_tag,
        public adjacency_graph_tag,
        public bidirectional_graph_tag
    {
    };

    // boost::GraphConcept
    using vertex_descriptor = mimir::graphs::VertexIndex;
    using edge_descriptor = mimir::graphs::EdgeIndex;
    using directed_category = directed_tag;
    using edge_parallel_category = allow_parallel_edge_tag;
    using traversal_category = vertex_list_and_incidence_and_edge_list_and_adjacency_and_bidirectional_graph_tag;
    // boost::VertexListGraph
    using vertex_iterator = typename Graph::VertexIndexConstIterator;
    using vertices_size_type = size_t;
    // boost::IncidenceGraph
    using out_edge_iterator = typename Graph::template AdjacentEdgeIndexConstIterator<Direction>;
    using degree_size_type = size_t;
    // boost::EdgeListGraph
    using edge_iterator = typename Graph::EdgeIndexConstIterator;
    using edges_size_type = size_t;
    // boost::AdjacencyGraph
    using adjacency_iterator = typename Graph::template AdjacentVertexIndexConstIterator<Direction>;
    // boost::BidirectionalGraph
    using inverse_direction = typename Direction::Inverse;
    using in_edge_iterator = typename Graph::template AdjacentEdgeIndexConstIterator<inverse_direction>;
    // boost::strong_components
    constexpr static vertex_descriptor null_vertex() { return std::numeric_limits<vertex_descriptor>::max(); }
};

}

#endif
