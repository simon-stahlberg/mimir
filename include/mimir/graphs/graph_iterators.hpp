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
    std::span<const Vertex> m_vertices;
    std::span<const Edge> m_edges;

public:
    TargetVertexIterator(VertexIndex source, std::span<const Vertex> vertices, std::span<const Edge> edges);

    class const_iterator
    {
    private:
        VertexIndex m_source;
        size_t m_pos;
        std::span<const Vertex> m_vertices;
        std::span<const Edge> m_edges;

        void advance();

    public:
        using difference_type = int;
        using value_type = const Vertex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(VertexIndex source, std::span<const Vertex> vertices, std::span<const Edge> edges, bool begin);
        [[nodiscard]] reference operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        [[nodiscard]] bool operator==(const const_iterator& other) const;
        [[nodiscard]] bool operator!=(const const_iterator& other) const;
    };

    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;
};

// static_assert(std::ranges::forward_range<TargetVertexIterator<Edge>>);

template<IsVertex Vertex, IsEdge Edge>
class SourceVertexIterator
{
private:
    VertexIndex m_target;
    std::span<const Vertex> m_vertices;
    std::span<const Edge> m_edges;

public:
    SourceVertexIterator(VertexIndex target, std::span<const Vertex> vertices, std::span<const Edge> edges);

    class const_iterator
    {
    private:
        VertexIndex m_target;
        size_t m_pos;
        std::span<const Vertex> m_vertices;
        std::span<const Edge> m_edges;

        void advance();

    public:
        using difference_type = int;
        using value_type = const Vertex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(VertexIndex target, std::span<const Vertex> vertices, std::span<const Edge> edges, bool begin);
        [[nodiscard]] reference operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        [[nodiscard]] bool operator==(const const_iterator& other) const;
        [[nodiscard]] bool operator!=(const const_iterator& other) const;
    };

    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;
};

template<IsEdge Edge>
class ForwardEdgeIterator
{
private:
    VertexIndex m_source;
    std::span<const Edge> m_edges;

public:
    ForwardEdgeIterator(VertexIndex source, std::span<const Edge> edges);

    class const_iterator
    {
    private:
        VertexIndex m_source;
        size_t m_pos;
        std::span<const Edge> m_edges;

        void advance();

    public:
        using difference_type = int;
        using value_type = const Edge;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(VertexIndex source, std::span<const Edge> edges, bool begin);
        [[nodiscard]] reference operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        [[nodiscard]] bool operator==(const const_iterator& other) const;
        [[nodiscard]] bool operator!=(const const_iterator& other) const;
    };

    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;
};

// static_assert(std::ranges::forward_range<ForwardEdgeIterator<Edge>>);

template<IsEdge Edge>
class BackwardEdgeIterator
{
private:
    VertexIndex m_target;
    std::span<const Edge> m_edges;

public:
    BackwardEdgeIterator(VertexIndex target, std::span<const Edge> edges);

    class const_iterator
    {
    private:
        VertexIndex m_target;
        size_t m_pos;
        std::span<const Edge> m_edges;

        void advance();

    public:
        using difference_type = int;
        using value_type = const Edge;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(VertexIndex target, std::span<const Edge> edges, bool begin);
        [[nodiscard]] reference operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        [[nodiscard]] bool operator==(const const_iterator& other) const;
        [[nodiscard]] bool operator!=(const const_iterator& other) const;
    };

    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;
};

// static_assert(std::ranges::forward_range<BackwardEdgeIterator<Edge>>);

/**
 * Implementations
 */

/* TargetVertexIterator */

template<IsVertex Vertex, IsEdge Edge>
TargetVertexIterator<Vertex, Edge>::TargetVertexIterator(VertexIndex source, std::span<const Vertex> vertices, std::span<const Edge> edges) :
    m_source(source),
    m_vertices(vertices),
    m_edges(edges)
{
}

template<IsVertex Vertex, IsEdge Edge>
void TargetVertexIterator<Vertex, Edge>::const_iterator::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;
    } while (m_pos < m_edges.size() && m_edges[m_pos].get_source() != m_source);
}

template<IsVertex Vertex, IsEdge Edge>
TargetVertexIterator<Vertex, Edge>::const_iterator::const_iterator()
{
}

template<IsVertex Vertex, IsEdge Edge>
TargetVertexIterator<Vertex, Edge>::const_iterator::const_iterator(VertexIndex source,
                                                                   std::span<const Vertex> vertices,
                                                                   std::span<const Edge> edges,
                                                                   bool begin) :
    m_source(source),
    m_pos(begin ? 0 : edges.size()),
    m_vertices(vertices),
    m_edges(edges)
{
    if (begin && m_edges.size() > 0 && m_edges[0].get_source() != m_source)
    {
        advance();
    }
}

template<IsVertex Vertex, IsEdge Edge>
TargetVertexIterator<Vertex, Edge>::const_iterator::reference TargetVertexIterator<Vertex, Edge>::const_iterator::operator*() const
{
    assert(m_pos < m_edges.size());
    assert(m_edges[m_pos].get_target() < m_vertices.size());
    return m_vertices[m_edges[m_pos].get_target()];
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
    return const_iterator(m_source, m_vertices, m_edges, true);
}

