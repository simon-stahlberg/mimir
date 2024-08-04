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

#ifndef MIMIR_GRAPHS_DYNAMIC_GRAPH_ITERATORS_HPP_
#define MIMIR_GRAPHS_DYNAMIC_GRAPH_ITERATORS_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/graphs/graph_edge_interface.hpp"
#include "mimir/graphs/graph_traversal_interface.hpp"
#include "mimir/graphs/graph_vertex_interface.hpp"

#include <cassert>
#include <unordered_map>
#include <unordered_set>

namespace mimir
{

template<IsVertex Vertex>
class DynamicVertexIndexIterator
{
private:
    std::reference_wrapper<const std::unordered_map<VertexIndex, Vertex>> m_vertices;

public:
    explicit DynamicVertexIndexIterator(const std::unordered_map<VertexIndex, Vertex>& vertices);

    class const_iterator
    {
    private:
        size_t m_pos;
        const std::unordered_map<VertexIndex, Vertex>* m_vertices;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = VertexIndex;
        using pointer = value_type*;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(const std::unordered_map<VertexIndex, Vertex>& vertices, bool begin);
        value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
    };

    const_iterator begin() const;
    const_iterator end() const;
};

template<IsEdge Edge>
class DynamicEdgeIndexIterator
{
private:
    std::reference_wrapper<const std::unordered_map<EdgeIndex, Edge>> m_edges;

public:
    explicit DynamicEdgeIndexIterator(const std::unordered_map<EdgeIndex, Edge>& edges);

    class const_iterator
    {
    private:
        size_t m_pos;
        const std::unordered_map<EdgeIndex, Edge>* m_edges;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = EdgeIndex;
        using pointer = value_type*;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(const std::unordered_map<EdgeIndex, Edge>& edges, bool begin);
        value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
    };

    const_iterator begin() const;
    const_iterator end() const;
};

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
class DynamicAdjacentVertexIterator
{
private:
    std::reference_wrapper<const std::unordered_map<VertexIndex, Vertex>> m_vertices;
    std::reference_wrapper<const std::unordered_map<EdgeIndex, Edge>> m_edges;
    std::reference_wrapper<const std::unordered_set<EdgeIndex>> m_slice;

public:
    DynamicAdjacentVertexIterator(const std::unordered_map<VertexIndex, Vertex>& vertices,
                                  const std::unordered_map<EdgeIndex, Edge>& edges,
                                  const std::unordered_set<EdgeIndex>& slice);

    template<IsTraversalDirection Direction_>
    class const_iterator
    {
    private:
        size_t m_pos;
        const std::unordered_map<VertexIndex, Vertex>* m_vertices;
        const std::unordered_map<EdgeIndex, Edge>* m_edges;
        const std::unordered_set<EdgeIndex>* m_slice;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = const Vertex;
        using pointer = value_type*;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(const std::unordered_map<VertexIndex, Vertex>& vertices,
                       const std::unordered_map<EdgeIndex, Edge>& edges,
                       const std::unordered_set<EdgeIndex>& slice,
                       bool begin);
        reference operator*() const;
        const_iterator<Direction_>& operator++();
        const_iterator<Direction_> operator++(int);
        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
    };

    const_iterator<Direction> begin() const;
    const_iterator<Direction> end() const;
};

template<IsEdge Edge, IsTraversalDirection Direction>
class DynamicAdjacentVertexIndexIterator
{
private:
    std::reference_wrapper<const std::unordered_map<EdgeIndex, Edge>> m_edges;
    std::reference_wrapper<const std::unordered_set<EdgeIndex>> m_slice;

public:
    DynamicAdjacentVertexIndexIterator(const std::unordered_map<EdgeIndex, Edge>& edges, const std::unordered_set<EdgeIndex>& slice);

    template<IsTraversalDirection Direction_>
    class const_iterator
    {
    private:
        size_t m_pos;
        std::reference_wrapper<const std::unordered_map<EdgeIndex, Edge>> m_edges;
        std::reference_wrapper<const std::unordered_set<EdgeIndex>> m_slice;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = VertexIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(const std::unordered_map<EdgeIndex, Edge>& edges, const std::unordered_set<EdgeIndex>& slice, bool begin);
        value_type operator*() const;
        const_iterator<Direction_>& operator++();
        const_iterator<Direction_> operator++(int);
        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
    };

