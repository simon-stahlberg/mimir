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

#ifndef MIMIR_GRAPHS_GRAPH_ITERATORS_HPP_
#define MIMIR_GRAPHS_GRAPH_ITERATORS_HPP_

#include "mimir/graphs/graph_edge_interface.hpp"
#include "mimir/graphs/graph_vertex_interface.hpp"

#include <cassert>

namespace mimir
{

template<IsVertex Vertex, IsEdge Edge>
class VertexIterator
{
private:
    VertexIndex m_vertex;
    std::reference_wrapper<const std::vector<Vertex>> m_vertices;
    std::reference_wrapper<const std::vector<Edge>> m_edges;
    std::span<const EdgeIndex> m_slice;
    bool m_forward;

public:
    VertexIterator(VertexIndex vertex, const std::vector<Vertex>& vertices, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool forward);

    class const_iterator
    {
    private:
        size_t m_pos;
        VertexIndex m_vertex;
        const std::vector<Vertex>* m_vertices;
        const std::vector<Edge>* m_edges;
        std::span<const EdgeIndex> m_slice;
        bool m_forward;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = const Vertex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(VertexIndex vertex,
                       const std::vector<Vertex>& vertices,
                       const std::vector<Edge>& edges,
                       std::span<const EdgeIndex> slice,
                       bool forward,
                       bool begin);
        reference operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
    };

    const_iterator begin() const;
    const_iterator end() const;
};

template<IsEdge Edge>
class VertexIndexIterator
{
private:
    VertexIndex m_vertex;
    std::reference_wrapper<const std::vector<Edge>> m_edges;
    std::span<const EdgeIndex> m_slice;
    bool m_forward;

public:
    VertexIndexIterator(VertexIndex vertex, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool forward);

    class const_iterator
    {
    private:
        size_t m_pos;
        VertexIndex m_vertex;
        const std::vector<Edge>* m_edges;
        std::span<const EdgeIndex> m_slice;
        bool m_forward;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = VertexIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(VertexIndex vertex, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool forward, bool begin);
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
class EdgeIterator
{
private:
    VertexIndex m_vertex;
    std::reference_wrapper<const std::vector<Edge>> m_edges;
    std::span<const EdgeIndex> m_slice;
    bool m_forward;

public:
    EdgeIterator(VertexIndex vertex, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool forward);

    class const_iterator
    {
    private:
        size_t m_pos;
        VertexIndex m_vertex;
        const std::vector<Edge>* m_edges;
        std::span<const EdgeIndex> m_slice;
        bool m_forward;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = const Edge;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(VertexIndex vertex, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool forward, bool begin);
        reference operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
    };

    const_iterator begin() const;
    const_iterator end() const;
};

template<IsEdge Edge>
class EdgeIndexIterator
{
private:
    VertexIndex m_vertex;
    std::reference_wrapper<const std::vector<Edge>> m_edges;
    std::span<const EdgeIndex> m_slice;
    bool m_forward;

public:
    EdgeIndexIterator(VertexIndex vertex, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool forward);

    class const_iterator
    {
    private:
        size_t m_pos;
        VertexIndex m_vertex;
        const std::vector<Edge>* m_edges;
        std::span<const EdgeIndex> m_slice;
        bool m_forward;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = EdgeIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(VertexIndex vertex, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool forward, bool begin);
        value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
    };