template<IsVertex Vertex, IsEdge Edge>
TargetVertexIterator<Vertex, Edge>::const_iterator TargetVertexIterator<Vertex, Edge>::end() const
{
    return const_iterator(m_source, m_vertices, m_edges, false);
}

/* SourceStateIndexIterator */

template<IsVertex Vertex, IsEdge Edge>
SourceVertexIterator<Vertex, Edge>::SourceVertexIterator(VertexIndex target, std::span<const Vertex> vertices, std::span<const Edge> edges) :
    m_target(target),
    m_vertices(vertices),
    m_edges(edges)
{
}

template<IsVertex Vertex, IsEdge Edge>
void SourceVertexIterator<Vertex, Edge>::const_iterator::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;
    } while (m_pos < m_edges.size() && m_edges[m_pos].get_target() != m_target);
}

template<IsVertex Vertex, IsEdge Edge>
SourceVertexIterator<Vertex, Edge>::const_iterator::const_iterator()
{
}

template<IsVertex Vertex, IsEdge Edge>
SourceVertexIterator<Vertex, Edge>::const_iterator::const_iterator(VertexIndex target,
                                                                   std::span<const Vertex> vertices,
                                                                   std::span<const Edge> edges,
                                                                   bool begin) :
    m_target(target),
    m_pos(begin ? 0 : edges.size()),
    m_vertices(vertices),
    m_edges(edges)
{
    if (begin && m_edges.size() > 0 && m_edges[0].get_target() != m_target)
    {
        advance();
    }
}

template<IsVertex Vertex, IsEdge Edge>
SourceVertexIterator<Vertex, Edge>::const_iterator::reference SourceVertexIterator<Vertex, Edge>::const_iterator::operator*() const
{
    assert(m_pos < m_edges.size());
    assert(m_edges[m_pos].get_source() < m_vertices.size());
    return m_vertices[m_edges[m_pos].get_source()];
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
    return const_iterator(m_target, m_vertices, m_edges, true);
}

template<IsVertex Vertex, IsEdge Edge>
SourceVertexIterator<Vertex, Edge>::const_iterator SourceVertexIterator<Vertex, Edge>::end() const
{
    return const_iterator(m_target, m_vertices, m_edges, false);
}

/* ForwardEdgeIterator */

template<IsEdge Edge>
ForwardEdgeIterator<Edge>::ForwardEdgeIterator(VertexIndex source, std::span<const Edge> edges) : m_source(source), m_edges(edges)
{
}

template<IsEdge Edge>
void ForwardEdgeIterator<Edge>::const_iterator::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;
    } while (m_pos < m_edges.size() && m_edges[m_pos].get_source() != m_source);
}

template<IsEdge Edge>
ForwardEdgeIterator<Edge>::const_iterator::const_iterator()
{
}

template<IsEdge Edge>
ForwardEdgeIterator<Edge>::const_iterator::const_iterator(VertexIndex source, std::span<const Edge> edges, bool begin) :
    m_source(source),
    m_pos(begin ? 0 : edges.size()),
    m_edges(edges)
{
    if (begin && m_edges.size() > 0 && m_edges[0].get_source() != m_source)
    {
        advance();
    }
}

template<IsEdge Edge>
ForwardEdgeIterator<Edge>::const_iterator::reference ForwardEdgeIterator<Edge>::const_iterator::operator*() const
{
    assert(m_pos < m_edges.size());
    return m_edges[m_pos];
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
    return const_iterator(m_source, m_edges, true);
}

template<IsEdge Edge>
ForwardEdgeIterator<Edge>::const_iterator ForwardEdgeIterator<Edge>::end() const
{
    return const_iterator(m_source, m_edges, false);
}

/* BackwardEdgeIterator */

template<IsEdge Edge>
BackwardEdgeIterator<Edge>::BackwardEdgeIterator(VertexIndex target, std::span<const Edge> edges) : m_target(target), m_edges(edges)
{
}

template<IsEdge Edge>
void BackwardEdgeIterator<Edge>::const_iterator::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;
    } while (m_pos < m_edges.size() && m_edges[m_pos].get_target() != m_target);
}

template<IsEdge Edge>
BackwardEdgeIterator<Edge>::const_iterator::const_iterator()
{
}

template<IsEdge Edge>
BackwardEdgeIterator<Edge>::const_iterator::const_iterator(VertexIndex target, std::span<const Edge> edges, bool begin) :
    m_target(target),
    m_pos(begin ? 0 : edges.size()),
    m_edges(edges)
{
    if (begin && m_edges.size() > 0 && m_edges[0].get_target() != m_target)
    {
        advance();
    }
}

template<IsEdge Edge>
BackwardEdgeIterator<Edge>::const_iterator::reference BackwardEdgeIterator<Edge>::const_iterator::operator*() const
{
    assert(m_pos < m_edges.size());
    return m_edges[m_pos];
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
    return const_iterator(m_target, m_edges, true);
}

template<IsEdge Edge>
BackwardEdgeIterator<Edge>::const_iterator BackwardEdgeIterator<Edge>::end() const
{
    return const_iterator(m_target, m_edges, false);
}

}

#endif
