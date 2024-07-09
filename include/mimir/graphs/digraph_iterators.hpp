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

#ifndef MIMIR_GRAPHS_DIGRAPH_ITERATORS_HPP_
#define MIMIR_GRAPHS_DIGRAPH_ITERATORS_HPP_

#include "mimir/graphs/digraph_edge_interface.hpp"

#include <cassert>

namespace mimir
{

template<IsDigraphEdge Edge>
class DigraphTargetIndexIterator
{
private:
    DigraphVertexIndex m_source;
    std::span<const Edge> m_edges;

public:
    DigraphTargetIndexIterator(DigraphVertexIndex source, std::span<const Edge> edges);

    class const_iterator
    {
    private:
        DigraphVertexIndex m_source;
        size_t m_pos;
        std::span<const Edge> m_edges;

        void advance();

    public:
        using difference_type = int;
        using value_type = DigraphVertexIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(DigraphVertexIndex source, std::span<const Edge> edges, bool begin);
        [[nodiscard]] value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        [[nodiscard]] bool operator==(const const_iterator& other) const;
        [[nodiscard]] bool operator!=(const const_iterator& other) const;
    };

    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;
};

static_assert(std::ranges::forward_range<DigraphTargetIndexIterator<DigraphEdge>>);

template<IsDigraphEdge Edge>
class DigraphSourceIndexIterator
{
private:
    DigraphVertexIndex m_target;
    std::span<const Edge> m_edges;

public:
    DigraphSourceIndexIterator(DigraphVertexIndex target, std::span<const Edge> edges);

    class const_iterator
    {
    private:
        DigraphVertexIndex m_target;
        size_t m_pos;
        std::span<const Edge> m_edges;

        void advance();

    public:
        using difference_type = int;
        using value_type = DigraphVertexIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(DigraphVertexIndex target, std::span<const Edge> edges, bool begin);
        [[nodiscard]] value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        [[nodiscard]] bool operator==(const const_iterator& other) const;
        [[nodiscard]] bool operator!=(const const_iterator& other) const;
    };

    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;
};

static_assert(std::ranges::forward_range<DigraphSourceIndexIterator<DigraphEdge>>);

template<IsDigraphEdge Edge>
class DigraphForwardEdgeIndexIterator
{
private:
    DigraphVertexIndex m_source;
    std::span<const Edge> m_edges;

public:
    DigraphForwardEdgeIndexIterator(DigraphVertexIndex source, std::span<const Edge> edges);

    class const_iterator
    {
    private:
        DigraphVertexIndex m_source;
        size_t m_pos;
        std::span<const Edge> m_edges;

        void advance();

    public:
        using difference_type = int;
        using value_type = DigraphEdgeIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(DigraphVertexIndex source, std::span<const Edge> edges, bool begin);
        [[nodiscard]] value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        [[nodiscard]] bool operator==(const const_iterator& other) const;
        [[nodiscard]] bool operator!=(const const_iterator& other) const;
    };

    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;
};

static_assert(std::ranges::forward_range<DigraphForwardEdgeIndexIterator<DigraphEdge>>);

template<IsDigraphEdge Edge>
class DigraphBackwardEdgeIndexIterator
{
private:
    DigraphVertexIndex m_target;
    std::span<const Edge> m_edges;

public:
    DigraphBackwardEdgeIndexIterator(DigraphVertexIndex target, std::span<const Edge> edges);

    class const_iterator
    {
    private:
        DigraphVertexIndex m_target;
        size_t m_pos;
        std::span<const Edge> m_edges;

        void advance();

    public:
        using difference_type = int;
        using value_type = DigraphEdgeIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(DigraphVertexIndex target, std::span<const Edge> edges, bool begin);
        [[nodiscard]] value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        [[nodiscard]] bool operator==(const const_iterator& other) const;
        [[nodiscard]] bool operator!=(const const_iterator& other) const;
    };

    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;
};

static_assert(std::ranges::forward_range<DigraphBackwardEdgeIndexIterator<DigraphEdge>>);

template<IsDigraphEdge Edge>
class DigraphForwardEdgeIterator
{
private:
    DigraphVertexIndex m_source;
    std::span<const Edge> m_edges;

public:
    DigraphForwardEdgeIterator(DigraphVertexIndex source, std::span<const Edge> edges);

