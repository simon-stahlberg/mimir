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

#ifndef MIMIR_GRAPHS_DYNAMIC_GRAPH_IMPL_HPP_
#define MIMIR_GRAPHS_DYNAMIC_GRAPH_IMPL_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/graphs/dynamic_graph_interface.hpp"
#include "mimir/graphs/graph_edge_interface.hpp"
#include "mimir/graphs/graph_traversal_interface.hpp"
#include "mimir/graphs/graph_vertex_interface.hpp"
#include "mimir/graphs/types.hpp"

#include <boost/hana.hpp>
#include <cassert>
#include <iostream>
#include <ranges>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mimir::graphs
{

/* DynamicGraph */

/* VertexIndexConstIterator */

template<IsVertex V, IsEdge E>
void DynamicGraph<V, E>::VertexIndexConstIterator::advance()
{
    ++m_pos;
}

template<IsVertex V, IsEdge E>
DynamicGraph<V, E>::VertexIndexConstIterator::VertexIndexConstIterator() : m_pos(-1), m_vertices(nullptr)
{
}

template<IsVertex V, IsEdge E>
DynamicGraph<V, E>::VertexIndexConstIterator::VertexIndexConstIterator(const std::unordered_map<VertexIndex, V>& vertices, bool begin) :
    m_pos(begin ? 0 : vertices.size()),
    m_vertices(&vertices)
{
}

template<IsVertex V, IsEdge E>
DynamicGraph<V, E>::VertexIndexConstIterator::value_type DynamicGraph<V, E>::VertexIndexConstIterator::operator*() const
{
    assert(m_vertices);
    assert(m_pos < m_vertices->size());
    return m_vertices->at(m_pos).get_index();
}

template<IsVertex V, IsEdge E>
DynamicGraph<V, E>::VertexIndexConstIterator& DynamicGraph<V, E>::VertexIndexConstIterator::operator++()
{
    advance();
    return *this;
}

template<IsVertex V, IsEdge E>
DynamicGraph<V, E>::VertexIndexConstIterator DynamicGraph<V, E>::VertexIndexConstIterator::operator++(int)
{
    VertexIndexConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsVertex V, IsEdge E>
bool DynamicGraph<V, E>::VertexIndexConstIterator::operator==(const VertexIndexConstIterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsVertex V, IsEdge E>
bool DynamicGraph<V, E>::VertexIndexConstIterator::operator!=(const VertexIndexConstIterator& other) const
{
    return !(*this == other);
}

/* EdgeIndexConstIterator */

template<IsVertex V, IsEdge E>
void DynamicGraph<V, E>::EdgeIndexConstIterator::advance()
{
    ++m_pos;
}

template<IsVertex V, IsEdge E>
DynamicGraph<V, E>::EdgeIndexConstIterator::EdgeIndexConstIterator() : m_pos(-1), m_edges(nullptr)
{
}

template<IsVertex V, IsEdge E>
DynamicGraph<V, E>::EdgeIndexConstIterator::EdgeIndexConstIterator(const std::unordered_map<EdgeIndex, E>& edges, bool begin) :
    m_pos(begin ? 0 : edges.size()),
    m_edges(&edges)
{
}

template<IsVertex V, IsEdge E>
DynamicGraph<V, E>::EdgeIndexConstIterator::value_type DynamicGraph<V, E>::EdgeIndexConstIterator::operator*() const
{
    assert(m_edges);
    assert(m_pos < m_edges->size());
    return m_edges->at(m_pos).get_index();
}

template<IsVertex V, IsEdge E>
DynamicGraph<V, E>::EdgeIndexConstIterator& DynamicGraph<V, E>::EdgeIndexConstIterator::operator++()
{
    advance();
    return *this;
}

template<IsVertex V, IsEdge E>
DynamicGraph<V, E>::EdgeIndexConstIterator DynamicGraph<V, E>::EdgeIndexConstIterator::operator++(int)
{
    EdgeIndexConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsVertex V, IsEdge E>
bool DynamicGraph<V, E>::EdgeIndexConstIterator::operator==(const EdgeIndexConstIterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsVertex V, IsEdge E>
bool DynamicGraph<V, E>::EdgeIndexConstIterator::operator!=(const EdgeIndexConstIterator& other) const
{
    return !(*this == other);
}

/* AdjacentVertexConstIterator */

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
void DynamicGraph<V, E>::AdjacentVertexConstIterator<Direction>::advance()
{
    ++m_slice_iter;
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
DynamicGraph<V, E>::AdjacentVertexConstIterator<Direction>::AdjacentVertexConstIterator() : m_vertices(nullptr), m_edges(nullptr), m_slice_iter()
{
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
DynamicGraph<V, E>::AdjacentVertexConstIterator<Direction>::AdjacentVertexConstIterator(const std::unordered_map<VertexIndex, V>& vertices,
                                                                                        const std::unordered_map<EdgeIndex, E>& edges,
                                                                                        const std::unordered_set<EdgeIndex>& slice,
                                                                                        bool begin) :
    m_vertices(&vertices),
    m_edges(&edges),
    m_slice_iter(begin ? slice.begin() : slice.end())
{
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
DynamicGraph<V, E>::AdjacentVertexConstIterator<Direction>::reference DynamicGraph<V, E>::AdjacentVertexConstIterator<Direction>::operator*() const
{
    assert(m_vertices);
    assert(m_edges);

    if constexpr (std::is_same_v<Direction, ForwardTag>)
    {
        return m_vertices->at(m_edges->at(*m_slice_iter).get_target());
    }
    else if constexpr (std::is_same_v<Direction, BackwardTag>)
    {
        return m_vertices->at(m_edges->at(*m_slice_iter).get_source());
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "AdjacentVertexConstIterator<Edge, Direction>::operator*(): Missing implementation for IsDirection.");
    }
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
DynamicGraph<V, E>::AdjacentVertexConstIterator<Direction>& DynamicGraph<V, E>::AdjacentVertexConstIterator<Direction>::operator++()
{
    advance();
    return *this;
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
DynamicGraph<V, E>::AdjacentVertexConstIterator<Direction> DynamicGraph<V, E>::AdjacentVertexConstIterator<Direction>::operator++(int)
{
    AdjacentVertexConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
bool DynamicGraph<V, E>::AdjacentVertexConstIterator<Direction>::operator==(const AdjacentVertexConstIterator& other) const
{
    return (m_slice_iter == other.m_slice_iter);
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
bool DynamicGraph<V, E>::AdjacentVertexConstIterator<Direction>::operator!=(const AdjacentVertexConstIterator& other) const
{
    return !(*this == other);
}

/* AdjacentVertexIndexConstIterator */

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
void DynamicGraph<V, E>::AdjacentVertexIndexConstIterator<Direction>::advance()
{
    ++m_slice_iter;
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
DynamicGraph<V, E>::AdjacentVertexIndexConstIterator<Direction>::AdjacentVertexIndexConstIterator() : m_edges(nullptr), m_slice_iter()
{
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
DynamicGraph<V, E>::AdjacentVertexIndexConstIterator<Direction>::AdjacentVertexIndexConstIterator(const std::unordered_map<EdgeIndex, E>& edges,
                                                                                                  const std::unordered_set<EdgeIndex>& slice,
                                                                                                  bool begin) :
    m_edges(&edges),
    m_slice_iter(begin ? slice.begin() : slice.end())
{
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
DynamicGraph<V, E>::AdjacentVertexIndexConstIterator<Direction>::value_type DynamicGraph<V, E>::AdjacentVertexIndexConstIterator<Direction>::operator*() const
{
    assert(m_edges);

    if constexpr (std::is_same_v<Direction, ForwardTag>)
    {
        return m_edges->at(*m_slice_iter).get_target();
    }
    else if constexpr (std::is_same_v<Direction, BackwardTag>)
    {
        return m_edges->at(*m_slice_iter).get_source();
    }
    else
    {
        static_assert(dependent_false<Direction>::value,
                      "AdjacentVertexIndexConstIterator<Edge, Direction>::operator*(): Missing implementation for IsDirection.");
    }
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
DynamicGraph<V, E>::AdjacentVertexIndexConstIterator<Direction>& DynamicGraph<V, E>::AdjacentVertexIndexConstIterator<Direction>::operator++()
{
    advance();
    return *this;
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
DynamicGraph<V, E>::AdjacentVertexIndexConstIterator<Direction> DynamicGraph<V, E>::AdjacentVertexIndexConstIterator<Direction>::operator++(int)
{
    AdjacentVertexIndexConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
bool DynamicGraph<V, E>::AdjacentVertexIndexConstIterator<Direction>::operator==(const AdjacentVertexIndexConstIterator& other) const
{
    return (m_slice_iter == other.m_slice_iter);
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
bool DynamicGraph<V, E>::AdjacentVertexIndexConstIterator<Direction>::operator!=(const AdjacentVertexIndexConstIterator& other) const
{
    return !(*this == other);
}

/* AdjacentEdgeConstIterator */

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
void DynamicGraph<V, E>::AdjacentEdgeConstIterator<Direction>::advance()
{
    ++m_slice_iter;
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
DynamicGraph<V, E>::AdjacentEdgeConstIterator<Direction>::AdjacentEdgeConstIterator() : m_edges(nullptr), m_slice_iter()
{
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
DynamicGraph<V, E>::AdjacentEdgeConstIterator<Direction>::AdjacentEdgeConstIterator(const std::unordered_map<EdgeIndex, E>& edges,
                                                                                    const std::unordered_set<EdgeIndex>& slice,
                                                                                    bool begin) :
    m_edges(&edges),
    m_slice_iter(begin ? slice.begin() : slice.end())
{
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
DynamicGraph<V, E>::AdjacentEdgeConstIterator<Direction>::reference DynamicGraph<V, E>::AdjacentEdgeConstIterator<Direction>::operator*() const
{
    assert(m_edges);

    return m_edges->at(*m_slice_iter);
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
DynamicGraph<V, E>::AdjacentEdgeConstIterator<Direction>& DynamicGraph<V, E>::AdjacentEdgeConstIterator<Direction>::operator++()
{
    advance();
    return *this;
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
DynamicGraph<V, E>::AdjacentEdgeConstIterator<Direction> DynamicGraph<V, E>::AdjacentEdgeConstIterator<Direction>::operator++(int)
{
    AdjacentEdgeConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
bool DynamicGraph<V, E>::AdjacentEdgeConstIterator<Direction>::operator==(const AdjacentEdgeConstIterator& other) const
{
    return (m_slice_iter == other.m_slice_iter);
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
bool DynamicGraph<V, E>::AdjacentEdgeConstIterator<Direction>::operator!=(const AdjacentEdgeConstIterator& other) const
{
    return !(*this == other);
}

/* AdjacentEdgeIndexConstIterator */

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
void DynamicGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction>::advance()
{
    ++m_slice_iter;
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
DynamicGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction>::AdjacentEdgeIndexConstIterator() : m_edges(nullptr), m_slice_iter()
{
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
DynamicGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction>::AdjacentEdgeIndexConstIterator(const std::unordered_map<EdgeIndex, E>& edges,
                                                                                              const std::unordered_set<EdgeIndex>& slice,
                                                                                              bool begin) :
    m_edges(&edges),
    m_slice_iter(begin ? slice.begin() : slice.end())
{
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
DynamicGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction>::value_type DynamicGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction>::operator*() const
{
    assert(m_edges);

    return m_edges->at(*m_slice_iter).get_index();
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
DynamicGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction>& DynamicGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction>::operator++()
{
    advance();
    return *this;
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
DynamicGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction> DynamicGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction>::operator++(int)
{
    AdjacentEdgeIndexConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
bool DynamicGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction>::operator==(const AdjacentEdgeIndexConstIterator& other) const
{
    return (m_slice_iter == other.m_slice_iter);
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
bool DynamicGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction>::operator!=(const AdjacentEdgeIndexConstIterator& other) const
{
    return !(*this == other);
}

template<IsVertex V, IsEdge E>
DynamicGraph<V, E>::DynamicGraph() :
    m_vertices(),
    m_free_vertices(),
    m_next_vertex_index(0),
    m_edges(),
    m_free_edges(),
    m_next_edge_index(0),
    m_adjacent_edges(),
    m_degrees()
{
}

template<IsVertex V, IsEdge E>
void DynamicGraph<V, E>::clear()
{
    m_vertices.clear();
    m_free_edges.clear();
    m_next_vertex_index = 0;
    m_edges.clear();
    m_free_edges.clear();
    m_next_edge_index = 0;
    boost::hana::at_key(m_adjacent_edges, boost::hana::type<ForwardTag> {}).clear();
    boost::hana::at_key(m_adjacent_edges, boost::hana::type<BackwardTag> {}).clear();
    boost::hana::at_key(m_degrees, boost::hana::type<ForwardTag> {}).clear();
    boost::hana::at_key(m_degrees, boost::hana::type<BackwardTag> {}).clear();
}

template<IsVertex V, IsEdge E>
template<typename... VertexProperties>
    requires HasVertexProperties<V, VertexProperties...>
VertexIndex DynamicGraph<V, E>::add_vertex(VertexProperties&&... properties)
{
    /* Get the vertex index. */
    auto index = m_free_vertices.empty() ? m_next_vertex_index++ : m_free_vertices.back();

    /* Create the vertex. */
    m_vertices.emplace(index, V(index, std::forward<VertexProperties>(properties)...));

    /* Initialize the data structures. */
    if (m_free_vertices.empty())
    {
        boost::hana::at_key(m_adjacent_edges, boost::hana::type<ForwardTag> {}).emplace(index, EdgeIndexSet());
        boost::hana::at_key(m_adjacent_edges, boost::hana::type<BackwardTag> {}).emplace(index, EdgeIndexSet());
        boost::hana::at_key(m_degrees, boost::hana::type<ForwardTag> {}).emplace(index, 0);
        boost::hana::at_key(m_degrees, boost::hana::type<BackwardTag> {}).emplace(index, 0);
    }
    else
    {
        boost::hana::at_key(m_adjacent_edges, boost::hana::type<ForwardTag> {}).at(index).clear();
        boost::hana::at_key(m_adjacent_edges, boost::hana::type<BackwardTag> {}).at(index).clear();
        boost::hana::at_key(m_degrees, boost::hana::type<ForwardTag> {}).at(index) = 0;
        boost::hana::at_key(m_degrees, boost::hana::type<BackwardTag> {}).at(index) = 0;
    }

    if (!m_free_vertices.empty())
    {
        // If m_free_vertices was non-empty, we additionally need to pop_back the used index.
        m_free_vertices.pop_back();
    }

    return index;
}

template<IsVertex V, IsEdge E>
template<typename... VertexProperties>
    requires HasVertexProperties<V, VertexProperties...>
VertexIndex DynamicGraph<V, E>::add_vertex(const Vertex<VertexProperties...>& vertex)
{
    return std::apply([this](auto&&... properties) { return this->add_vertex(std::forward<decltype(properties)>(properties)...); }, vertex.get_properties());
}

template<IsVertex V, IsEdge E>
template<typename... EdgeProperties>
    requires HasEdgeProperties<E, EdgeProperties...>
EdgeIndex DynamicGraph<V, E>::add_directed_edge(VertexIndex source, VertexIndex target, EdgeProperties&&... properties)
{
    vertex_index_check(source, "DynamicGraph<V, E>::add_directed_edge(...): Source vertex does not exist.");
    vertex_index_check(target, "DynamicGraph<V, E>::add_directed_edge(...): Target vertex does not exist.");

    /* Get the edge index */
    const auto index = m_free_edges.empty() ? m_next_edge_index++ : m_free_edges.back();

    /* Create the edge */
    m_edges.emplace(index, E(index, source, target, std::forward<EdgeProperties>(properties)...));

    /* Initialize the data structures. */
    boost::hana::at_key(m_adjacent_edges, boost::hana::type<ForwardTag> {}).at(source).insert(index);
    boost::hana::at_key(m_adjacent_edges, boost::hana::type<BackwardTag> {}).at(target).insert(index);
    ++boost::hana::at_key(m_degrees, boost::hana::type<ForwardTag> {}).at(source);
    ++boost::hana::at_key(m_degrees, boost::hana::type<BackwardTag> {}).at(target);

    if (!m_free_edges.empty())
    {
        // If m_free_edges was non-empty, we additionally need to pop_back the used index.
        m_free_edges.pop_back();
    }

    return index;
}

template<IsVertex V, IsEdge E>
template<typename... EdgeProperties>
    requires HasEdgeProperties<E, EdgeProperties...>
EdgeIndex DynamicGraph<V, E>::add_directed_edge(VertexIndex source, VertexIndex target, const Edge<EdgeProperties...>& edge)
{
    return std::apply([this, source, target](auto&&... properties)
                      { return this->add_directed_edge(source, target, std::forward<decltype(properties)>(properties)...); },
                      edge.get_properties());
}

template<IsVertex V, IsEdge E>
template<typename... EdgeProperties>
    requires HasEdgeProperties<E, EdgeProperties...>
std::pair<EdgeIndex, EdgeIndex> DynamicGraph<V, E>::add_undirected_edge(VertexIndex source, VertexIndex target, EdgeProperties&&... properties)
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
void DynamicGraph<V, E>::remove_vertex(VertexIndex vertex)
{
    vertex_index_check(vertex, "DynamicGraph<V, E>::remove_vertex(...): Vertex does not exist.");

    /* Remove backward adjacent edges from vertex of adjacent vertices */
    for (const auto& edge : get_adjacent_edge_indices<ForwardTag>(vertex))
    {
        const auto target = get_target<ForwardTag>(edge);
        if (target == vertex)
        {
            // Ignore loops over vertex.
            continue;
        }

        boost::hana::at_key(m_adjacent_edges, boost::hana::type<BackwardTag> {}).at(target).erase(edge);
        --boost::hana::at_key(m_degrees, boost::hana::type<BackwardTag> {}).at(target);
        m_edges.erase(edge);
        m_free_edges.push_back(edge);
    }
    /* Remove forward adjacent edges to vertex of adjacent vertices */
    for (const auto& edge : get_adjacent_edge_indices<BackwardTag>(vertex))
    {
        const auto target = get_target<BackwardTag>(edge);
        if (target == vertex)
        {
            // Ignore loops over vertex.
            continue;
        }

        boost::hana::at_key(m_adjacent_edges, boost::hana::type<ForwardTag> {}).at(target).erase(edge);
        --boost::hana::at_key(m_degrees, boost::hana::type<ForwardTag> {}).at(target);
        m_edges.erase(edge);
        m_free_edges.push_back(edge);
    }

    m_vertices.erase(vertex);
    m_free_vertices.push_back(vertex);
}

template<IsVertex V, IsEdge E>
void DynamicGraph<V, E>::remove_edge(EdgeIndex edge)
{
    edge_index_check(edge, "DynamicGraph<V, E>::remove_edge(...): Edge does not exist.");

    const auto source = get_source<ForwardTag>(edge);
    const auto target = get_target<ForwardTag>(edge);

    boost::hana::at_key(m_adjacent_edges, boost::hana::type<ForwardTag> {}).at(source).erase(edge);
    boost::hana::at_key(m_adjacent_edges, boost::hana::type<BackwardTag> {}).at(target).erase(edge);
    --boost::hana::at_key(m_degrees, boost::hana::type<ForwardTag> {}).at(source);
    --boost::hana::at_key(m_degrees, boost::hana::type<BackwardTag> {}).at(target);
    m_edges.erase(edge);
    m_free_edges.push_back(edge);
}

template<IsVertex V, IsEdge E>
DynamicGraph<V, E> DynamicGraph<V, E>::compute_induced_subgraph(const VertexIndexList& vertices) const
{
    auto subgraph = DynamicGraph<V, E>();

    auto remapping = std::unordered_map<VertexIndex, VertexIndex> {};

    for (const auto& v_idx : vertices)
    {
        remapping.emplace(v_idx, subgraph.add_vertex(get_vertex(v_idx)));
    }

    for (const auto& v_idx : vertices)
    {
        for (const auto& e : get_adjacent_edges<ForwardTag>(v_idx))
        {
            const auto src_v_idx = e.get_source();
            const auto dst_v_idx = e.get_target();

            if (remapping.contains(src_v_idx) && remapping.contains(dst_v_idx))
            {
                subgraph.add_directed_edge(remapping.at(src_v_idx), remapping.at(dst_v_idx), e);
            }
        }
    }

    return subgraph;
}

template<IsVertex V, IsEdge E>
std::ranges::subrange<typename DynamicGraph<V, E>::VertexIndexConstIterator> DynamicGraph<V, E>::get_vertex_indices() const
{
    return std::ranges::subrange(typename DynamicGraph<V, E>::VertexIndexConstIterator(m_vertices, true),
                                 typename DynamicGraph<V, E>::VertexIndexConstIterator(m_vertices, false));
}

template<IsVertex V, IsEdge E>
std::ranges::subrange<typename DynamicGraph<V, E>::EdgeIndexConstIterator> DynamicGraph<V, E>::get_edge_indices() const
{
    return std::ranges::subrange(typename DynamicGraph<V, E>::EdgeIndexConstIterator(m_edges, true),
                                 typename DynamicGraph<V, E>::EdgeIndexConstIterator(m_edges, false));
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
std::ranges::subrange<typename DynamicGraph<V, E>::template AdjacentVertexConstIterator<Direction>>
DynamicGraph<V, E>::get_adjacent_vertices(VertexIndex vertex) const
{
    vertex_index_check(vertex, "DynamicGraph<V, E>::get_adjacent_vertices(...): Vertex does not exist.");

    return std::ranges::subrange(
        typename DynamicGraph<V, E>::AdjacentVertexConstIterator<Direction>(m_vertices,
                                                                            m_edges,
                                                                            boost::hana::at_key(m_adjacent_edges, boost::hana::type<Direction> {}).at(vertex),
                                                                            true),
        typename DynamicGraph<V, E>::AdjacentVertexConstIterator<Direction>(m_vertices,
                                                                            m_edges,
                                                                            boost::hana::at_key(m_adjacent_edges, boost::hana::type<Direction> {}).at(vertex),
                                                                            false));
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
std::ranges::subrange<typename DynamicGraph<V, E>::template AdjacentVertexIndexConstIterator<Direction>>
DynamicGraph<V, E>::get_adjacent_vertex_indices(VertexIndex vertex) const
{
    vertex_index_check(vertex, "DynamicGraph<V, E>::get_adjacent_vertex_indices(...): Vertex does not exist.");

    return std::ranges::subrange(typename DynamicGraph<V, E>::AdjacentVertexIndexConstIterator<Direction>(
                                     m_edges,
                                     boost::hana::at_key(m_adjacent_edges, boost::hana::type<Direction> {}).at(vertex),
                                     true),
                                 typename DynamicGraph<V, E>::AdjacentVertexIndexConstIterator<Direction>(
                                     m_edges,
                                     boost::hana::at_key(m_adjacent_edges, boost::hana::type<Direction> {}).at(vertex),
                                     false));
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
std::ranges::subrange<typename DynamicGraph<V, E>::template AdjacentEdgeConstIterator<Direction>>
DynamicGraph<V, E>::get_adjacent_edges(VertexIndex vertex) const
{
    vertex_index_check(vertex, "DynamicGraph<V, E>::get_adjacent_edges(...): Vertex does not exist.");

    return std::ranges::subrange(
        typename DynamicGraph<V, E>::AdjacentEdgeConstIterator<Direction>(m_edges,
                                                                          boost::hana::at_key(m_adjacent_edges, boost::hana::type<Direction> {}).at(vertex),
                                                                          true),
        typename DynamicGraph<V, E>::AdjacentEdgeConstIterator<Direction>(m_edges,
                                                                          boost::hana::at_key(m_adjacent_edges, boost::hana::type<Direction> {}).at(vertex),
                                                                          false));
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
std::ranges::subrange<typename DynamicGraph<V, E>::template AdjacentEdgeIndexConstIterator<Direction>>
DynamicGraph<V, E>::get_adjacent_edge_indices(VertexIndex vertex) const
{
    vertex_index_check(vertex, "DynamicGraph<V, E>::get_adjacent_edge_indices(...): Vertex does not exist.");

    return std::ranges::subrange(typename DynamicGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction>(
                                     m_edges,
                                     boost::hana::at_key(m_adjacent_edges, boost::hana::type<Direction> {}).at(vertex),
                                     true),
                                 typename DynamicGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction>(
                                     m_edges,
                                     boost::hana::at_key(m_adjacent_edges, boost::hana::type<Direction> {}).at(vertex),
                                     false));
}

template<IsVertex V, IsEdge E>
const typename DynamicGraph<V, E>::VertexMap& DynamicGraph<V, E>::get_vertices() const
{
    return m_vertices;
}

template<IsVertex V, IsEdge E>
const V& DynamicGraph<V, E>::get_vertex(VertexIndex vertex) const
{
    vertex_index_check(vertex, "DynamicGraph<V, E>::get_vertex(...): Vertex does not exist.");

    return m_vertices.at(vertex);
}

template<IsVertex V, IsEdge E>
const typename DynamicGraph<V, E>::EdgeMap& DynamicGraph<V, E>::get_edges() const
{
    return m_edges;
}

template<IsVertex V, IsEdge E>
const E& DynamicGraph<V, E>::get_edge(EdgeIndex edge) const
{
    edge_index_check(edge, "DynamicGraph<V, E>::get_edge(...): Edge does not exist.");

    return m_edges.at(edge);
}

template<IsVertex V, IsEdge E>
size_t DynamicGraph<V, E>::get_num_vertices() const
{
    return m_vertices.size();
}

template<IsVertex V, IsEdge E>
size_t DynamicGraph<V, E>::get_num_edges() const
{
    return m_edges.size();
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
VertexIndex DynamicGraph<V, E>::get_source(EdgeIndex edge) const
{
    edge_index_check(edge, "DynamicGraph<V, E>::get_source(...): Edge does not exist.");

    if constexpr (std::is_same_v<Direction, ForwardTag>)
    {
        return m_edges.at(edge).get_source();
    }
    else if constexpr (std::is_same_v<Direction, BackwardTag>)
    {
        return m_edges.at(edge).get_target();
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "DynamicGraph<V, E>::get_source(...): Missing implementation for IsDirection.");
    }
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
VertexIndex DynamicGraph<V, E>::get_target(EdgeIndex edge) const
{
    edge_index_check(edge, "DynamicGraph<V, E>::get_target(...): Edge does not exist.");

    if constexpr (std::is_same_v<Direction, ForwardTag>)
    {
        return m_edges.at(edge).get_target();
    }
    else if constexpr (std::is_same_v<Direction, BackwardTag>)
    {
        return m_edges.at(edge).get_source();
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "DynamicGraph<V, E>::get_target(...): Missing implementation for IsDirection.");
    }
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
const DegreeMap& DynamicGraph<V, E>::get_degrees() const
{
    return boost::hana::at_key(m_degrees, boost::hana::type<Direction> {});
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
Degree DynamicGraph<V, E>::get_degree(VertexIndex vertex) const
{
    vertex_index_check(vertex, "DynamicGraph<V, E>::get_degree(...): Vertex does not exist.");

    return boost::hana::at_key(m_degrees, boost::hana::type<Direction> {}).at(vertex);
}

template<IsVertex V, IsEdge E>
void DynamicGraph<V, E>::vertex_index_check(VertexIndex vertex, const std::string& error_message) const
{
    if (!m_vertices.contains(vertex))
    {
        throw std::out_of_range(error_message);
    }
}

template<IsVertex V, IsEdge E>
void DynamicGraph<V, E>::edge_index_check(EdgeIndex edge, const std::string& error_message) const
{
    if (!m_edges.contains(edge))
    {
        throw std::out_of_range(error_message);
    }
}

template<IsVertex V, IsEdge E>
std::ostream& operator<<(std::ostream& out, const DynamicGraph<V, E>& graph)
{
    out << "digraph Tree {\n"
           "rankdir=TB;\n\n";

    /* Node definitions */
    for (const auto& [v_idx, v] : graph.get_vertices())
    {
        out << "n" << v.get_index() << " [label=\"" << v << "\"];\n";
    }
    out << "\n";

    /* Edge definitions */
    for (const auto& [e_idx, e] : graph.get_edges())
    {
        out << "n" << e.get_source() << " -> " << "n" << e.get_target() << " [label=\"" << e << "\"];\n";
    }
    out << "\n";

    out << "}\n";  // end graph

    return out;
}

}

#endif
