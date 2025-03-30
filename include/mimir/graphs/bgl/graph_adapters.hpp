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

#ifndef MIMIR_GRAPHS_BGL_GRAPH_ADAPTERS_HPP_
#define MIMIR_GRAPHS_BGL_GRAPH_ADAPTERS_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/graphs/bgl/graph_traits.hpp"
#include "mimir/graphs/graph_interface.hpp"

#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>
#include <concepts>
#include <limits>
#include <ranges>

namespace mimir::graphs
{

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::VertexListGraph
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

/// @brief Get the vertices of the graph.
/// @param g the graph.
/// @return an iterator-range providing access to all the vertices in the graph.
template<IsVertexListGraph Graph, IsDirection Direction>
std::pair<typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_iterator,
          typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_iterator>
vertices(const DirectionTaggedType<Graph, Direction>& g)
{
    return { g.get().get_vertex_indices().begin(), g.get().get_vertex_indices().end() };
}

/// @brief Get the number of vertices in the graph.
/// @param g the graph.
/// @return the number of vertices in the graph.
template<IsVertexListGraph Graph, IsDirection Direction>
boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertices_size_type num_vertices(const DirectionTaggedType<Graph, Direction>& g)
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
template<IsIncidenceGraph Graph, IsDirection Direction>
typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor
source(const typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::edge_descriptor& e, const DirectionTaggedType<Graph, Direction>& g)
{
    return g.get().template get_source<Direction>(e);
}

/// @brief Get the target vertex of an edge.
/// @param e the edge.
/// @param g the graph.
/// @return the target vertex of the edge.
template<IsIncidenceGraph Graph, IsDirection Direction>
typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor
target(const typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::edge_descriptor& e, const DirectionTaggedType<Graph, Direction>& g)
{
    return g.get().template get_target<Direction>(e);
}

/// @brief Get the out edges of a vertex in the graph.
/// @param u the vertex.
/// @param g the graph.
/// @return an iterator-range providing access to all the ou edges in the graph.
template<IsIncidenceGraph Graph, IsDirection Direction>
std::pair<typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::out_edge_iterator,
          typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::out_edge_iterator>
out_edges(typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor const& u, const DirectionTaggedType<Graph, Direction>& g)
{
    return { g.get().template get_adjacent_edge_indices<Direction>(u).begin(), g.get().template get_adjacent_edge_indices<Direction>(u).end() };
}

/// @brief Get the out degree of a vertex in the graph.
/// @param u the vertex.
/// @param g the graph.
/// @return the number of out edges of the vertex.
template<IsIncidenceGraph Graph, IsDirection Direction>
boost::graph_traits<DirectionTaggedType<Graph, Direction>>::degree_size_type
out_degree(typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor const& u, const DirectionTaggedType<Graph, Direction>& g)
{
    return g.get().template get_degree<Direction>(u);
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::EdgeListGraph
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

template<IsEdgeListGraph Graph, IsDirection Direction>
std::pair<typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::edge_iterator,
          typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::edge_iterator>
edges(const DirectionTaggedType<Graph, Direction>& g)
{
    return { g.get().get_edge_indices().begin(), g.get().get_edge_indices().end() };
}

template<IsEdgeListGraph Graph, IsDirection Direction>
boost::graph_traits<DirectionTaggedType<Graph, Direction>>::edges_size_type num_edges(const DirectionTaggedType<Graph, Direction>& g)
{
    return g.get().get_num_edges();
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// boost::AdjacencyGraph
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

template<IsAdjacencyGraph Graph, IsDirection Direction>
std::pair<typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::adjacency_iterator,
          typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::adjacency_iterator>
adjacent_vertices(typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor const& u,
                  const DirectionTaggedType<Graph, Direction>& g)
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
template<IsBidirectionalGraph Graph, IsDirection Direction>
std::pair<typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::in_edge_iterator,
          typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::in_edge_iterator>
in_edges(typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor const& u, const DirectionTaggedType<Graph, Direction>& g)
{
    using InverseDirection = boost::graph_traits<DirectionTaggedType<Graph, Direction>>::inverse_direction;

    return { g.get().template get_adjacent_edge_indices<InverseDirection>(u).begin(), g.get().template get_adjacent_edge_indices<InverseDirection>(u).end() };
}

/// @brief Get the in degree of a vertex in the graph.
/// @param u the vertex.
/// @param g the graph.
/// @return the number of in edges of the vertex.
template<IsBidirectionalGraph Graph, IsDirection Direction>
boost::graph_traits<DirectionTaggedType<Graph, Direction>>::degree_size_type
in_degree(typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor const& u, const DirectionTaggedType<Graph, Direction>& g)
{
    using InverseDirection = boost::graph_traits<DirectionTaggedType<Graph, Direction>>::inverse_direction;

    return g.get().template get_degree<InverseDirection>(u);
}

/// @brief Get the toal degree (in + out) of a vertex in the graph.
/// @param u the vertex.
/// @param g the graph.
/// @return the number of in and out edges of the vertex.
template<IsBidirectionalGraph Graph, IsDirection Direction>
boost::graph_traits<DirectionTaggedType<Graph, Direction>>::degree_size_type
degree(typename boost::graph_traits<DirectionTaggedType<Graph, Direction>>::vertex_descriptor const& u, const DirectionTaggedType<Graph, Direction>& g)
{
    return g.get().template get_degree<ForwardTag>(u) + g.get().template get_degree<BackwardTag>(u);
}

}

#endif