    class const_iterator
    {
    private:
        DigraphVertexIndex m_source;
        size_t m_pos;
        std::span<const Edge> m_edges;

        void advance();

    public:
        using difference_type = int;
        using value_type = const DigraphEdge;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(DigraphVertexIndex source, std::span<const Edge> edges, bool begin);
        [[nodiscard]] reference operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        [[nodiscard]] bool operator==(const const_iterator& other) const;
        [[nodiscard]] bool operator!=(const const_iterator& other) const;
    };

    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;
};

static_assert(std::ranges::forward_range<DigraphForwardEdgeIterator<DigraphEdge>>);

template<IsDigraphEdge Edge>
class DigraphBackwardEdgeIterator
{
private:
    DigraphVertexIndex m_target;
    std::span<const Edge> m_edges;

public:
    DigraphBackwardEdgeIterator(DigraphVertexIndex target, std::span<const Edge> edges);

    class const_iterator
    {
    private:
        DigraphVertexIndex m_target;
        size_t m_pos;
        std::span<const Edge> m_edges;

        void advance();

    public:
        using difference_type = int;
        using value_type = const DigraphEdge;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(DigraphVertexIndex target, std::span<const Edge> edges, bool begin);
        [[nodiscard]] reference operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        [[nodiscard]] bool operator==(const const_iterator& other) const;
        [[nodiscard]] bool operator!=(const const_iterator& other) const;
    };

    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;
};

static_assert(std::ranges::forward_range<DigraphBackwardEdgeIterator<DigraphEdge>>);

/**
 * Implementations
 */

/* DigraphTargetIndexIterator */

template<IsDigraphEdge Edge>
DigraphTargetIndexIterator<Edge>::DigraphTargetIndexIterator(DigraphVertexIndex source, std::span<const Edge> edges) : m_source(source), m_edges(edges)
{
}

template<IsDigraphEdge Edge>
void DigraphTargetIndexIterator<Edge>::const_iterator::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;
    } while (m_pos < m_edges.size() && m_edges[m_pos].get_source() != m_source);
}

template<IsDigraphEdge Edge>
DigraphTargetIndexIterator<Edge>::const_iterator::const_iterator()
{
}

template<IsDigraphEdge Edge>
DigraphTargetIndexIterator<Edge>::const_iterator::const_iterator(DigraphVertexIndex source, std::span<const Edge> edges, bool begin) :
    m_source(source),
    m_pos(begin ? 0 : edges.size()),
    m_edges(edges)
{
    if (begin && m_edges.size() > 0 && m_edges[0].get_source() != m_source)
    {
        advance();
    }
}

template<IsDigraphEdge Edge>
DigraphTargetIndexIterator<Edge>::const_iterator::value_type DigraphTargetIndexIterator<Edge>::const_iterator::operator*() const
{
    assert(m_pos < m_edges.size());
    return m_edges[m_pos].get_target();
}

template<IsDigraphEdge Edge>
DigraphTargetIndexIterator<Edge>::const_iterator& DigraphTargetIndexIterator<Edge>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsDigraphEdge Edge>
DigraphTargetIndexIterator<Edge>::const_iterator DigraphTargetIndexIterator<Edge>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsDigraphEdge Edge>
bool DigraphTargetIndexIterator<Edge>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsDigraphEdge Edge>
bool DigraphTargetIndexIterator<Edge>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsDigraphEdge Edge>
DigraphTargetIndexIterator<Edge>::const_iterator DigraphTargetIndexIterator<Edge>::begin() const
{
    return const_iterator(m_source, m_edges, true);
}

