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
class TargetVertexIterator
{
private:
    VertexIndex m_source;
    std::reference_wrapper<const std::vector<Vertex>> m_vertices;
    std::reference_wrapper<const std::vector<Edge>> m_edges;
    std::span<const EdgeIndex> m_slice;

public:
    TargetVertexIterator(VertexIndex source, const std::vector<Vertex>& vertices, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice);

    class const_iterator
    {
    private:
        size_t m_pos;
        VertexIndex m_source;
        std::reference_wrapper<const std::vector<Vertex>> m_vertices;
        std::reference_wrapper<const std::vector<Edge>> m_edges;
        std::span<const EdgeIndex> m_slice;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = const Vertex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(VertexIndex source, const std::vector<Vertex>& vertices, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool begin);
        reference operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
    };

    const_iterator begin() const;
    const_iterator end() const;
};

template<IsVertex Vertex, IsEdge Edge>
class SourceVertexIterator
{
private:
    VertexIndex m_target;
    std::reference_wrapper<const std::vector<Vertex>> m_vertices;
    std::reference_wrapper<const std::vector<Edge>> m_edges;
    std::span<const EdgeIndex> m_slice;

public:
    SourceVertexIterator(VertexIndex target, const std::vector<Vertex>& vertices, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice);

    class const_iterator
    {
    private:
        size_t m_pos;
        VertexIndex m_target;
        std::reference_wrapper<const std::vector<Vertex>> m_vertices;
        std::reference_wrapper<const std::vector<Edge>> m_edges;
        std::span<const EdgeIndex> m_slice;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = const Vertex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(VertexIndex target, const std::vector<Vertex>& vertices, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool begin);
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
class ForwardEdgeIterator
{
private:
    VertexIndex m_source;
    std::reference_wrapper<const std::vector<Edge>> m_edges;
    std::span<const EdgeIndex> m_slice;

public:
    ForwardEdgeIterator(VertexIndex source, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice);

    class const_iterator
    {
    private:
        size_t m_pos;
        VertexIndex m_source;
        std::reference_wrapper<const std::vector<Edge>> m_edges;
        std::span<const EdgeIndex> m_slice;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = const Edge;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(VertexIndex source, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool begin);
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
class ForwardEdgeIndexIterator
{
private:
    VertexIndex m_source;
    std::reference_wrapper<const std::vector<Edge>> m_edges;
    std::span<const EdgeIndex> m_slice;

public:
    ForwardEdgeIndexIterator(VertexIndex source, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice);

    class const_iterator
    {
    private:
        size_t m_pos;
        VertexIndex m_source;
        std::reference_wrapper<const std::vector<Edge>> m_edges;
        std::span<const EdgeIndex> m_slice;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = EdgeIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(VertexIndex source, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool begin);
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
class BackwardEdgeIterator
{
private:
    VertexIndex m_target;
    std::reference_wrapper<const std::vector<Edge>> m_edges;
    std::span<const EdgeIndex> m_slice;

public:
    BackwardEdgeIterator(VertexIndex target, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice);

    class const_iterator
    {
    private:
        size_t m_pos;
        VertexIndex m_target;
        std::reference_wrapper<const std::vector<Edge>> m_edges;
        std::span<const EdgeIndex> m_slice;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = const Edge;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(VertexIndex target, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool begin);
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
class BackwardEdgeIndexIterator
{
private:
    VertexIndex m_target;
    std::reference_wrapper<const std::vector<Edge>> m_edges;
    std::span<const EdgeIndex> m_slice;

public:
    BackwardEdgeIndexIterator(VertexIndex target, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice);