    const_iterator<Direction> begin() const;
    const_iterator<Direction> end() const;
};

template<IsEdge Edge, IsTraversalDirection Direction>
class DynamicAdjacentEdgeIterator
{
private:
    std::reference_wrapper<const std::unordered_map<EdgeIndex, Edge>> m_edges;
    std::reference_wrapper<const std::unordered_set<EdgeIndex>> m_slice;

public:
    DynamicAdjacentEdgeIterator(const std::unordered_map<EdgeIndex, Edge>& edges, const std::unordered_set<EdgeIndex>& slice);

    template<IsTraversalDirection Direction_>
    class const_iterator
    {
    private:
        size_t m_pos;
        std::reference_wrapper<const std::unordered_map<EdgeIndex, Edge>> m_edges;
        std::reference_wrapper<const std::unordered_set<EdgeIndex>> m_slice;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = Edge;
        using pointer = value_type*;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(const std::unordered_map<EdgeIndex, Edge>& edges, const std::unordered_set<EdgeIndex>& slice, bool begin);
        reference operator*() const;
        const_iterator<Direction_>& operator++();
        const_iterator<Direction_> operator++(int);
        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
    };

    const_iterator<Direction> begin() const;
    const_iterator<Direction> end() const;
};

template<IsEdge Edge, IsTraversalDirection Direction>
class DynamicAdjacentEdgeIndexIterator
{
private:
    std::reference_wrapper<const std::unordered_map<EdgeIndex, Edge>> m_edges;
    std::reference_wrapper<const std::unordered_set<EdgeIndex>> m_slice;

public:
    DynamicAdjacentEdgeIndexIterator(const std::unordered_map<EdgeIndex, Edge>& edges, const std::unordered_set<EdgeIndex>& slice);

    template<IsTraversalDirection Direction_>
    class const_iterator
    {
    private:
        size_t m_pos;
        std::reference_wrapper<const std::unordered_map<EdgeIndex, Edge>> m_edges;
        std::reference_wrapper<const std::unordered_set<EdgeIndex>> m_slice;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = EdgeIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(const std::unordered_map<EdgeIndex, Edge>& edges, const std::unordered_set<EdgeIndex>& slice, bool begin);
        value_type operator*() const;
        const_iterator<Direction_>& operator++();
        const_iterator<Direction_> operator++(int);
        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
    };

