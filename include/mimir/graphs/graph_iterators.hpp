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

#include "mimir/common/concepts.hpp"
#include "mimir/graphs/graph_edge_interface.hpp"
#include "mimir/graphs/graph_traversal_interface.hpp"
#include "mimir/graphs/graph_vertex_interface.hpp"

#include <cassert>

namespace mimir
{

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
class AdjacentVertexIterator
{
private:
    VertexIndex m_vertex;
    std::reference_wrapper<const std::vector<Vertex>> m_vertices;
    std::reference_wrapper<const std::vector<Edge>> m_edges;
    std::span<const EdgeIndex> m_slice;

public:
    AdjacentVertexIterator(VertexIndex vertex, const std::vector<Vertex>& vertices, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice);

    template<IsTraversalDirection Direction_>
    class const_iterator
    {
    private:
        size_t m_pos;
        VertexIndex m_vertex;
        const std::vector<Vertex>* m_vertices;
        const std::vector<Edge>* m_edges;
        std::span<const EdgeIndex> m_slice;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = const Vertex;
        using pointer = value_type*;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(VertexIndex vertex, const std::vector<Vertex>& vertices, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool begin);
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
class AdjacentVertexIndexIterator
{
private:
    VertexIndex m_vertex;
    std::reference_wrapper<const std::vector<Edge>> m_edges;
    std::span<const EdgeIndex> m_slice;

public:
    AdjacentVertexIndexIterator(VertexIndex vertex, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice);

    template<IsTraversalDirection Direction_>
    class const_iterator
    {
    private:
        size_t m_pos;
        VertexIndex m_vertex;
        const std::vector<Edge>* m_edges;
        std::span<const EdgeIndex> m_slice;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = VertexIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(VertexIndex vertex, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool begin);
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
class AdjacentEdgeIterator
{
private:
    VertexIndex m_vertex;
    std::reference_wrapper<const std::vector<Edge>> m_edges;
    std::span<const EdgeIndex> m_slice;

public:
    AdjacentEdgeIterator(VertexIndex vertex, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice);

    template<IsTraversalDirection Direction_>
    class const_iterator
    {
    private:
        size_t m_pos;
        VertexIndex m_vertex;
        const std::vector<Edge>* m_edges;
        std::span<const EdgeIndex> m_slice;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = Edge;
        using pointer = value_type*;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(VertexIndex vertex, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool begin);
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
class AdjacentEdgeIndexIterator
{
private:
    VertexIndex m_vertex;
    std::reference_wrapper<const std::vector<Edge>> m_edges;
    std::span<const EdgeIndex> m_slice;

public:
    AdjacentEdgeIndexIterator(VertexIndex vertex, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice);