    const_iterator begin() const;
    const_iterator end() const;
};

/**
 * Implementations
 */

/* VertexIterator */

template<IsVertex Vertex, IsEdge Edge>
VertexIterator<Vertex, Edge>::VertexIterator(VertexIndex vertex,
                                             const std::vector<Vertex>& vertices,
                                             const std::vector<Edge>& edges,
                                             std::span<const EdgeIndex> slice,
                                             bool forward) :
    m_vertex(vertex),
    m_vertices(vertices),
    m_edges(edges),
    m_slice(slice),
    m_forward(forward)
{
}

template<IsVertex Vertex, IsEdge Edge>
void VertexIterator<Vertex, Edge>::const_iterator::advance()
{
    ++m_pos;
}

template<IsVertex Vertex, IsEdge Edge>
VertexIterator<Vertex, Edge>::const_iterator::const_iterator() : m_pos(-1), m_vertex(-1), m_vertices(nullptr), m_edges(nullptr), m_slice(), m_forward(false)
{
}

template<IsVertex Vertex, IsEdge Edge>
VertexIterator<Vertex, Edge>::const_iterator::const_iterator(VertexIndex vertex,
                                                             const std::vector<Vertex>& vertices,
                                                             const std::vector<Edge>& edges,
                                                             std::span<const EdgeIndex> slice,
                                                             bool forward,
                                                             bool begin) :
    m_pos(begin ? 0 : slice.size()),
    m_vertex(vertex),
    m_vertices(&vertices),
    m_edges(&edges),
    m_slice(slice),
    m_forward(forward)
{
}

template<IsVertex Vertex, IsEdge Edge>
VertexIterator<Vertex, Edge>::const_iterator::reference VertexIterator<Vertex, Edge>::const_iterator::operator*() const
{
    assert(m_vertices);
    assert(m_edges);
    assert(m_pos < m_slice.size());

    if (m_forward)
    {
        assert(m_edges->at(m_slice[m_pos]).get_target() < m_vertices->size());
        assert(m_edges->at(m_slice[m_pos]).get_source() == m_vertex);
        return m_vertices->at(m_edges->at(m_slice[m_pos]).get_target());
    }
    else
    {
        assert(m_edges->at(m_slice[m_pos]).get_source() < m_vertices->size());
        assert(m_edges->at(m_slice[m_pos]).get_target() == m_vertex);
        return m_vertices->at(m_edges->at(m_slice[m_pos]).get_source());
    }
}

template<IsVertex Vertex, IsEdge Edge>
VertexIterator<Vertex, Edge>::const_iterator& VertexIterator<Vertex, Edge>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsVertex Vertex, IsEdge Edge>
VertexIterator<Vertex, Edge>::const_iterator VertexIterator<Vertex, Edge>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsVertex Vertex, IsEdge Edge>
bool VertexIterator<Vertex, Edge>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsVertex Vertex, IsEdge Edge>
bool VertexIterator<Vertex, Edge>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsVertex Vertex, IsEdge Edge>
VertexIterator<Vertex, Edge>::const_iterator VertexIterator<Vertex, Edge>::begin() const
{
    return const_iterator(m_vertex, m_vertices, m_edges, m_slice, m_forward, true);
}

template<IsVertex Vertex, IsEdge Edge>
VertexIterator<Vertex, Edge>::const_iterator VertexIterator<Vertex, Edge>::end() const
{
    return const_iterator(m_vertex, m_vertices, m_edges, m_slice, m_forward, false);
}

/* VertexIndexIterator */

template<IsEdge Edge>
VertexIndexIterator<Edge>::VertexIndexIterator(VertexIndex vertex, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool forward) :
    m_vertex(vertex),
    m_edges(edges),
    m_slice(slice),
    m_forward(forward)
{
}

template<IsEdge Edge>
void VertexIndexIterator<Edge>::const_iterator::advance()
{
    ++m_pos;
}

template<IsEdge Edge>
VertexIndexIterator<Edge>::const_iterator::const_iterator() : m_pos(-1), m_vertex(-1), m_edges(nullptr), m_slice(), m_forward(false)
{
}

template<IsEdge Edge>
VertexIndexIterator<Edge>::const_iterator::const_iterator(VertexIndex vertex,
                                                          const std::vector<Edge>& edges,
                                                          std::span<const EdgeIndex> slice,
                                                          bool forward,
                                                          bool begin) :
    m_pos(begin ? 0 : slice.size()),
    m_vertex(vertex),
    m_edges(&edges),
    m_slice(slice),
    m_forward(forward)
{
}

template<IsEdge Edge>
VertexIndexIterator<Edge>::const_iterator::value_type VertexIndexIterator<Edge>::const_iterator::operator*() const
{
    assert(m_edges);
    assert(m_pos < m_slice.size());

    if (m_forward)
    {
        assert(m_edges->at(m_slice[m_pos]).get_source() == m_vertex);
        return m_edges->at(m_slice[m_pos]).get_target();
    }
    else
    {
        assert(m_edges->at(m_slice[m_pos]).get_target() == m_vertex);
        return m_edges->at(m_slice[m_pos]).get_source();
    }
}

template<IsEdge Edge>
VertexIndexIterator<Edge>::const_iterator& VertexIndexIterator<Edge>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge>
VertexIndexIterator<Edge>::const_iterator VertexIndexIterator<Edge>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge>
bool VertexIndexIterator<Edge>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge>
bool VertexIndexIterator<Edge>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsEdge Edge>
VertexIndexIterator<Edge>::const_iterator VertexIndexIterator<Edge>::begin() const
{
    return const_iterator(m_vertex, m_edges, m_slice, m_forward, true);
}

template<IsEdge Edge>
VertexIndexIterator<Edge>::const_iterator VertexIndexIterator<Edge>::end() const
{
    return const_iterator(m_vertex, m_edges, m_slice, m_forward, false);
}

/* EdgeIterator */

template<IsEdge Edge>
EdgeIterator<Edge>::EdgeIterator(VertexIndex vertex, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool forward) :
    m_vertex(vertex),
    m_edges(edges),
    m_slice(slice),
    m_forward(forward)
{
}

template<IsEdge Edge>
void EdgeIterator<Edge>::const_iterator::advance()
{
    ++m_pos;
}

template<IsEdge Edge>
EdgeIterator<Edge>::const_iterator::const_iterator() : m_pos(-1), m_vertex(-1), m_edges(nullptr), m_slice(), m_forward(false)
{
}

template<IsEdge Edge>
EdgeIterator<Edge>::const_iterator::const_iterator(VertexIndex vertex,
                                                   const std::vector<Edge>& edges,
                                                   std::span<const EdgeIndex> slice,
                                                   bool forward,
                                                   bool begin) :
    m_pos(begin ? 0 : slice.size()),
    m_vertex(vertex),
    m_edges(&edges),
    m_slice(slice),
    m_forward(forward)
{
}

template<IsEdge Edge>
EdgeIterator<Edge>::const_iterator::reference EdgeIterator<Edge>::const_iterator::operator*() const
{
    assert(m_edges);
    assert(m_pos < m_slice.size());
    assert(m_slice[m_pos] < m_edges->size());

    if (m_forward)
    {
        assert(m_edges->at(m_slice[m_pos]).get_source() == m_vertex);
        return m_edges->at(m_slice[m_pos]);
    }
    else
    {
        assert(m_edges->at(m_slice[m_pos]).get_target() == m_vertex);
        return m_edges->at(m_slice[m_pos]);
    }
}

template<IsEdge Edge>
EdgeIterator<Edge>::const_iterator& EdgeIterator<Edge>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge>
EdgeIterator<Edge>::const_iterator EdgeIterator<Edge>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge>
bool EdgeIterator<Edge>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge>
bool EdgeIterator<Edge>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsEdge Edge>
EdgeIterator<Edge>::const_iterator EdgeIterator<Edge>::begin() const
{
    return const_iterator(m_vertex, m_edges, m_slice, m_forward, true);
}

template<IsEdge Edge>
EdgeIterator<Edge>::const_iterator EdgeIterator<Edge>::end() const
{
    return const_iterator(m_vertex, m_edges, m_slice, m_forward, false);
}

/* EdgeIndexIterator */

template<IsEdge Edge>
EdgeIndexIterator<Edge>::EdgeIndexIterator(VertexIndex vertex, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool forward) :
    m_vertex(vertex),
    m_edges(edges),
    m_slice(slice),
    m_forward(forward)
{
}

template<IsEdge Edge>
void EdgeIndexIterator<Edge>::const_iterator::advance()
{
    ++m_pos;
}

template<IsEdge Edge>
EdgeIndexIterator<Edge>::const_iterator::const_iterator() : m_pos(-1), m_vertex(-1), m_edges(nullptr), m_slice(), m_forward(false)
{
}

template<IsEdge Edge>
EdgeIndexIterator<Edge>::const_iterator::const_iterator(VertexIndex vertex,
                                                        const std::vector<Edge>& edges,
                                                        std::span<const EdgeIndex> slice,
                                                        bool forward,
                                                        bool begin) :
    m_pos(begin ? 0 : slice.size()),
    m_vertex(vertex),
    m_edges(&edges),
    m_slice(slice),
    m_forward(forward)
{
}

template<IsEdge Edge>
EdgeIndexIterator<Edge>::const_iterator::value_type EdgeIndexIterator<Edge>::const_iterator::operator*() const
{
    assert(m_edges);
    assert(m_pos < m_slice.size());
    assert(m_slice[m_pos] < m_edges->size());

    if (m_forward)
    {
        assert(m_edges->at(m_slice[m_pos]).get_source() == m_vertex);
        return m_edges->at(m_slice[m_pos]).get_index();
    }
    else
    {
        assert(m_edges->at(m_slice[m_pos]).get_target() == m_vertex);
        return m_edges->at(m_slice[m_pos]).get_index();
    }
}

template<IsEdge Edge>
EdgeIndexIterator<Edge>::const_iterator& EdgeIndexIterator<Edge>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge>
EdgeIndexIterator<Edge>::const_iterator EdgeIndexIterator<Edge>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge>
bool EdgeIndexIterator<Edge>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge>
bool EdgeIndexIterator<Edge>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsEdge Edge>
EdgeIndexIterator<Edge>::const_iterator EdgeIndexIterator<Edge>::begin() const
{
    return const_iterator(m_vertex, m_edges, m_slice, m_forward, true);
}

template<IsEdge Edge>
EdgeIndexIterator<Edge>::const_iterator EdgeIndexIterator<Edge>::end() const
{
    return const_iterator(m_vertex, m_edges, m_slice, m_forward, false);
}

}

#endif
