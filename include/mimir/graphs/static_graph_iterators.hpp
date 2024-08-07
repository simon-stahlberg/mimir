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

#ifndef MIMIR_GRAPHS_STATIC_GRAPH_ITERATORS_HPP_
#define MIMIR_GRAPHS_STATIC_GRAPH_ITERATORS_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/graphs/graph_edge_interface.hpp"
#include "mimir/graphs/graph_traversal_interface.hpp"
#include "mimir/graphs/graph_vertex_interface.hpp"

#include <cassert>
#include <span>
#include <vector>

namespace mimir
{

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
class StaticAdjacentVertexConstIterator
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
    using value_type = Vertex;
    using pointer = value_type*;
    using reference = const value_type&;
    using iterator_category = std::forward_iterator_tag;

    StaticAdjacentVertexConstIterator();
    StaticAdjacentVertexConstIterator(VertexIndex vertex,
                                      const std::vector<Vertex>& vertices,
                                      const std::vector<Edge>& edges,
                                      std::span<const EdgeIndex> slice,
                                      bool begin);
    reference operator*() const;
    StaticAdjacentVertexConstIterator& operator++();
    StaticAdjacentVertexConstIterator operator++(int);
    bool operator==(const StaticAdjacentVertexConstIterator& other) const;
    bool operator!=(const StaticAdjacentVertexConstIterator& other) const;
};

template<IsEdge Edge, IsTraversalDirection Direction>
class StaticAdjacentVertexIndexConstIterator
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

    StaticAdjacentVertexIndexConstIterator();
    StaticAdjacentVertexIndexConstIterator(VertexIndex vertex, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool begin);
    value_type operator*() const;
    StaticAdjacentVertexIndexConstIterator& operator++();
    StaticAdjacentVertexIndexConstIterator operator++(int);
    bool operator==(const StaticAdjacentVertexIndexConstIterator& other) const;
    bool operator!=(const StaticAdjacentVertexIndexConstIterator& other) const;
};

template<IsEdge Edge, IsTraversalDirection Direction>
class StaticAdjacentEdgeConstIterator
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

    StaticAdjacentEdgeConstIterator();
    StaticAdjacentEdgeConstIterator(VertexIndex vertex, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool begin);
    reference operator*() const;
    StaticAdjacentEdgeConstIterator& operator++();
    StaticAdjacentEdgeConstIterator operator++(int);
    bool operator==(const StaticAdjacentEdgeConstIterator& other) const;
    bool operator!=(const StaticAdjacentEdgeConstIterator& other) const;
};

template<IsEdge Edge, IsTraversalDirection Direction>
class StaticAdjacentEdgeIndexConstIterator
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

    StaticAdjacentEdgeIndexConstIterator();
    StaticAdjacentEdgeIndexConstIterator(VertexIndex vertex, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool begin);
    value_type operator*() const;
    StaticAdjacentEdgeIndexConstIterator& operator++();
    StaticAdjacentEdgeIndexConstIterator operator++(int);
    bool operator==(const StaticAdjacentEdgeIndexConstIterator& other) const;
    bool operator!=(const StaticAdjacentEdgeIndexConstIterator& other) const;
};

/**
 * Implementations
 */