template<IsDigraphEdge Edge>
DigraphTargetIndexIterator<Edge>::const_iterator DigraphTargetIndexIterator<Edge>::end() const
{
    return const_iterator(m_source, m_edges, false);
}

/* SourceStateIndexIterator */

template<IsDigraphEdge Edge>
DigraphSourceIndexIterator<Edge>::DigraphSourceIndexIterator(DigraphVertexIndex target, std::span<const Edge> edges) : m_target(target), m_edges(edges)
{
}

template<IsDigraphEdge Edge>
void DigraphSourceIndexIterator<Edge>::const_iterator::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;
    } while (m_pos < m_edges.size() && m_edges[m_pos].get_target() != m_target);
}

template<IsDigraphEdge Edge>
DigraphSourceIndexIterator<Edge>::const_iterator::const_iterator()
{
}

template<IsDigraphEdge Edge>
DigraphSourceIndexIterator<Edge>::const_iterator::const_iterator(DigraphVertexIndex target, std::span<const Edge> edges, bool begin) :
    m_target(target),
    m_pos(begin ? 0 : edges.size()),
    m_edges(edges)
{
    if (begin && m_edges.size() > 0 && m_edges[0].get_target() != m_target)
    {
        advance();
    }
}

template<IsDigraphEdge Edge>
DigraphSourceIndexIterator<Edge>::const_iterator::value_type DigraphSourceIndexIterator<Edge>::const_iterator::operator*() const
{
    assert(m_pos < m_edges.size());
    return m_edges[m_pos].get_source();
}

template<IsDigraphEdge Edge>
DigraphSourceIndexIterator<Edge>::const_iterator& DigraphSourceIndexIterator<Edge>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsDigraphEdge Edge>
DigraphSourceIndexIterator<Edge>::const_iterator DigraphSourceIndexIterator<Edge>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsDigraphEdge Edge>
bool DigraphSourceIndexIterator<Edge>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsDigraphEdge Edge>
bool DigraphSourceIndexIterator<Edge>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsDigraphEdge Edge>
DigraphSourceIndexIterator<Edge>::const_iterator DigraphSourceIndexIterator<Edge>::begin() const
{
    return const_iterator(m_target, m_edges, true);
}

template<IsDigraphEdge Edge>
DigraphSourceIndexIterator<Edge>::const_iterator DigraphSourceIndexIterator<Edge>::end() const
{
    return const_iterator(m_target, m_edges, false);
}

/* DigraphForwardEdgeIndexIterator */

template<IsDigraphEdge Edge>
DigraphForwardEdgeIndexIterator<Edge>::DigraphForwardEdgeIndexIterator(DigraphVertexIndex source, std::span<const Edge> edges) :
    m_source(source),
    m_edges(edges)
{
}

template<IsDigraphEdge Edge>
void DigraphForwardEdgeIndexIterator<Edge>::const_iterator::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;
    } while (m_pos < m_edges.size() && m_edges[m_pos].get_source() != m_source);
}

template<IsDigraphEdge Edge>
DigraphForwardEdgeIndexIterator<Edge>::const_iterator::const_iterator()
{
}

template<IsDigraphEdge Edge>
DigraphForwardEdgeIndexIterator<Edge>::const_iterator::const_iterator(DigraphVertexIndex source, std::span<const Edge> edges, bool begin) :
    m_source(source),
    m_pos(begin ? 0 : edges.size()),
    m_edges(edges)
{
    if (begin && m_edges.size() > 0 && m_edges[0].get_source() != m_source)
    {
        advance();
    }
}

template<IsDigraphEdge Edge>
DigraphForwardEdgeIndexIterator<Edge>::const_iterator::value_type DigraphForwardEdgeIndexIterator<Edge>::const_iterator::operator*() const
{
    assert(m_pos < m_edges.size());
    return m_edges[m_pos].get_index();
}