    class const_iterator
    {
    private:
        size_t m_pos;
        VertexIndex m_target;
        std::reference_wrapper<const std::vector<Edge>> m_edges;
        std::span<const EdgeIndex> m_slice;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = EdgeIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(VertexIndex target, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool begin);
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

/* TargetVertexIterator */

template<IsVertex Vertex, IsEdge Edge>
TargetVertexIterator<Vertex, Edge>::TargetVertexIterator(VertexIndex source,
                                                         const std::vector<Vertex>& vertices,
                                                         const std::vector<Edge>& edges,
                                                         std::span<const EdgeIndex> slice) :
    m_source(source),
    m_vertices(vertices),
    m_edges(edges),
    m_slice(slice)
{
}

template<IsVertex Vertex, IsEdge Edge>
void TargetVertexIterator<Vertex, Edge>::const_iterator::advance()
{
    ++m_pos;
}

template<IsVertex Vertex, IsEdge Edge>
TargetVertexIterator<Vertex, Edge>::const_iterator::const_iterator()
{
}

template<IsVertex Vertex, IsEdge Edge>
TargetVertexIterator<Vertex, Edge>::const_iterator::const_iterator(VertexIndex source,
                                                                   const std::vector<Vertex>& vertices,
                                                                   const std::vector<Edge>& edges,
                                                                   std::span<const EdgeIndex> slice,
                                                                   bool begin) :
    m_pos(begin ? 0 : slice.size()),
    m_source(source),
    m_vertices(vertices),
    m_edges(edges),
    m_slice(slice)
{
}

template<IsVertex Vertex, IsEdge Edge>
TargetVertexIterator<Vertex, Edge>::const_iterator::reference TargetVertexIterator<Vertex, Edge>::const_iterator::operator*() const
{
    assert(m_pos < m_slice.size());
    assert(m_edges.get().at(m_slice[m_pos]).get_target() < m_vertices.get().size());
    assert(m_edges.get().at(m_slice[m_pos]).get_source() == m_source);
    return m_vertices.get().at(m_edges.get().at(m_slice[m_pos]).get_target());
}

template<IsVertex Vertex, IsEdge Edge>
TargetVertexIterator<Vertex, Edge>::const_iterator& TargetVertexIterator<Vertex, Edge>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsVertex Vertex, IsEdge Edge>
TargetVertexIterator<Vertex, Edge>::const_iterator TargetVertexIterator<Vertex, Edge>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsVertex Vertex, IsEdge Edge>
bool TargetVertexIterator<Vertex, Edge>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsVertex Vertex, IsEdge Edge>
bool TargetVertexIterator<Vertex, Edge>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsVertex Vertex, IsEdge Edge>
TargetVertexIterator<Vertex, Edge>::const_iterator TargetVertexIterator<Vertex, Edge>::begin() const
{
    return const_iterator(m_source, m_vertices, m_edges, m_slice, true);
}

template<IsVertex Vertex, IsEdge Edge>
TargetVertexIterator<Vertex, Edge>::const_iterator TargetVertexIterator<Vertex, Edge>::end() const
{
    return const_iterator(m_source, m_vertices, m_edges, m_slice, false);
}

/* SourceVertexIterator */

template<IsVertex Vertex, IsEdge Edge>
SourceVertexIterator<Vertex, Edge>::SourceVertexIterator(VertexIndex target,
                                                         const std::vector<Vertex>& vertices,
                                                         const std::vector<Edge>& edges,
                                                         std::span<const EdgeIndex> slice) :
    m_target(target),
    m_vertices(vertices),
    m_edges(edges),
    m_slice(slice)
{
}

template<IsVertex Vertex, IsEdge Edge>
void SourceVertexIterator<Vertex, Edge>::const_iterator::advance()
{
    ++m_pos;
}

template<IsVertex Vertex, IsEdge Edge>
SourceVertexIterator<Vertex, Edge>::const_iterator::const_iterator()
{
}

template<IsVertex Vertex, IsEdge Edge>
SourceVertexIterator<Vertex, Edge>::const_iterator::const_iterator(VertexIndex target,
                                                                   const std::vector<Vertex>& vertices,
                                                                   const std::vector<Edge>& edges,
                                                                   std::span<const EdgeIndex> slice,
                                                                   bool begin) :
    m_pos(begin ? 0 : slice.size()),
    m_target(target),
    m_vertices(vertices),
    m_edges(edges),
    m_slice(slice)
{
}

template<IsVertex Vertex, IsEdge Edge>
SourceVertexIterator<Vertex, Edge>::const_iterator::reference SourceVertexIterator<Vertex, Edge>::const_iterator::operator*() const
{
    assert(m_pos < m_slice.size());
    assert(m_edges.get().at(m_slice[m_pos]).get_source() < m_vertices.get().size());
    assert(m_edges.get().at(m_slice[m_pos]).get_target() == m_target);
    return m_vertices.get().at(m_edges.get().at(m_slice[m_pos]).get_source());
}

template<IsVertex Vertex, IsEdge Edge>
SourceVertexIterator<Vertex, Edge>::const_iterator& SourceVertexIterator<Vertex, Edge>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsVertex Vertex, IsEdge Edge>
SourceVertexIterator<Vertex, Edge>::const_iterator SourceVertexIterator<Vertex, Edge>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsVertex Vertex, IsEdge Edge>
bool SourceVertexIterator<Vertex, Edge>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsVertex Vertex, IsEdge Edge>
bool SourceVertexIterator<Vertex, Edge>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsVertex Vertex, IsEdge Edge>
SourceVertexIterator<Vertex, Edge>::const_iterator SourceVertexIterator<Vertex, Edge>::begin() const
{
    return const_iterator(m_target, m_vertices, m_edges, m_slice, true);
}

template<IsVertex Vertex, IsEdge Edge>
SourceVertexIterator<Vertex, Edge>::const_iterator SourceVertexIterator<Vertex, Edge>::end() const
{
    return const_iterator(m_target, m_vertices, m_edges, m_slice, false);
}

/* ForwardEdgeIterator */

template<IsEdge Edge>
ForwardEdgeIterator<Edge>::ForwardEdgeIterator(VertexIndex source, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice) :
    m_source(source),
    m_edges(edges),
    m_slice(slice)
{
}

template<IsEdge Edge>
void ForwardEdgeIterator<Edge>::const_iterator::advance()
{
    ++m_pos;
}

template<IsEdge Edge>
ForwardEdgeIterator<Edge>::const_iterator::const_iterator()
{
}

template<IsEdge Edge>
ForwardEdgeIterator<Edge>::const_iterator::const_iterator(VertexIndex source, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool begin) :
    m_pos(begin ? 0 : slice.size()),
    m_source(source),
    m_edges(edges),
    m_slice(slice)
{
}

template<IsEdge Edge>
ForwardEdgeIterator<Edge>::const_iterator::reference ForwardEdgeIterator<Edge>::const_iterator::operator*() const
{
    assert(m_pos < m_slice.size());
    assert(m_slice[m_pos] < m_edges.get().size());
    assert(m_edges.get().at(m_slice[m_pos]).get_source() == m_source);
    return m_edges.get().at(m_slice[m_pos]);
}

template<IsEdge Edge>
ForwardEdgeIterator<Edge>::const_iterator& ForwardEdgeIterator<Edge>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge>
ForwardEdgeIterator<Edge>::const_iterator ForwardEdgeIterator<Edge>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge>
bool ForwardEdgeIterator<Edge>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge>
bool ForwardEdgeIterator<Edge>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsEdge Edge>
ForwardEdgeIterator<Edge>::const_iterator ForwardEdgeIterator<Edge>::begin() const
{
    return const_iterator(m_source, m_edges, m_slice, true);
}

template<IsEdge Edge>
ForwardEdgeIterator<Edge>::const_iterator ForwardEdgeIterator<Edge>::end() const
{
    return const_iterator(m_source, m_edges, m_slice, false);
}

/* ForwardEdgeIndexIterator */

template<IsEdge Edge>
ForwardEdgeIndexIterator<Edge>::ForwardEdgeIndexIterator(VertexIndex source, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice) :
    m_source(source),
    m_edges(edges),
    m_slice(slice)
{
}

template<IsEdge Edge>
void ForwardEdgeIndexIterator<Edge>::const_iterator::advance()
{
    ++m_pos;
}

template<IsEdge Edge>
ForwardEdgeIndexIterator<Edge>::const_iterator::const_iterator()
{
}

template<IsEdge Edge>
ForwardEdgeIndexIterator<Edge>::const_iterator::const_iterator(VertexIndex source,
                                                               const std::vector<Edge>& edges,
                                                               std::span<const EdgeIndex> slice,
                                                               bool begin) :
    m_pos(begin ? 0 : slice.size()),
    m_source(source),
    m_edges(edges),
    m_slice(slice)
{
}

template<IsEdge Edge>
ForwardEdgeIndexIterator<Edge>::const_iterator::value_type ForwardEdgeIndexIterator<Edge>::const_iterator::operator*() const
{
    assert(m_pos < m_slice.size());
    assert(m_slice[m_pos] < m_edges.get().size());
    assert(m_edges.get().at(m_slice[m_pos]).get_source() == m_source);
    return m_edges.get().at(m_slice[m_pos]).get_index();
}

template<IsEdge Edge>
ForwardEdgeIndexIterator<Edge>::const_iterator& ForwardEdgeIndexIterator<Edge>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge>
ForwardEdgeIndexIterator<Edge>::const_iterator ForwardEdgeIndexIterator<Edge>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge>
bool ForwardEdgeIndexIterator<Edge>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge>
bool ForwardEdgeIndexIterator<Edge>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsEdge Edge>
ForwardEdgeIndexIterator<Edge>::const_iterator ForwardEdgeIndexIterator<Edge>::begin() const
{
    return const_iterator(m_source, m_edges, m_slice, true);
}

template<IsEdge Edge>
ForwardEdgeIndexIterator<Edge>::const_iterator ForwardEdgeIndexIterator<Edge>::end() const
{
    return const_iterator(m_source, m_edges, m_slice, false);
}

/* BackwardEdgeIterator */

template<IsEdge Edge>
BackwardEdgeIterator<Edge>::BackwardEdgeIterator(VertexIndex target, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice) :
    m_target(target),
    m_edges(edges),
    m_slice(slice)
{
}

template<IsEdge Edge>
void BackwardEdgeIterator<Edge>::const_iterator::advance()
{
    ++m_pos;
}

template<IsEdge Edge>
BackwardEdgeIterator<Edge>::const_iterator::const_iterator()
{
}

template<IsEdge Edge>
BackwardEdgeIterator<Edge>::const_iterator::const_iterator(VertexIndex target, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool begin) :
    m_pos(begin ? 0 : slice.size()),
    m_target(target),
    m_edges(edges),
    m_slice(slice)
{
}

template<IsEdge Edge>
BackwardEdgeIterator<Edge>::const_iterator::reference BackwardEdgeIterator<Edge>::const_iterator::operator*() const
{
    assert(m_pos < m_slice.size());
    assert(m_slice[m_pos] < m_edges.get().size());
    assert(m_edges.get().at(m_slice[m_pos]).get_target() == m_target);
    return m_edges.get().at(m_slice[m_pos]);
}

template<IsEdge Edge>
BackwardEdgeIterator<Edge>::const_iterator& BackwardEdgeIterator<Edge>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge>
BackwardEdgeIterator<Edge>::const_iterator BackwardEdgeIterator<Edge>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge>
bool BackwardEdgeIterator<Edge>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge>
bool BackwardEdgeIterator<Edge>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsEdge Edge>
BackwardEdgeIterator<Edge>::const_iterator BackwardEdgeIterator<Edge>::begin() const
{
    return const_iterator(m_target, m_edges, m_slice, true);
}

template<IsEdge Edge>
BackwardEdgeIterator<Edge>::const_iterator BackwardEdgeIterator<Edge>::end() const
{
    return const_iterator(m_target, m_edges, m_slice, false);
}

/* BackwardEdgeIndexIterator */

template<IsEdge Edge>
BackwardEdgeIndexIterator<Edge>::BackwardEdgeIndexIterator(VertexIndex target, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice) :
    m_target(target),
    m_edges(edges),
    m_slice(slice)
{
}

template<IsEdge Edge>
void BackwardEdgeIndexIterator<Edge>::const_iterator::advance()
{
    ++m_pos;
}

template<IsEdge Edge>
BackwardEdgeIndexIterator<Edge>::const_iterator::const_iterator()
{
}

template<IsEdge Edge>
BackwardEdgeIndexIterator<Edge>::const_iterator::const_iterator(VertexIndex target,
                                                                const std::vector<Edge>& edges,
                                                                std::span<const EdgeIndex> slice,
                                                                bool begin) :
    m_pos(begin ? 0 : slice.size()),
    m_target(target),
    m_edges(edges),
    m_slice(slice)
{
}

template<IsEdge Edge>
BackwardEdgeIndexIterator<Edge>::const_iterator::value_type BackwardEdgeIndexIterator<Edge>::const_iterator::operator*() const
{
    assert(m_pos < m_slice.size());
    assert(m_slice[m_pos] < m_edges.get().size());
    assert(m_edges.get().at(m_slice[m_pos]).get_target() == m_target);
    return m_edges.get().at(m_slice[m_pos]).get_index();
}

template<IsEdge Edge>
BackwardEdgeIndexIterator<Edge>::const_iterator& BackwardEdgeIndexIterator<Edge>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge>
BackwardEdgeIndexIterator<Edge>::const_iterator BackwardEdgeIndexIterator<Edge>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge>
bool BackwardEdgeIndexIterator<Edge>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge>
bool BackwardEdgeIndexIterator<Edge>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsEdge Edge>
BackwardEdgeIndexIterator<Edge>::const_iterator BackwardEdgeIndexIterator<Edge>::begin() const
{
    return const_iterator(m_target, m_edges, m_slice, true);
}

template<IsEdge Edge>
BackwardEdgeIndexIterator<Edge>::const_iterator BackwardEdgeIndexIterator<Edge>::end() const
{
    return const_iterator(m_target, m_edges, m_slice, false);
}

}

#endif