/* StaticAdjacentVertexConstIterator */

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
void StaticAdjacentVertexConstIterator<Vertex, Edge, Direction>::StaticAdjacentVertexConstIterator::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;

        if (m_pos == m_slice.size())
        {
            break;
        }

        if constexpr (std::is_same_v<Direction, ForwardTraversal>)
        {
            if (m_edges->at(m_slice[m_pos]).get_source() == m_vertex)
            {
                break;
            }
        }
        else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
        {
            if (m_edges->at(m_slice[m_pos]).get_target() == m_vertex)
            {
                break;
            }
        }
        else
        {
            static_assert(dependent_false<Direction>::value,
                          "StaticAdjacentVertexConstIterator<Vertex, Edge, Direction>::advance(): Missing implementation for "
                          "IsTraversalDirection.");
        }
    } while (true);
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
StaticAdjacentVertexConstIterator<Vertex, Edge, Direction>::StaticAdjacentVertexConstIterator() :
    m_pos(-1),
    m_vertex(-1),
    m_vertices(nullptr),
    m_edges(nullptr),
    m_slice()
{
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
StaticAdjacentVertexConstIterator<Vertex, Edge, Direction>::StaticAdjacentVertexConstIterator(VertexIndex vertex,
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

        if constexpr (std::is_same_v<Direction, ForwardTraversal>)
        {
            if (m_edges->at(m_slice[0]).get_source() == m_vertex)
            {
                return;
            }
        }
        else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
        {
            if (m_edges->at(m_slice[0]).get_target() == m_vertex)
            {
                return;
            }
        }
        else
        {
            static_assert(dependent_false<Direction>::value,
                          "StaticAdjacentVertexConstIterator<Vertex, Edge, Direction>::StaticAdjacentVertexConstIterator(...): Missing implementation for "
                          "IsTraversalDirection.");
        }

        advance();
    }
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
StaticAdjacentVertexConstIterator<Vertex, Edge, Direction>::reference StaticAdjacentVertexConstIterator<Vertex, Edge, Direction>::operator*() const
{
    assert(m_vertices);
    assert(m_edges);
    assert(m_pos < m_slice.size());
    assert(m_slice[m_pos] < m_edges->size());

    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        assert(m_edges->at(m_slice[m_pos]).get_source() == m_vertex);
        return m_vertices->at(m_edges->at(m_slice[m_pos]).get_target());
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        assert(m_edges->at(m_slice[m_pos]).get_target() == m_vertex);
        return m_vertices->at(m_edges->at(m_slice[m_pos]).get_source());
    }
    else
    {
        static_assert(dependent_false<Direction>::value,
                      "StaticAdjacentVertexConstIterator<Vertex, Edge, Direction>::operator*(): Missing implementation for IsTraversalDirection.");
    }
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
StaticAdjacentVertexConstIterator<Vertex, Edge, Direction>& StaticAdjacentVertexConstIterator<Vertex, Edge, Direction>::operator++()
{
    advance();
    return *this;
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
StaticAdjacentVertexConstIterator<Vertex, Edge, Direction> StaticAdjacentVertexConstIterator<Vertex, Edge, Direction>::operator++(int)
{
    StaticAdjacentVertexConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
bool StaticAdjacentVertexConstIterator<Vertex, Edge, Direction>::operator==(const StaticAdjacentVertexConstIterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
bool StaticAdjacentVertexConstIterator<Vertex, Edge, Direction>::operator!=(const StaticAdjacentVertexConstIterator& other) const
{
    return !(*this == other);
}

/* StaticAdjacentVertexIndexConstIterator */

template<IsEdge Edge, IsTraversalDirection Direction>
void StaticAdjacentVertexIndexConstIterator<Edge, Direction>::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;

        if (m_pos == m_slice.size())
        {
            break;
        }

        if constexpr (std::is_same_v<Direction, ForwardTraversal>)
        {
            if (m_edges->at(m_slice[m_pos]).get_source() == m_vertex)
            {
                break;
            }
        }
        else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
        {
            if (m_edges->at(m_slice[m_pos]).get_target() == m_vertex)
            {
                break;
            }
        }
        else
        {
            static_assert(dependent_false<Direction>::value,
                          "StaticAdjacentVertexIndexConstIterator<Edge, Direction>::advance(): Missing implementation for IsTraversalDirection.");
        }
    } while (true);
}

template<IsEdge Edge, IsTraversalDirection Direction>
StaticAdjacentVertexIndexConstIterator<Edge, Direction>::StaticAdjacentVertexIndexConstIterator() : m_pos(-1), m_vertex(-1), m_edges(nullptr), m_slice()
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
StaticAdjacentVertexIndexConstIterator<Edge, Direction>::StaticAdjacentVertexIndexConstIterator(VertexIndex vertex,
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

        if constexpr (std::is_same_v<Direction, ForwardTraversal>)
        {
            if (m_edges->at(m_slice[0]).get_source() == m_vertex)
            {
                return;
            }
        }
        else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
        {
            if (m_edges->at(m_slice[0]).get_target() == m_vertex)
            {
                return;
            }
        }
        else
        {
            static_assert(dependent_false<Direction>::value,
                          "StaticAdjacentVertexIndexConstIterator<Edge, Direction>::StaticAdjacentEdgeConstIterator(...): Missing implementation for "
                          "IsTraversalDirection.");
        }

        advance();
    }
}

template<IsEdge Edge, IsTraversalDirection Direction>
StaticAdjacentVertexIndexConstIterator<Edge, Direction>::value_type StaticAdjacentVertexIndexConstIterator<Edge, Direction>::operator*() const
{
    assert(m_edges);
    assert(m_pos < m_slice.size());
    assert(m_slice[m_pos] < m_edges->size());

    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        assert(m_edges->at(m_slice[m_pos]).get_source() == m_vertex);
        return m_edges->at(m_slice[m_pos]).get_target();
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        assert(m_edges->at(m_slice[m_pos]).get_target() == m_vertex);
        return m_edges->at(m_slice[m_pos]).get_source();
    }
    else
    {
        static_assert(dependent_false<Direction>::value,
                      "StaticAdjacentVertexIndexConstIterator<Edge, Direction>::operator*(): Missing implementation for IsTraversalDirection.");
    }
}