template<IsDigraphEdge Edge>
DigraphForwardEdgeIndexIterator<Edge>::const_iterator& DigraphForwardEdgeIndexIterator<Edge>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsDigraphEdge Edge>
DigraphForwardEdgeIndexIterator<Edge>::const_iterator DigraphForwardEdgeIndexIterator<Edge>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsDigraphEdge Edge>
bool DigraphForwardEdgeIndexIterator<Edge>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsDigraphEdge Edge>
bool DigraphForwardEdgeIndexIterator<Edge>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsDigraphEdge Edge>
DigraphForwardEdgeIndexIterator<Edge>::const_iterator DigraphForwardEdgeIndexIterator<Edge>::begin() const
{
    return const_iterator(m_source, m_edges, true);
}

template<IsDigraphEdge Edge>
DigraphForwardEdgeIndexIterator<Edge>::const_iterator DigraphForwardEdgeIndexIterator<Edge>::end() const
{
    return const_iterator(m_source, m_edges, false);
}

/* DigraphBackwardEdgeIndexIterator */

template<IsDigraphEdge Edge>
DigraphBackwardEdgeIndexIterator<Edge>::DigraphBackwardEdgeIndexIterator(DigraphVertexIndex target, std::span<const Edge> edges) :
    m_target(target),
    m_edges(edges)
{
}

template<IsDigraphEdge Edge>
void DigraphBackwardEdgeIndexIterator<Edge>::const_iterator::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;
    } while (m_pos < m_edges.size() && m_edges[m_pos].get_target() != m_target);
}

template<IsDigraphEdge Edge>
DigraphBackwardEdgeIndexIterator<Edge>::const_iterator::const_iterator()
{
}

template<IsDigraphEdge Edge>
DigraphBackwardEdgeIndexIterator<Edge>::const_iterator::const_iterator(DigraphVertexIndex target, std::span<const Edge> edges, bool begin) :
    m_target(target),
    m_pos(begin ? 0 : edges.size()),
    m_edges(edges)
{
    if (begin && m_edges.size() > 0 && m_edges[0].get_target() != m_target)
    {
        advance();
    }
}

template<IsDigraphEdge Edge>
DigraphBackwardEdgeIndexIterator<Edge>::const_iterator::value_type DigraphBackwardEdgeIndexIterator<Edge>::const_iterator::operator*() const
{
    assert(m_pos < m_edges.size());
    return m_edges[m_pos].get_index();
}

template<IsDigraphEdge Edge>
DigraphBackwardEdgeIndexIterator<Edge>::const_iterator& DigraphBackwardEdgeIndexIterator<Edge>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsDigraphEdge Edge>
DigraphBackwardEdgeIndexIterator<Edge>::const_iterator DigraphBackwardEdgeIndexIterator<Edge>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsDigraphEdge Edge>
bool DigraphBackwardEdgeIndexIterator<Edge>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsDigraphEdge Edge>
bool DigraphBackwardEdgeIndexIterator<Edge>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsDigraphEdge Edge>
DigraphBackwardEdgeIndexIterator<Edge>::const_iterator DigraphBackwardEdgeIndexIterator<Edge>::begin() const
{
    return const_iterator(m_target, m_edges, true);
}

template<IsDigraphEdge Edge>
DigraphBackwardEdgeIndexIterator<Edge>::const_iterator DigraphBackwardEdgeIndexIterator<Edge>::end() const
{
    return const_iterator(m_target, m_edges, false);
}

/* DigraphForwardEdgeIterator */

template<IsDigraphEdge Edge>
DigraphForwardEdgeIterator<Edge>::DigraphForwardEdgeIterator(DigraphVertexIndex source, std::span<const Edge> edges) : m_source(source), m_edges(edges)
{
}

template<IsDigraphEdge Edge>
void DigraphForwardEdgeIterator<Edge>::const_iterator::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;
    } while (m_pos < m_edges.size() && m_edges[m_pos].get_source() != m_source);
}

template<IsDigraphEdge Edge>
DigraphForwardEdgeIterator<Edge>::const_iterator::const_iterator()
{
}