    const_iterator<Direction> begin() const;
    const_iterator<Direction> end() const;
};

/**
 * Implementations
 */

/* DynamicVertexIndexIterator */

template<IsVertex Vertex>
DynamicVertexIndexIterator<Vertex>::DynamicVertexIndexIterator(const std::unordered_map<VertexIndex, Vertex>& vertices) : m_vertices(vertices)
{
}

template<IsVertex Vertex>
void DynamicVertexIndexIterator<Vertex>::const_iterator::advance()
{
    ++m_pos;
}

template<IsVertex Vertex>
DynamicVertexIndexIterator<Vertex>::const_iterator::const_iterator() : m_pos(-1), m_vertices(nullptr)
{
}

template<IsVertex Vertex>
DynamicVertexIndexIterator<Vertex>::const_iterator::const_iterator(const std::unordered_map<VertexIndex, Vertex>& vertices, bool begin) :
    m_pos(begin ? 0 : vertices.size()),
    m_vertices(&vertices)
{
}

template<IsVertex Vertex>
DynamicVertexIndexIterator<Vertex>::const_iterator::value_type DynamicVertexIndexIterator<Vertex>::const_iterator::operator*() const
{
    assert(m_vertices);
    assert(m_pos < m_vertices->size());
    return m_vertices->at(m_pos).get_index();
}

template<IsVertex Vertex>
DynamicVertexIndexIterator<Vertex>::const_iterator& DynamicVertexIndexIterator<Vertex>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsVertex Vertex>
DynamicVertexIndexIterator<Vertex>::const_iterator DynamicVertexIndexIterator<Vertex>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsVertex Vertex>
bool DynamicVertexIndexIterator<Vertex>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsVertex Vertex>
bool DynamicVertexIndexIterator<Vertex>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsVertex Vertex>
DynamicVertexIndexIterator<Vertex>::const_iterator DynamicVertexIndexIterator<Vertex>::begin() const
{
    return const_iterator(m_vertices, true);
}

template<IsVertex Vertex>
DynamicVertexIndexIterator<Vertex>::const_iterator DynamicVertexIndexIterator<Vertex>::end() const
{
    return const_iterator(m_vertices, false);
}

/* DynamicEdgeIndexIterator */

template<IsEdge Edge>
DynamicEdgeIndexIterator<Edge>::DynamicEdgeIndexIterator(const std::unordered_map<EdgeIndex, Edge>& edges) : m_edges(edges)
{
}

template<IsEdge Edge>
void DynamicEdgeIndexIterator<Edge>::const_iterator::advance()
{
    ++m_pos;
}

template<IsEdge Edge>
DynamicEdgeIndexIterator<Edge>::const_iterator::const_iterator() : m_pos(-1), m_edges(nullptr)
{
}

template<IsEdge Edge>
DynamicEdgeIndexIterator<Edge>::const_iterator::const_iterator(const std::unordered_map<EdgeIndex, Edge>& edges, bool begin) :
    m_pos(begin ? 0 : edges.size()),
    m_edges(&edges)
{
}

template<IsEdge Edge>
DynamicEdgeIndexIterator<Edge>::const_iterator::value_type DynamicEdgeIndexIterator<Edge>::const_iterator::operator*() const
{
    assert(m_edges);
    assert(m_pos < m_edges->size());
    return m_edges->at(m_pos).get_index();
}

template<IsEdge Edge>
DynamicEdgeIndexIterator<Edge>::const_iterator& DynamicEdgeIndexIterator<Edge>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge>
DynamicEdgeIndexIterator<Edge>::const_iterator DynamicEdgeIndexIterator<Edge>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge>
bool DynamicEdgeIndexIterator<Edge>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge>
bool DynamicEdgeIndexIterator<Edge>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsEdge Edge>
DynamicEdgeIndexIterator<Edge>::const_iterator DynamicEdgeIndexIterator<Edge>::begin() const
{
    return const_iterator(m_edges, true);
}

template<IsEdge Edge>
DynamicEdgeIndexIterator<Edge>::const_iterator DynamicEdgeIndexIterator<Edge>::end() const
{
    return const_iterator(m_edges, false);
}

/* DynamicAdjacentVertexIterator */

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::DynamicAdjacentVertexIterator(const std::unordered_map<VertexIndex, Vertex>& vertices,
                                                                                      const std::unordered_map<EdgeIndex, Edge>& edges,
                                                                                      const std::unordered_set<EdgeIndex>& slice) :
    m_vertices(vertices),
    m_edges(edges),
    m_slice(slice)
{
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
void DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>::advance()
{
    ++m_pos;
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>::const_iterator() :
    m_pos(-1),
    m_vertices(nullptr),
    m_edges(nullptr),
    m_slice()
{
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>::const_iterator(const std::unordered_map<VertexIndex, Vertex>& vertices,
                                                                                                   const std::unordered_map<EdgeIndex, Edge>& edges,
                                                                                                   const std::unordered_set<EdgeIndex>& slice,
                                                                                                   bool begin) :
    m_pos(begin ? 0 : slice.size()),
    m_vertices(&vertices),
    m_edges(&edges),
    m_slice(slice)
{
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>::reference
DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>::operator*() const
{
    assert(m_vertices);
    assert(m_edges);
    assert(m_pos < m_slice.size());

    return m_vertices->at(m_edges->at(m_slice[m_pos]).get_source());
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>&
DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>::operator++()
{
    advance();
    return *this;
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>
DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
bool DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
bool DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction> DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::begin() const
{
    return const_iterator<Direction>(m_vertices, m_edges, m_slice, true);
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction> DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::end() const
{
    return const_iterator<Direction>(m_vertices, m_edges, m_slice, false);
}

/* DynamicAdjacentVertexIndexIterator */

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexIndexIterator<Edge, Direction>::DynamicAdjacentVertexIndexIterator(const std::unordered_map<EdgeIndex, Edge>& edges,
                                                                                        const std::unordered_set<EdgeIndex>& slice) :
    m_edges(edges),
    m_slice(slice)
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
void DynamicAdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>::advance()
{
    ++m_pos;
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
DynamicAdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>::const_iterator() : m_pos(-1), m_edges(nullptr), m_slice()
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
DynamicAdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>::const_iterator(const std::unordered_map<EdgeIndex, Edge>& edges,
                                                                                                const std::unordered_set<EdgeIndex>& slice,
                                                                                                bool begin) :
    m_pos(begin ? 0 : slice.size()),
    m_edges(&edges),
    m_slice(slice)
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
DynamicAdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>::value_type
DynamicAdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>::operator*() const
{
    assert(m_edges);
    assert(m_pos < m_slice.size());

    return m_edges->at(m_slice[m_pos]).get_source();
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
DynamicAdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>&
DynamicAdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
DynamicAdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>
DynamicAdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
bool DynamicAdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
bool DynamicAdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction> DynamicAdjacentVertexIndexIterator<Edge, Direction>::begin() const
{
    return const_iterator<Direction>(m_edges, m_slice, true);
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction> DynamicAdjacentVertexIndexIterator<Edge, Direction>::end() const
{
    return const_iterator<Direction>(m_edges, m_slice, false);
}

/* DynamicAdjacentEdgeIterator */

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeIterator<Edge, Direction>::DynamicAdjacentEdgeIterator(const std::unordered_map<EdgeIndex, Edge>& edges,
                                                                          const std::unordered_set<EdgeIndex>& slice) :
    m_edges(edges),
    m_slice(slice)
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
void DynamicAdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_>::advance()
{
    ++m_pos;
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
DynamicAdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_>::const_iterator() : m_pos(-1), m_edges(nullptr), m_slice()
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
DynamicAdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_>::const_iterator(const std::unordered_map<EdgeIndex, Edge>& edges,
                                                                                         const std::unordered_set<EdgeIndex>& slice,
                                                                                         bool begin) :
    m_pos(begin ? 0 : slice.size()),
    m_edges(&edges),
    m_slice(slice)
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
DynamicAdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_>::reference
DynamicAdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_>::operator*() const
{
    assert(m_edges);
    assert(m_pos < m_slice.size());

    return m_edges->at(m_slice[m_pos]);
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
DynamicAdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_>& DynamicAdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_>::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
DynamicAdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_>
DynamicAdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_>::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
bool DynamicAdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_>::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
bool DynamicAdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_>::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction> DynamicAdjacentEdgeIterator<Edge, Direction>::begin() const
{
    return const_iterator<Direction>(m_vertex, m_edges, m_slice, true);
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction> DynamicAdjacentEdgeIterator<Edge, Direction>::end() const
{
    return const_iterator<Direction>(m_vertex, m_edges, m_slice, false);
}

/* DynamicAdjacentEdgeIndexIterator */

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeIndexIterator<Edge, Direction>::DynamicAdjacentEdgeIndexIterator(const std::unordered_map<EdgeIndex, Edge>& edges,
                                                                                    const std::unordered_set<EdgeIndex>& slice) :
    m_edges(edges),
    m_slice(slice)
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
void DynamicAdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_>::advance()
{
    ++m_pos;
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
DynamicAdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_>::const_iterator() : m_pos(-1), m_edges(nullptr), m_slice()
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
DynamicAdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_>::const_iterator(const std::unordered_map<EdgeIndex, Edge>& edges,
                                                                                              const std::unordered_set<EdgeIndex>& slice,
                                                                                              bool begin) :
    m_pos(begin ? 0 : slice.size()),
    m_edges(&edges),
    m_slice(slice)
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
DynamicAdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_>::value_type
DynamicAdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_>::operator*() const
{
    assert(m_edges);
    assert(m_pos < m_slice.size());

    return m_edges->at(m_slice[m_pos]).get_index();
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
DynamicAdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_>&
DynamicAdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_>::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
DynamicAdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_>
DynamicAdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_>::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
bool DynamicAdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_>::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
bool DynamicAdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_>::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction> DynamicAdjacentEdgeIndexIterator<Edge, Direction>::begin() const
{
    return const_iterator<Direction>(m_edges, m_slice, true);
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction> DynamicAdjacentEdgeIndexIterator<Edge, Direction>::end() const
{
    return const_iterator<Direction>(m_edges, m_slice, false);
}

}

#endif