template<IsEdge Edge, IsTraversalDirection Direction>
StaticAdjacentVertexIndexConstIterator<Edge, Direction>& StaticAdjacentVertexIndexConstIterator<Edge, Direction>::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge, IsTraversalDirection Direction>
StaticAdjacentVertexIndexConstIterator<Edge, Direction> StaticAdjacentVertexIndexConstIterator<Edge, Direction>::operator++(int)
{
    StaticAdjacentVertexIndexConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge, IsTraversalDirection Direction>
bool StaticAdjacentVertexIndexConstIterator<Edge, Direction>::operator==(const StaticAdjacentVertexIndexConstIterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge, IsTraversalDirection Direction>
bool StaticAdjacentVertexIndexConstIterator<Edge, Direction>::operator!=(const StaticAdjacentVertexIndexConstIterator& other) const
{
    return !(*this == other);
}

/* StaticAdjacentEdgeConstIterator */

template<IsEdge Edge, IsTraversalDirection Direction>
void StaticAdjacentEdgeConstIterator<Edge, Direction>::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;

        if (m_pos == m_slice.size())
        {
            break;
        }

        if constexpr (std::is_same_v<Direction, ForwardTraversal>)
        {
            if (m_edges->at(m_slice[m_pos]).get_source() == m_vertex)
            {
                break;
            }
        }
        else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
        {
            if (m_edges->at(m_slice[m_pos]).get_target() == m_vertex)
            {
                break;
            }
        }
        else
        {
            static_assert(dependent_false<Direction>::value,
                          "StaticAdjacentEdgeConstIterator<Edge, Direction>::advance(): Missing implementation for IsTraversalDirection.");
        }
    } while (true);
}

template<IsEdge Edge, IsTraversalDirection Direction>
StaticAdjacentEdgeConstIterator<Edge, Direction>::StaticAdjacentEdgeConstIterator() : m_pos(-1), m_vertex(-1), m_edges(nullptr), m_slice()
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
StaticAdjacentEdgeConstIterator<Edge, Direction>::StaticAdjacentEdgeConstIterator(VertexIndex vertex,
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

        if constexpr (std::is_same_v<Direction, ForwardTraversal>)
        {
            if (m_edges->at(m_slice[0]).get_source() == m_vertex)
            {
                return;
            }
        }
        else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
        {
            if (m_edges->at(m_slice[0]).get_target() == m_vertex)
            {
                return;
            }
        }
        else
        {
            static_assert(
                dependent_false<Direction>::value,
                "StaticAdjacentEdgeConstIterator<Edge, Direction>::StaticAdjacentEdgeConstIterator(...): Missing implementation for IsTraversalDirection.");
        }

        advance();
    }
}

template<IsEdge Edge, IsTraversalDirection Direction>
StaticAdjacentEdgeConstIterator<Edge, Direction>::reference StaticAdjacentEdgeConstIterator<Edge, Direction>::operator*() const
{
    assert(m_edges);
    assert(m_pos < m_slice.size());
    assert(m_slice[m_pos] < m_edges->size());

    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        assert(m_edges->at(m_slice[m_pos]).get_source() == m_vertex);
        return m_edges->at(m_slice[m_pos]);
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        assert(m_edges->at(m_slice[m_pos]).get_target() == m_vertex);
        return m_edges->at(m_slice[m_pos]);
    }
    else
    {
        static_assert(dependent_false<Direction>::value,
                      "StaticAdjacentEdgeConstIterator<Edge, Direction>::operator*(): Missing implementation for IsTraversalDirection.");
    }
}