template<IsDigraphEdge Edge>
DigraphForwardEdgeIterator<Edge>::const_iterator::const_iterator(DigraphVertexIndex source, std::span<const Edge> edges, bool begin) :
    m_source(source),
    m_pos(begin ? 0 : edges.size()),
    m_edges(edges)
{
    if (begin && m_edges.size() > 0 && m_edges[0].get_source() != m_source)
    {
        advance();
    }
}

template<IsDigraphEdge Edge>
DigraphForwardEdgeIterator<Edge>::const_iterator::reference DigraphForwardEdgeIterator<Edge>::const_iterator::operator*() const
{
    assert(m_pos < m_edges.size());
    return m_edges[m_pos];
}

template<IsDigraphEdge Edge>
DigraphForwardEdgeIterator<Edge>::const_iterator& DigraphForwardEdgeIterator<Edge>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsDigraphEdge Edge>
DigraphForwardEdgeIterator<Edge>::const_iterator DigraphForwardEdgeIterator<Edge>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsDigraphEdge Edge>
bool DigraphForwardEdgeIterator<Edge>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsDigraphEdge Edge>
bool DigraphForwardEdgeIterator<Edge>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsDigraphEdge Edge>
DigraphForwardEdgeIterator<Edge>::const_iterator DigraphForwardEdgeIterator<Edge>::begin() const
{
    return const_iterator(m_source, m_edges, true);
}

template<IsDigraphEdge Edge>
DigraphForwardEdgeIterator<Edge>::const_iterator DigraphForwardEdgeIterator<Edge>::end() const
{
    return const_iterator(m_source, m_edges, false);
}

/* DigraphBackwardEdgeIterator */

template<IsDigraphEdge Edge>
DigraphBackwardEdgeIterator<Edge>::DigraphBackwardEdgeIterator(DigraphVertexIndex target, std::span<const Edge> edges) : m_target(target), m_edges(edges)
{
}

template<IsDigraphEdge Edge>
void DigraphBackwardEdgeIterator<Edge>::const_iterator::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;
    } while (m_pos < m_edges.size() && m_edges[m_pos].get_target() != m_target);
}

template<IsDigraphEdge Edge>
DigraphBackwardEdgeIterator<Edge>::const_iterator::const_iterator()
{
}

template<IsDigraphEdge Edge>
DigraphBackwardEdgeIterator<Edge>::const_iterator::const_iterator(DigraphVertexIndex target, std::span<const Edge> edges, bool begin) :
    m_target(target),
    m_pos(begin ? 0 : edges.size()),
    m_edges(edges)
{
    if (begin && m_edges.size() > 0 && m_edges[0].get_target() != m_target)
    {
        advance();
    }
}

template<IsDigraphEdge Edge>
DigraphBackwardEdgeIterator<Edge>::const_iterator::reference DigraphBackwardEdgeIterator<Edge>::const_iterator::operator*() const
{
    assert(m_pos < m_edges.size());
    return m_edges[m_pos];
}

template<IsDigraphEdge Edge>
DigraphBackwardEdgeIterator<Edge>::const_iterator& DigraphBackwardEdgeIterator<Edge>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsDigraphEdge Edge>
DigraphBackwardEdgeIterator<Edge>::const_iterator DigraphBackwardEdgeIterator<Edge>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsDigraphEdge Edge>
bool DigraphBackwardEdgeIterator<Edge>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsDigraphEdge Edge>
bool DigraphBackwardEdgeIterator<Edge>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsDigraphEdge Edge>
DigraphBackwardEdgeIterator<Edge>::const_iterator DigraphBackwardEdgeIterator<Edge>::begin() const
{
    return const_iterator(m_target, m_edges, true);
}

template<IsDigraphEdge Edge>
DigraphBackwardEdgeIterator<Edge>::const_iterator DigraphBackwardEdgeIterator<Edge>::end() const
{
    return const_iterator(m_target, m_edges, false);
}

}

#endif