    template<IsTraversalDirection Direction_>
    class const_iterator
    {
    private:
        size_t m_pos;
        VertexIndex m_vertex;
        const std::vector<Edge>* m_edges;
        std::span<const EdgeIndex> m_slice;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = EdgeIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(VertexIndex vertex, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool begin);
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

/* VertexIterator */

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
AdjacentVertexIterator<Vertex, Edge, Direction>::AdjacentVertexIterator(VertexIndex vertex,
                                                                        const std::vector<Vertex>& vertices,
                                                                        const std::vector<Edge>& edges,
                                                                        std::span<const EdgeIndex> slice) :
    m_vertex(vertex),
    m_vertices(vertices),
    m_edges(edges),
    m_slice(slice)
{
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
void AdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;

        if (m_pos == m_slice.size())
        {
            break;
        }

        if constexpr (std::is_same_v<Direction_, ForwardTraversal>)
        {
            if (m_edges->at(m_slice[m_pos]).get_source() == m_vertex)
            {
                break;
            }
        }
        else if constexpr (std::is_same_v<Direction_, BackwardTraversal>)
        {
            if (m_edges->at(m_slice[m_pos]).get_target() == m_vertex)
            {
                break;
            }
        }
        else
        {
            static_assert(dependent_false<Direction_>::value,
                          "VertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>::advance(): Missing implementation for IsTraversalDirection.");
        }
    } while (true);
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
AdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>::const_iterator() :
    m_pos(-1),
    m_vertex(-1),
    m_vertices(nullptr),
    m_edges(nullptr),
    m_slice()
{
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
AdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>::const_iterator(VertexIndex vertex,
                                                                                            const std::vector<Vertex>& vertices,
                                                                                            const std::vector<Edge>& edges,
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

        if constexpr (std::is_same_v<Direction_, ForwardTraversal>)
        {
            if (m_edges->at(m_slice[0]).get_source() == m_vertex)
            {
                return;
            }
        }
        else if constexpr (std::is_same_v<Direction_, BackwardTraversal>)
        {
            if (m_edges->at(m_slice[0]).get_target() == m_vertex)
            {
                return;
            }
        }
        else
        {
            static_assert(
                dependent_false<Direction_>::value,
                "VertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>::const_iterator(...): Missing implementation for IsTraversalDirection.");
        }

        advance();
    }
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
AdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>::reference
AdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>::operator*() const
{
    assert(m_vertices);
    assert(m_edges);
    assert(m_pos < m_slice.size());
    assert(m_slice[m_pos] < m_edges->size());

    if constexpr (std::is_same_v<Direction_, ForwardTraversal>)
    {
        assert(m_edges->at(m_slice[m_pos]).get_source() == m_vertex);
        return m_vertices->at(m_edges->at(m_slice[m_pos]).get_target());
    }
    else if constexpr (std::is_same_v<Direction_, BackwardTraversal>)
    {
        assert(m_edges->at(m_slice[m_pos]).get_target() == m_vertex);
        return m_vertices->at(m_edges->at(m_slice[m_pos]).get_source());
    }
    else
    {
        static_assert(dependent_false<Direction_>::value,
                      "VertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>::operator*(): Missing implementation for IsTraversalDirection.");
    }
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
AdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>&
AdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>::operator++()
{
    advance();
    return *this;
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
AdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>
AdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
bool AdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
bool AdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction_>::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
AdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction> AdjacentVertexIterator<Vertex, Edge, Direction>::begin() const
{
    return const_iterator<Direction>(m_vertex, m_vertices, m_edges, m_slice, true);
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
AdjacentVertexIterator<Vertex, Edge, Direction>::const_iterator<Direction> AdjacentVertexIterator<Vertex, Edge, Direction>::end() const
{
    return const_iterator<Direction>(m_vertex, m_vertices, m_edges, m_slice, false);
}

/* AdjacentVertexIndexIterator */

template<IsEdge Edge, IsTraversalDirection Direction>
AdjacentVertexIndexIterator<Edge, Direction>::AdjacentVertexIndexIterator(VertexIndex vertex,
                                                                          const std::vector<Edge>& edges,
                                                                          std::span<const EdgeIndex> slice) :
    m_vertex(vertex),
    m_edges(edges),
    m_slice(slice)
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
void AdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;

        if (m_pos == m_slice.size())
        {
            break;
        }

        if constexpr (std::is_same_v<Direction_, ForwardTraversal>)
        {
            if (m_edges->at(m_slice[m_pos]).get_source() == m_vertex)
            {
                break;
            }
        }
        else if constexpr (std::is_same_v<Direction_, BackwardTraversal>)
        {
            if (m_edges->at(m_slice[m_pos]).get_target() == m_vertex)
            {
                break;
            }
        }
        else
        {
            static_assert(
                dependent_false<Direction_>::value,
                "AdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>::advance(): Missing implementation for IsTraversalDirection.");
        }
    } while (true);
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
AdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>::const_iterator() : m_pos(-1), m_vertex(-1), m_edges(nullptr), m_slice()
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
AdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>::const_iterator(VertexIndex vertex,
                                                                                         const std::vector<Edge>& edges,
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

        if constexpr (std::is_same_v<Direction_, ForwardTraversal>)
        {
            if (m_edges->at(m_slice[0]).get_source() == m_vertex)
            {
                return;
            }
        }
        else if constexpr (std::is_same_v<Direction_, BackwardTraversal>)
        {
            if (m_edges->at(m_slice[0]).get_target() == m_vertex)
            {
                return;
            }
        }
        else
        {
            static_assert(dependent_false<Direction_>::value,
                          "AdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>::const_iterator(...): Missing implementation for "
                          "IsTraversalDirection.");
        }

        advance();
    }
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
AdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>::value_type
AdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>::operator*() const
{
    assert(m_edges);
    assert(m_pos < m_slice.size());
    assert(m_slice[m_pos] < m_edges->size());

    if constexpr (std::is_same_v<Direction_, ForwardTraversal>)
    {
        assert(m_edges->at(m_slice[m_pos]).get_source() == m_vertex);
        return m_edges->at(m_slice[m_pos]).get_target();
    }
    else if constexpr (std::is_same_v<Direction_, BackwardTraversal>)
    {
        assert(m_edges->at(m_slice[m_pos]).get_target() == m_vertex);
        return m_edges->at(m_slice[m_pos]).get_source();
    }
    else
    {
        static_assert(
            dependent_false<Direction_>::value,
            "AdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>::operator*(): Missing implementation for IsTraversalDirection.");
    }
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
AdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>& AdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
AdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>
AdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
bool AdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
bool AdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction_>::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsEdge Edge, IsTraversalDirection Direction>
AdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction> AdjacentVertexIndexIterator<Edge, Direction>::begin() const
{
    return const_iterator<Direction>(m_vertex, m_edges, m_slice, true);
}

template<IsEdge Edge, IsTraversalDirection Direction>
AdjacentVertexIndexIterator<Edge, Direction>::const_iterator<Direction> AdjacentVertexIndexIterator<Edge, Direction>::end() const
{
    return const_iterator<Direction>(m_vertex, m_edges, m_slice, false);
}

/* EdgeIterator */

template<IsEdge Edge, IsTraversalDirection Direction>
AdjacentEdgeIterator<Edge, Direction>::AdjacentEdgeIterator(VertexIndex vertex, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice) :
    m_vertex(vertex),
    m_edges(edges),
    m_slice(slice)
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
void AdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_>::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;

        if (m_pos == m_slice.size())
        {
            break;
        }

        if constexpr (std::is_same_v<Direction_, ForwardTraversal>)
        {
            if (m_edges->at(m_slice[m_pos]).get_source() == m_vertex)
            {
                break;
            }
        }
        else if constexpr (std::is_same_v<Direction_, BackwardTraversal>)
        {
            if (m_edges->at(m_slice[m_pos]).get_target() == m_vertex)
            {
                break;
            }
        }
        else
        {
            static_assert(dependent_false<Direction_>::value,
                          "EdgeIterator<Edge, Direction>::const_iterator<Direction_>::advance(): Missing implementation for IsTraversalDirection.");
        }
    } while (true);
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
AdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_>::const_iterator() : m_pos(-1), m_vertex(-1), m_edges(nullptr), m_slice()
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
AdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_>::const_iterator(VertexIndex vertex,
                                                                                  const std::vector<Edge>& edges,
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

        if constexpr (std::is_same_v<Direction_, ForwardTraversal>)
        {
            if (m_edges->at(m_slice[0]).get_source() == m_vertex)
            {
                return;
            }
        }
        else if constexpr (std::is_same_v<Direction_, BackwardTraversal>)
        {
            if (m_edges->at(m_slice[0]).get_target() == m_vertex)
            {
                return;
            }
        }
        else
        {
            static_assert(dependent_false<Direction_>::value,
                          "EdgeIterator<Edge, Direction>::const_iterator<Direction_>::const_iterator(...): Missing implementation for "
                          "IsTraversalDirection.");
        }

        advance();
    }
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
AdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_>::reference
AdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_>::operator*() const
{
    assert(m_edges);
    assert(m_pos < m_slice.size());
    assert(m_slice[m_pos] < m_edges->size());

    if constexpr (std::is_same_v<Direction_, ForwardTraversal>)
    {
        assert(m_edges->at(m_slice[m_pos]).get_source() == m_vertex);
        return m_edges->at(m_slice[m_pos]);
    }
    else if constexpr (std::is_same_v<Direction_, BackwardTraversal>)
    {
        assert(m_edges->at(m_slice[m_pos]).get_target() == m_vertex);
        return m_edges->at(m_slice[m_pos]);
    }
    else
    {
        static_assert(dependent_false<Direction_>::value,
                      "EdgeIterator<Edge, Direction>::const_iterator<Direction_>::operator*(): Missing implementation for IsTraversalDirection.");
    }
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
AdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_>& AdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_>::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
AdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_> AdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_>::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
bool AdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_>::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
bool AdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_>::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsEdge Edge, IsTraversalDirection Direction>
AdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction> AdjacentEdgeIterator<Edge, Direction>::begin() const
{
    return const_iterator<Direction>(m_vertex, m_edges, m_slice, true);
}

template<IsEdge Edge, IsTraversalDirection Direction>
AdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction> AdjacentEdgeIterator<Edge, Direction>::end() const
{
    return const_iterator<Direction>(m_vertex, m_edges, m_slice, false);
}

/* AdjacentEdgeIndexIterator */

template<IsEdge Edge, IsTraversalDirection Direction>
AdjacentEdgeIndexIterator<Edge, Direction>::AdjacentEdgeIndexIterator(VertexIndex vertex, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice) :
    m_vertex(vertex),
    m_edges(edges),
    m_slice(slice)
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
void AdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_>::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;

        if (m_pos == m_slice.size())
        {
            break;
        }

        if constexpr (std::is_same_v<Direction_, ForwardTraversal>)
        {
            if (m_edges->at(m_slice[m_pos]).get_source() == m_vertex)
            {
                break;
            }
        }
        else if constexpr (std::is_same_v<Direction_, BackwardTraversal>)
        {
            if (m_edges->at(m_slice[m_pos]).get_target() == m_vertex)
            {
                break;
            }
        }
        else
        {
            static_assert(
                dependent_false<Direction_>::value,
                "AdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_>::advance(): Missing implementation for IsTraversalDirection.");
        }
    } while (true);
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
AdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_>::const_iterator() : m_pos(-1), m_vertex(-1), m_edges(nullptr), m_slice()
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
AdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_>::const_iterator(VertexIndex vertex,
                                                                                       const std::vector<Edge>& edges,
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

        if constexpr (std::is_same_v<Direction_, ForwardTraversal>)
        {
            if (m_edges->at(m_slice[0]).get_source() == m_vertex)
            {
                return;
            }
        }
        else if constexpr (std::is_same_v<Direction_, BackwardTraversal>)
        {
            if (m_edges->at(m_slice[0]).get_target() == m_vertex)
            {
                return;
            }
        }
        else
        {
            static_assert(dependent_false<Direction_>::value,
                          "AdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_>::const_iterator(...): Missing implementation for "
                          "IsTraversalDirection.");
        }

        advance();
    }
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
AdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_>::value_type
AdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_>::operator*() const
{
    assert(m_edges);
    assert(m_pos < m_slice.size());
    assert(m_slice[m_pos] < m_edges->size());

    if constexpr (std::is_same_v<Direction_, ForwardTraversal>)
    {
        assert(m_edges->at(m_slice[m_pos]).get_source() == m_vertex);
        return m_edges->at(m_slice[m_pos]).get_index();
    }
    else if constexpr (std::is_same_v<Direction_, BackwardTraversal>)
    {
        assert(m_edges->at(m_slice[m_pos]).get_target() == m_vertex);
        return m_edges->at(m_slice[m_pos]).get_index();
    }
    else
    {
        static_assert(dependent_false<Direction_>::value,
                      "AdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_>::operator*(): Missing implementation for IsTraversalDirection.");
    }
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
AdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_>& AdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_>::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
AdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_> AdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_>::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
bool AdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_>::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge, IsTraversalDirection Direction>
template<IsTraversalDirection Direction_>
bool AdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction_>::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsEdge Edge, IsTraversalDirection Direction>
AdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction> AdjacentEdgeIndexIterator<Edge, Direction>::begin() const
{
    return const_iterator<Direction>(m_vertex, m_edges, m_slice, true);
}

template<IsEdge Edge, IsTraversalDirection Direction>
AdjacentEdgeIndexIterator<Edge, Direction>::const_iterator<Direction> AdjacentEdgeIndexIterator<Edge, Direction>::end() const
{
    return const_iterator<Direction>(m_vertex, m_edges, m_slice, false);
}

}

#endif