template<IsEdge Edge, IsTraversalDirection Direction>
StaticAdjacentEdgeConstIterator<Edge, Direction>& StaticAdjacentEdgeConstIterator<Edge, Direction>::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge, IsTraversalDirection Direction>
StaticAdjacentEdgeConstIterator<Edge, Direction> StaticAdjacentEdgeConstIterator<Edge, Direction>::operator++(int)
{
    StaticAdjacentEdgeConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge, IsTraversalDirection Direction>
bool StaticAdjacentEdgeConstIterator<Edge, Direction>::operator==(const StaticAdjacentEdgeConstIterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge, IsTraversalDirection Direction>
bool StaticAdjacentEdgeConstIterator<Edge, Direction>::operator!=(const StaticAdjacentEdgeConstIterator& other) const
{
    return !(*this == other);
}

/* StaticAdjacentEdgeIndexConstIterator */

template<IsEdge Edge, IsTraversalDirection Direction>
void StaticAdjacentEdgeIndexConstIterator<Edge, Direction>::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;

        if (m_pos == m_slice.size())
        {
            break;
        }

        if constexpr (std::is_same_v<Direction, ForwardTraversal>)
        {
            if (m_edges->at(m_slice[m_pos]).get_source() == m_vertex)
            {
                break;
            }
        }
        else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
        {
            if (m_edges->at(m_slice[m_pos]).get_target() == m_vertex)
            {
                break;
            }
        }
        else
        {
            static_assert(dependent_false<Direction>::value,
                          "StaticAdjacentEdgeIndexConstIterator<Edge, Direction>::advance(): Missing implementation for IsTraversalDirection.");
        }
    } while (true);
}

template<IsEdge Edge, IsTraversalDirection Direction>
StaticAdjacentEdgeIndexConstIterator<Edge, Direction>::StaticAdjacentEdgeIndexConstIterator() : m_pos(-1), m_vertex(-1), m_edges(nullptr), m_slice()
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
StaticAdjacentEdgeIndexConstIterator<Edge, Direction>::StaticAdjacentEdgeIndexConstIterator(VertexIndex vertex,
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

        if constexpr (std::is_same_v<Direction, ForwardTraversal>)
        {
            if (m_edges->at(m_slice[0]).get_source() == m_vertex)
            {
                return;
            }
        }
        else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
        {
            if (m_edges->at(m_slice[0]).get_target() == m_vertex)
            {
                return;
            }
        }
        else
        {
            static_assert(dependent_false<Direction>::value,
                          "StaticAdjacentEdgeIndexConstIterator<Edge, Direction>::const_iterator(...): Missing implementation for IsTraversalDirection.");
        }

        advance();
    }
}

template<IsEdge Edge, IsTraversalDirection Direction>
StaticAdjacentEdgeIndexConstIterator<Edge, Direction>::value_type StaticAdjacentEdgeIndexConstIterator<Edge, Direction>::operator*() const
{
    assert(m_edges);
    assert(m_pos < m_slice.size());
    assert(m_slice[m_pos] < m_edges->size());

    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        assert(m_edges->at(m_slice[m_pos]).get_source() == m_vertex);
        return m_edges->at(m_slice[m_pos]).get_index();
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        assert(m_edges->at(m_slice[m_pos]).get_target() == m_vertex);
        return m_edges->at(m_slice[m_pos]).get_index();
    }
    else
    {
        static_assert(dependent_false<Direction>::value,
                      "StaticAdjacentEdgeIndexConstIterator<Edge, Direction>::operator*(): Missing implementation for IsTraversalDirection.");
    }
}

template<IsEdge Edge, IsTraversalDirection Direction>
StaticAdjacentEdgeIndexConstIterator<Edge, Direction>& StaticAdjacentEdgeIndexConstIterator<Edge, Direction>::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge, IsTraversalDirection Direction>
StaticAdjacentEdgeIndexConstIterator<Edge, Direction> StaticAdjacentEdgeIndexConstIterator<Edge, Direction>::operator++(int)
{
    StaticAdjacentEdgeIndexConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge, IsTraversalDirection Direction>
bool StaticAdjacentEdgeIndexConstIterator<Edge, Direction>::operator==(const StaticAdjacentEdgeIndexConstIterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge, IsTraversalDirection Direction>
bool StaticAdjacentEdgeIndexConstIterator<Edge, Direction>::operator!=(const StaticAdjacentEdgeIndexConstIterator& other) const
{
    return !(*this == other);
}

}

#endif
