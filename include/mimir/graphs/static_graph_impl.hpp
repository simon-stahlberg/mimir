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

#ifndef MIMIR_GRAPHS_STATIC_GRAPH_IMPL_HPP_
#define MIMIR_GRAPHS_STATIC_GRAPH_IMPL_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/common/grouped_vector.hpp"
#include "mimir/graphs/graph_edge_interface.hpp"
#include "mimir/graphs/graph_edges.hpp"
#include "mimir/graphs/graph_traversal_interface.hpp"
#include "mimir/graphs/graph_vertex_interface.hpp"
#include "mimir/graphs/graph_vertices.hpp"
#include "mimir/graphs/static_graph_interface.hpp"

#include <boost/hana.hpp>
#include <cassert>
#include <ranges>
#include <span>
#include <vector>

namespace mimir::graphs
{

/* StaticGraph */

/* AdjacentVertexConstIterator */

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
void StaticGraph<V, E>::AdjacentVertexConstIterator<Direction>::AdjacentVertexConstIterator::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;

        if (m_pos == m_slice.size())
        {
            break;
        }

        if constexpr (std::is_same_v<Direction, ForwardTag>)
        {
            if (m_edges->at(m_slice[m_pos]).get_source() == m_vertex)
            {
                break;
            }
        }
        else if constexpr (std::is_same_v<Direction, BackwardTag>)
        {
            if (m_edges->at(m_slice[m_pos]).get_target() == m_vertex)
            {
                break;
            }
        }
        else
        {
            static_assert(dependent_false<Direction>::value,
                          "AdjacentVertexConstIterator<Vertex, Edge, Direction>::advance(): Missing implementation for "
                          "IsDirection.");
        }
    } while (true);
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
StaticGraph<V, E>::AdjacentVertexConstIterator<Direction>::AdjacentVertexConstIterator() :
    m_pos(-1),
    m_vertex(-1),
    m_vertices(nullptr),
    m_edges(nullptr),
    m_slice()
{
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
StaticGraph<V, E>::AdjacentVertexConstIterator<Direction>::AdjacentVertexConstIterator(VertexIndex vertex,
                                                                                       const std::vector<V>& vertices,
                                                                                       const std::vector<E>& edges,
                                                                                       std::span<const EdgeIndex> slice,
                                                                                       bool begin) :
    m_pos(begin ? 0 : slice.size()),
    m_vertex(vertex),
    m_vertices(&vertices),
    m_edges(&edges),
    m_slice(slice)
{
    if (begin && m_slice.size() > 0)
    {
        assert(m_vertices);
        assert(m_edges);

        if constexpr (std::is_same_v<Direction, ForwardTag>)
        {
            if (m_edges->at(m_slice[0]).get_source() == m_vertex)
            {
                return;
            }
        }
        else if constexpr (std::is_same_v<Direction, BackwardTag>)
        {
            if (m_edges->at(m_slice[0]).get_target() == m_vertex)
            {
                return;
            }
        }
        else
        {
            static_assert(dependent_false<Direction>::value,
                          "AdjacentVertexConstIterator<Vertex, Edge, Direction>::AdjacentVertexConstIterator(...): Missing implementation for "
                          "IsDirection.");
        }

        advance();
    }
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
StaticGraph<V, E>::AdjacentVertexConstIterator<Direction>::reference StaticGraph<V, E>::AdjacentVertexConstIterator<Direction>::operator*() const
{
    assert(m_vertices);
    assert(m_edges);
    assert(m_pos < m_slice.size());
    assert(m_slice[m_pos] < m_edges->size());

    if constexpr (std::is_same_v<Direction, ForwardTag>)
    {
        assert(m_edges->at(m_slice[m_pos]).get_source() == m_vertex);
        return m_vertices->at(m_edges->at(m_slice[m_pos]).get_target());
    }
    else if constexpr (std::is_same_v<Direction, BackwardTag>)
    {
        assert(m_edges->at(m_slice[m_pos]).get_target() == m_vertex);
        return m_vertices->at(m_edges->at(m_slice[m_pos]).get_source());
    }
    else
    {
        static_assert(dependent_false<Direction>::value,
                      "AdjacentVertexConstIterator<Vertex, Edge, Direction>::operator*(): Missing implementation for IsDirection.");
    }
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
StaticGraph<V, E>::AdjacentVertexConstIterator<Direction>& StaticGraph<V, E>::AdjacentVertexConstIterator<Direction>::operator++()
{
    advance();
    return *this;
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
StaticGraph<V, E>::AdjacentVertexConstIterator<Direction> StaticGraph<V, E>::AdjacentVertexConstIterator<Direction>::operator++(int)
{
    AdjacentVertexConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
bool StaticGraph<V, E>::AdjacentVertexConstIterator<Direction>::operator==(const AdjacentVertexConstIterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
bool StaticGraph<V, E>::AdjacentVertexConstIterator<Direction>::operator!=(const AdjacentVertexConstIterator& other) const
{
    return !(*this == other);
}

/* AdjacentVertexIndexConstIterator */

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
void StaticGraph<V, E>::AdjacentVertexIndexConstIterator<Direction>::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;

        if (m_pos == m_slice.size())
        {
            break;
        }

        if constexpr (std::is_same_v<Direction, ForwardTag>)
        {
            if (m_edges->at(m_slice[m_pos]).get_source() == m_vertex)
            {
                break;
            }
        }
        else if constexpr (std::is_same_v<Direction, BackwardTag>)
        {
            if (m_edges->at(m_slice[m_pos]).get_target() == m_vertex)
            {
                break;
            }
        }
        else
        {
            static_assert(dependent_false<Direction>::value,
                          "AdjacentVertexIndexConstIterator<Edge, Direction>::advance(): Missing implementation for IsDirection.");
        }
    } while (true);
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
StaticGraph<V, E>::AdjacentVertexIndexConstIterator<Direction>::AdjacentVertexIndexConstIterator() : m_pos(-1), m_vertex(-1), m_edges(nullptr), m_slice()
{
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
StaticGraph<V, E>::AdjacentVertexIndexConstIterator<Direction>::AdjacentVertexIndexConstIterator(VertexIndex vertex,
                                                                                                 const std::vector<E>& edges,
                                                                                                 std::span<const EdgeIndex> slice,
                                                                                                 bool begin) :
    m_pos(begin ? 0 : slice.size()),
    m_vertex(vertex),
    m_edges(&edges),
    m_slice(slice)
{
    if (begin && m_slice.size() > 0)
    {
        assert(m_edges);

        if constexpr (std::is_same_v<Direction, ForwardTag>)
        {
            if (m_edges->at(m_slice[0]).get_source() == m_vertex)
            {
                return;
            }
        }
        else if constexpr (std::is_same_v<Direction, BackwardTag>)
        {
            if (m_edges->at(m_slice[0]).get_target() == m_vertex)
            {
                return;
            }
        }
        else
        {
            static_assert(dependent_false<Direction>::value,
                          "AdjacentVertexIndexConstIterator<Edge, Direction>::AdjacentEdgeConstIterator(...): Missing implementation for "
                          "IsDirection.");
        }

        advance();
    }
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
StaticGraph<V, E>::AdjacentVertexIndexConstIterator<Direction>::value_type StaticGraph<V, E>::AdjacentVertexIndexConstIterator<Direction>::operator*() const
{
    assert(m_edges);
    assert(m_pos < m_slice.size());
    assert(m_slice[m_pos] < m_edges->size());

    if constexpr (std::is_same_v<Direction, ForwardTag>)
    {
        assert(m_edges->at(m_slice[m_pos]).get_source() == m_vertex);
        return m_edges->at(m_slice[m_pos]).get_target();
    }
    else if constexpr (std::is_same_v<Direction, BackwardTag>)
    {
        assert(m_edges->at(m_slice[m_pos]).get_target() == m_vertex);
        return m_edges->at(m_slice[m_pos]).get_source();
    }
    else
    {
        static_assert(dependent_false<Direction>::value,
                      "AdjacentVertexIndexConstIterator<Edge, Direction>::operator*(): Missing implementation for IsDirection.");
    }
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
StaticGraph<V, E>::AdjacentVertexIndexConstIterator<Direction>& StaticGraph<V, E>::AdjacentVertexIndexConstIterator<Direction>::operator++()
{
    advance();
    return *this;
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
StaticGraph<V, E>::AdjacentVertexIndexConstIterator<Direction> StaticGraph<V, E>::AdjacentVertexIndexConstIterator<Direction>::operator++(int)
{
    AdjacentVertexIndexConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
bool StaticGraph<V, E>::AdjacentVertexIndexConstIterator<Direction>::operator==(const AdjacentVertexIndexConstIterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
bool StaticGraph<V, E>::AdjacentVertexIndexConstIterator<Direction>::operator!=(const AdjacentVertexIndexConstIterator& other) const
{
    return !(*this == other);
}

/* AdjacentEdgeConstIterator */

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
void StaticGraph<V, E>::AdjacentEdgeConstIterator<Direction>::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;

        if (m_pos == m_slice.size())
        {
            break;
        }

        if constexpr (std::is_same_v<Direction, ForwardTag>)
        {
            if (m_edges->at(m_slice[m_pos]).get_source() == m_vertex)
            {
                break;
            }
        }
        else if constexpr (std::is_same_v<Direction, BackwardTag>)
        {
            if (m_edges->at(m_slice[m_pos]).get_target() == m_vertex)
            {
                break;
            }
        }
        else
        {
            static_assert(dependent_false<Direction>::value, "AdjacentEdgeConstIterator<Edge, Direction>::advance(): Missing implementation for IsDirection.");
        }
    } while (true);
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
StaticGraph<V, E>::AdjacentEdgeConstIterator<Direction>::AdjacentEdgeConstIterator() : m_pos(-1), m_vertex(-1), m_edges(nullptr), m_slice()
{
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
StaticGraph<V, E>::AdjacentEdgeConstIterator<Direction>::AdjacentEdgeConstIterator(VertexIndex vertex,
                                                                                   const std::vector<E>& edges,
                                                                                   std::span<const EdgeIndex> slice,
                                                                                   bool begin) :
    m_pos(begin ? 0 : slice.size()),
    m_vertex(vertex),
    m_edges(&edges),
    m_slice(slice)
{
    if (begin && m_slice.size() > 0)
    {
        assert(m_edges);

        if constexpr (std::is_same_v<Direction, ForwardTag>)
        {
            if (m_edges->at(m_slice[0]).get_source() == m_vertex)
            {
                return;
            }
        }
        else if constexpr (std::is_same_v<Direction, BackwardTag>)
        {
            if (m_edges->at(m_slice[0]).get_target() == m_vertex)
            {
                return;
            }
        }
        else
        {
            static_assert(dependent_false<Direction>::value,
                          "AdjacentEdgeConstIterator<Edge, Direction>::AdjacentEdgeConstIterator(...): Missing implementation for IsDirection.");
        }

        advance();
    }
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
StaticGraph<V, E>::AdjacentEdgeConstIterator<Direction>::reference StaticGraph<V, E>::AdjacentEdgeConstIterator<Direction>::operator*() const
{
    assert(m_edges);
    assert(m_pos < m_slice.size());
    assert(m_slice[m_pos] < m_edges->size());

    if constexpr (std::is_same_v<Direction, ForwardTag>)
    {
        assert(m_edges->at(m_slice[m_pos]).get_source() == m_vertex);
        return m_edges->at(m_slice[m_pos]);
    }
    else if constexpr (std::is_same_v<Direction, BackwardTag>)
    {
        assert(m_edges->at(m_slice[m_pos]).get_target() == m_vertex);
        return m_edges->at(m_slice[m_pos]);
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "AdjacentEdgeConstIterator<Edge, Direction>::operator*(): Missing implementation for IsDirection.");
    }
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
StaticGraph<V, E>::AdjacentEdgeConstIterator<Direction>& StaticGraph<V, E>::AdjacentEdgeConstIterator<Direction>::operator++()
{
    advance();
    return *this;
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
StaticGraph<V, E>::AdjacentEdgeConstIterator<Direction> StaticGraph<V, E>::AdjacentEdgeConstIterator<Direction>::operator++(int)
{
    AdjacentEdgeConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
bool StaticGraph<V, E>::AdjacentEdgeConstIterator<Direction>::operator==(const AdjacentEdgeConstIterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
bool StaticGraph<V, E>::AdjacentEdgeConstIterator<Direction>::operator!=(const AdjacentEdgeConstIterator& other) const
{
    return !(*this == other);
}

/* AdjacentEdgeIndexConstIterator */

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
void StaticGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction>::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;

        if (m_pos == m_slice.size())
        {
            break;
        }

        if constexpr (std::is_same_v<Direction, ForwardTag>)
        {
            if (m_edges->at(m_slice[m_pos]).get_source() == m_vertex)
            {
                break;
            }
        }
        else if constexpr (std::is_same_v<Direction, BackwardTag>)
        {
            if (m_edges->at(m_slice[m_pos]).get_target() == m_vertex)
            {
                break;
            }
        }
        else
        {
            static_assert(dependent_false<Direction>::value,
                          "AdjacentEdgeIndexConstIterator<Edge, Direction>::advance(): Missing implementation for IsDirection.");
        }
    } while (true);
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
StaticGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction>::AdjacentEdgeIndexConstIterator() : m_pos(-1), m_vertex(-1), m_edges(nullptr), m_slice()
{
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
StaticGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction>::AdjacentEdgeIndexConstIterator(VertexIndex vertex,
                                                                                             const std::vector<E>& edges,
                                                                                             std::span<const EdgeIndex> slice,
                                                                                             bool begin) :
    m_pos(begin ? 0 : slice.size()),
    m_vertex(vertex),
    m_edges(&edges),
    m_slice(slice)
{
    if (begin && m_slice.size() > 0)
    {
        assert(m_edges);

        if constexpr (std::is_same_v<Direction, ForwardTag>)
        {
            if (m_edges->at(m_slice[0]).get_source() == m_vertex)
            {
                return;
            }
        }
        else if constexpr (std::is_same_v<Direction, BackwardTag>)
        {
            if (m_edges->at(m_slice[0]).get_target() == m_vertex)
            {
                return;
            }
        }
        else
        {
            static_assert(dependent_false<Direction>::value,
                          "AdjacentEdgeIndexConstIterator<Edge, Direction>::const_iterator(...): Missing implementation for IsDirection.");
        }

        advance();
    }
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
StaticGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction>::value_type StaticGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction>::operator*() const
{
    assert(m_edges);
    assert(m_pos < m_slice.size());
    assert(m_slice[m_pos] < m_edges->size());

    if constexpr (std::is_same_v<Direction, ForwardTag>)
    {
        assert(m_edges->at(m_slice[m_pos]).get_source() == m_vertex);
        return m_edges->at(m_slice[m_pos]).get_index();
    }
    else if constexpr (std::is_same_v<Direction, BackwardTag>)
    {
        assert(m_edges->at(m_slice[m_pos]).get_target() == m_vertex);
        return m_edges->at(m_slice[m_pos]).get_index();
    }
    else
    {
        static_assert(dependent_false<Direction>::value,
                      "AdjacentEdgeIndexConstIterator<Edge, Direction>::operator*(): Missing implementation for IsDirection.");
    }
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
StaticGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction>& StaticGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction>::operator++()
{
    advance();
    return *this;
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
StaticGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction> StaticGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction>::operator++(int)
{
    AdjacentEdgeIndexConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
bool StaticGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction>::operator==(const AdjacentEdgeIndexConstIterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
bool StaticGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction>::operator!=(const AdjacentEdgeIndexConstIterator& other) const
{
    return !(*this == other);
}

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

    boost::hana::at_key(m_degrees, boost::hana::type<ForwardTag> {}).resize(index + 1, 0);
    boost::hana::at_key(m_degrees, boost::hana::type<BackwardTag> {}).resize(index + 1, 0);

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
    ++boost::hana::at_key(m_degrees, boost::hana::type<ForwardTag> {}).at(source);
    ++boost::hana::at_key(m_degrees, boost::hana::type<BackwardTag> {}).at(target);
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
    boost::hana::at_key(m_degrees, boost::hana::type<ForwardTag> {}).clear();
    boost::hana::at_key(m_degrees, boost::hana::type<BackwardTag> {}).clear();
    m_slice.clear();
}

template<IsVertex V, IsEdge E>
std::ranges::subrange<typename StaticGraph<V, E>::VertexIndexConstIterator> StaticGraph<V, E>::get_vertex_indices() const
{
    auto range = std::ranges::iota_view<VertexIndex, VertexIndex>(0, get_num_vertices());
    static_assert(std::ranges::borrowed_range<decltype(range)>);
    return std::ranges::subrange<VertexIndexConstIterator>(range.begin(), range.end());
}

template<IsVertex V, IsEdge E>
std::ranges::subrange<typename StaticGraph<V, E>::EdgeIndexConstIterator> StaticGraph<V, E>::get_edge_indices() const
{
    auto range = std::ranges::iota_view<EdgeIndex, EdgeIndex>(0, get_num_edges());
    static_assert(std::ranges::borrowed_range<decltype(range)>);
    return std::ranges::subrange<EdgeIndexConstIterator>(range.begin(), range.end());
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
std::ranges::subrange<typename StaticGraph<V, E>::template AdjacentVertexConstIterator<Direction>>
StaticGraph<V, E>::get_adjacent_vertices(VertexIndex vertex) const
{
    vertex_index_check(vertex, "StaticGraph<V, E>::get_adjacent_vertices(...): Vertex out of range");

    return std::ranges::subrange(typename StaticGraph<V, E>::AdjacentVertexConstIterator<Direction>(vertex, m_vertices, m_edges, m_slice, true),
                                 typename StaticGraph<V, E>::AdjacentVertexConstIterator<Direction>(vertex, m_vertices, m_edges, m_slice, false));
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
std::ranges::subrange<typename StaticGraph<V, E>::template AdjacentVertexIndexConstIterator<Direction>>
StaticGraph<V, E>::get_adjacent_vertex_indices(VertexIndex vertex) const
{
    vertex_index_check(vertex, "StaticGraph<V, E>::get_adjacent_vertex_indices(...): Vertex out of range");

    return std::ranges::subrange(typename StaticGraph<V, E>::AdjacentVertexIndexConstIterator<Direction>(vertex, m_edges, m_slice, true),
                                 typename StaticGraph<V, E>::AdjacentVertexIndexConstIterator<Direction>(vertex, m_edges, m_slice, false));
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
std::ranges::subrange<typename StaticGraph<V, E>::template AdjacentEdgeConstIterator<Direction>> StaticGraph<V, E>::get_adjacent_edges(VertexIndex vertex) const
{
    vertex_index_check(vertex, "StaticGraph<V, E>::get_adjacent_edges(...): Vertex out of range");

    return std::ranges::subrange(typename StaticGraph<V, E>::AdjacentEdgeConstIterator<Direction>(vertex, m_edges, m_slice, true),
                                 typename StaticGraph<V, E>::AdjacentEdgeConstIterator<Direction>(vertex, m_edges, m_slice, false));
}

template<IsVertex V, IsEdge E>
template<IsDirection Direction>
std::ranges::subrange<typename StaticGraph<V, E>::template AdjacentEdgeIndexConstIterator<Direction>>
StaticGraph<V, E>::get_adjacent_edge_indices(VertexIndex vertex) const
{
    vertex_index_check(vertex, "StaticGraph<V, E>::get_adjacent_edge_indices(...): Vertex out of range");

    return std::ranges::subrange(typename StaticGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction>(vertex, m_edges, m_slice, true),
                                 typename StaticGraph<V, E>::AdjacentEdgeIndexConstIterator<Direction>(vertex, m_edges, m_slice, false));
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

    if constexpr (std::is_same_v<Direction, ForwardTag>)
    {
        return m_edges[edge].get_source();
    }
    else if constexpr (std::is_same_v<Direction, BackwardTag>)
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

    if constexpr (std::is_same_v<Direction, ForwardTag>)
    {
        return m_edges[edge].get_target();
    }
    else if constexpr (std::is_same_v<Direction, BackwardTag>)
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
std::ranges::subrange<typename StaticForwardGraph<G>::VertexIndexConstIterator> StaticForwardGraph<G>::get_vertex_indices() const
{
    return m_graph.get_vertex_indices();
}

template<IsStaticGraph G>
std::ranges::subrange<typename StaticForwardGraph<G>::EdgeIndexConstIterator> StaticForwardGraph<G>::get_edge_indices() const
{
    return m_graph.get_edge_indices();
}

template<IsStaticGraph G>
template<IsDirection Direction>
std::ranges::subrange<typename StaticForwardGraph<G>::template AdjacentVertexConstIterator<Direction>>
StaticForwardGraph<G>::get_adjacent_vertices(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, ForwardTag>)
    {
        return std::ranges::subrange(typename StaticForwardGraph<G>::AdjacentVertexConstIterator<ForwardTag>(vertex,
                                                                                                             m_graph.get_vertices(),
                                                                                                             m_graph.get_edges(),
                                                                                                             m_edge_indices_grouped_by_source.at(vertex),
                                                                                                             true),
                                     typename StaticForwardGraph<G>::AdjacentVertexConstIterator<ForwardTag>(vertex,
                                                                                                             m_graph.get_vertices(),
                                                                                                             m_graph.get_edges(),
                                                                                                             m_edge_indices_grouped_by_source.at(vertex),
                                                                                                             false));
    }
    else if constexpr (std::is_same_v<Direction, BackwardTag>)
    {
        return m_graph.template get_adjacent_vertices<BackwardTag>(vertex);
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "StaticForwardGraph<G>::get_adjacent_vertices: Missing implementation for IsDirection.");
    }
}

template<IsStaticGraph G>
template<IsDirection Direction>
std::ranges::subrange<typename StaticForwardGraph<G>::template AdjacentVertexIndexConstIterator<Direction>>
StaticForwardGraph<G>::get_adjacent_vertex_indices(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, ForwardTag>)
    {
        return std::ranges::subrange(typename StaticForwardGraph<G>::AdjacentVertexIndexConstIterator<ForwardTag>(vertex,
                                                                                                                  m_graph.get_edges(),
                                                                                                                  m_edge_indices_grouped_by_source.at(vertex),
                                                                                                                  true),
                                     typename StaticForwardGraph<G>::AdjacentVertexIndexConstIterator<ForwardTag>(vertex,
                                                                                                                  m_graph.get_edges(),
                                                                                                                  m_edge_indices_grouped_by_source.at(vertex),
                                                                                                                  false));
    }
    else if constexpr (std::is_same_v<Direction, BackwardTag>)
    {
        return m_graph.template get_adjacent_vertex_indices<BackwardTag>(vertex);
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "StaticForwardGraph<G>::get_adjacent_vertex_indices: Missing implementation for IsDirection.");
    }
}

template<IsStaticGraph G>
template<IsDirection Direction>
std::ranges::subrange<typename StaticForwardGraph<G>::template AdjacentEdgeConstIterator<Direction>>
StaticForwardGraph<G>::get_adjacent_edges(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, ForwardTag>)
    {
        return std::ranges::subrange(typename StaticForwardGraph<G>::AdjacentEdgeConstIterator<ForwardTag>(vertex,
                                                                                                           m_graph.get_edges(),
                                                                                                           m_edge_indices_grouped_by_source.at(vertex),
                                                                                                           true),
                                     typename StaticForwardGraph<G>::AdjacentEdgeConstIterator<ForwardTag>(vertex,
                                                                                                           m_graph.get_edges(),
                                                                                                           m_edge_indices_grouped_by_source.at(vertex),
                                                                                                           false));
    }
    else if constexpr (std::is_same_v<Direction, BackwardTag>)
    {
        return m_graph.template get_adjacent_edges<BackwardTag>(vertex);
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "StaticForwardGraph<G>::get_adjacent_edges: Missing implementation for IsDirection.");
    }
}

template<IsStaticGraph G>
template<IsDirection Direction>
std::ranges::subrange<typename StaticForwardGraph<G>::template AdjacentEdgeIndexConstIterator<Direction>>
StaticForwardGraph<G>::get_adjacent_edge_indices(VertexIndex vertex) const
{
    if constexpr (std::is_same_v<Direction, ForwardTag>)
    {
        return std::ranges::subrange(typename StaticForwardGraph<G>::AdjacentEdgeIndexConstIterator<ForwardTag>(vertex,
                                                                                                                m_graph.get_edges(),
                                                                                                                m_edge_indices_grouped_by_source.at(vertex),
                                                                                                                true),
                                     typename StaticForwardGraph<G>::AdjacentEdgeIndexConstIterator<ForwardTag>(vertex,
                                                                                                                m_graph.get_edges(),
                                                                                                                m_edge_indices_grouped_by_source.at(vertex),
                                                                                                                false));
    }
    else if constexpr (std::is_same_v<Direction, BackwardTag>)
    {
        return m_graph.template get_adjacent_edge_indices<BackwardTag>(vertex);
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
    boost::hana::at_key(m_edge_indices_grouped_by_vertex, boost::hana::type<ForwardTag> {}) = std::move(compute_index_grouped_edge_indices(m_graph, true));
    boost::hana::at_key(m_edge_indices_grouped_by_vertex, boost::hana::type<BackwardTag> {}) = std::move(compute_index_grouped_edge_indices(m_graph, false));
}

template<IsStaticGraph G>
std::ranges::subrange<typename StaticBidirectionalGraph<G>::VertexIndexConstIterator> StaticBidirectionalGraph<G>::get_vertex_indices() const
{
    return m_graph.get_vertex_indices();
}

template<IsStaticGraph G>
std::ranges::subrange<typename StaticBidirectionalGraph<G>::EdgeIndexConstIterator> StaticBidirectionalGraph<G>::get_edge_indices() const
{
    return m_graph.get_edge_indices();
}

template<IsStaticGraph G>
template<IsDirection Direction>
std::ranges::subrange<typename StaticBidirectionalGraph<G>::template AdjacentVertexConstIterator<Direction>>
StaticBidirectionalGraph<G>::get_adjacent_vertices(VertexIndex vertex) const
{
    return std::ranges::subrange(typename StaticBidirectionalGraph<G>::AdjacentVertexConstIterator<Direction>(
                                     vertex,
                                     m_graph.get_vertices(),
                                     m_graph.get_edges(),
                                     boost::hana::at_key(m_edge_indices_grouped_by_vertex, boost::hana::type<Direction> {}).at(vertex),
                                     true),
                                 typename StaticBidirectionalGraph<G>::AdjacentVertexConstIterator<Direction>(
                                     vertex,
                                     m_graph.get_vertices(),
                                     m_graph.get_edges(),
                                     boost::hana::at_key(m_edge_indices_grouped_by_vertex, boost::hana::type<Direction> {}).at(vertex),
                                     false));
}

template<IsStaticGraph G>
template<IsDirection Direction>
std::ranges::subrange<typename StaticBidirectionalGraph<G>::template AdjacentVertexIndexConstIterator<Direction>>
StaticBidirectionalGraph<G>::get_adjacent_vertex_indices(VertexIndex vertex) const
{
    return std::ranges::subrange(typename StaticBidirectionalGraph<G>::AdjacentVertexIndexConstIterator<Direction>(
                                     vertex,
                                     m_graph.get_edges(),
                                     boost::hana::at_key(m_edge_indices_grouped_by_vertex, boost::hana::type<Direction> {}).at(vertex),
                                     true),
                                 typename StaticBidirectionalGraph<G>::AdjacentVertexIndexConstIterator<Direction>(
                                     vertex,
                                     m_graph.get_edges(),
                                     boost::hana::at_key(m_edge_indices_grouped_by_vertex, boost::hana::type<Direction> {}).at(vertex),
                                     false));
}

template<IsStaticGraph G>
template<IsDirection Direction>
std::ranges::subrange<typename StaticBidirectionalGraph<G>::template AdjacentEdgeConstIterator<Direction>>
StaticBidirectionalGraph<G>::get_adjacent_edges(VertexIndex vertex) const
{
    return std::ranges::subrange(typename StaticBidirectionalGraph<G>::AdjacentEdgeConstIterator<Direction>(
                                     vertex,
                                     m_graph.get_edges(),
                                     boost::hana::at_key(m_edge_indices_grouped_by_vertex, boost::hana::type<Direction> {}).at(vertex),
                                     true),
                                 typename StaticBidirectionalGraph<G>::AdjacentEdgeConstIterator<Direction>(
                                     vertex,
                                     m_graph.get_edges(),
                                     boost::hana::at_key(m_edge_indices_grouped_by_vertex, boost::hana::type<Direction> {}).at(vertex),
                                     false));
}

template<IsStaticGraph G>
template<IsDirection Direction>
std::ranges::subrange<typename StaticBidirectionalGraph<G>::template AdjacentEdgeIndexConstIterator<Direction>>
StaticBidirectionalGraph<G>::get_adjacent_edge_indices(VertexIndex vertex) const
{
    return std::ranges::subrange(typename StaticBidirectionalGraph<G>::AdjacentEdgeIndexConstIterator<Direction>(
                                     vertex,
                                     m_graph.get_edges(),
                                     boost::hana::at_key(m_edge_indices_grouped_by_vertex, boost::hana::type<Direction> {}).at(vertex),
                                     true),
                                 typename StaticBidirectionalGraph<G>::AdjacentEdgeIndexConstIterator<Direction>(
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
