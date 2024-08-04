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

template<IsVertex Vertex>
class VertexIndexIterator
{
private:
    std::reference_wrapper<const std::vector<Vertex>> m_vertices;

public:
    explicit VertexIndexIterator(const std::vector<Vertex>& vertices);

    class const_iterator
    {
    private:
        size_t m_pos;
        const std::vector<Vertex>* m_vertices;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = VertexIndex;
        using pointer = value_type*;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(const std::vector<Vertex>& vertices, bool begin);
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
class EdgeIndexIterator
{
private:
    std::reference_wrapper<const std::vector<Edge>> m_edges;

public:
    explicit EdgeIndexIterator(const std::vector<Edge>& edges);

    class const_iterator
    {
    private:
        size_t m_pos;
        const std::vector<Edge>* m_edges;

        void advance();

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = EdgeIndex;
        using pointer = value_type*;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(const std::vector<Edge>& edges, bool begin);
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
class AdjacentVertexConstIterator
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

    AdjacentVertexConstIterator();
    AdjacentVertexConstIterator(VertexIndex vertex,
                                const std::vector<Vertex>& vertices,
                                const std::vector<Edge>& edges,
                                std::span<const EdgeIndex> slice,
                                bool begin);
    reference operator*() const;
    AdjacentVertexConstIterator& operator++();
    AdjacentVertexConstIterator operator++(int);
    bool operator==(const AdjacentVertexConstIterator& other) const;
    bool operator!=(const AdjacentVertexConstIterator& other) const;
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

/* VertexIndexIterator */

template<IsVertex Vertex>
VertexIndexIterator<Vertex>::VertexIndexIterator(const std::vector<Vertex>& vertices) : m_vertices(vertices)
{
}

template<IsVertex Vertex>
void VertexIndexIterator<Vertex>::const_iterator::advance()
{
    ++m_pos;
}

template<IsVertex Vertex>
VertexIndexIterator<Vertex>::const_iterator::const_iterator() : m_pos(-1), m_vertices(nullptr)
{
}

template<IsVertex Vertex>
VertexIndexIterator<Vertex>::const_iterator::const_iterator(const std::vector<Vertex>& vertices, bool begin) :
    m_pos(begin ? 0 : vertices.size()),
    m_vertices(&vertices)
{
}

template<IsVertex Vertex>
VertexIndexIterator<Vertex>::const_iterator::value_type VertexIndexIterator<Vertex>::const_iterator::operator*() const
{
    assert(m_vertices);
    assert(m_pos < m_vertices->size());
    return m_vertices->at(m_pos).get_index();
}

template<IsVertex Vertex>
VertexIndexIterator<Vertex>::const_iterator& VertexIndexIterator<Vertex>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsVertex Vertex>
VertexIndexIterator<Vertex>::const_iterator VertexIndexIterator<Vertex>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsVertex Vertex>
bool VertexIndexIterator<Vertex>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsVertex Vertex>
bool VertexIndexIterator<Vertex>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsVertex Vertex>
VertexIndexIterator<Vertex>::const_iterator VertexIndexIterator<Vertex>::begin() const
{
    return const_iterator(m_vertices, true);
}

template<IsVertex Vertex>
VertexIndexIterator<Vertex>::const_iterator VertexIndexIterator<Vertex>::end() const
{
    return const_iterator(m_vertices, false);
}

/* EdgeIndexIterator */

template<IsEdge Edge>
EdgeIndexIterator<Edge>::EdgeIndexIterator(const std::vector<Edge>& edges) : m_edges(edges)
{
}

template<IsEdge Edge>
void EdgeIndexIterator<Edge>::const_iterator::advance()
{
    ++m_pos;
}

template<IsEdge Edge>
EdgeIndexIterator<Edge>::const_iterator::const_iterator() : m_pos(-1), m_edges(nullptr)
{
}

template<IsEdge Edge>
EdgeIndexIterator<Edge>::const_iterator::const_iterator(const std::vector<Edge>& edges, bool begin) : m_pos(begin ? 0 : edges.size()), m_edges(&edges)
{
}

template<IsEdge Edge>
EdgeIndexIterator<Edge>::const_iterator::value_type EdgeIndexIterator<Edge>::const_iterator::operator*() const
{
    assert(m_edges);
    assert(m_pos < m_edges->size());
    return m_edges->at(m_pos).get_index();
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
    return const_iterator(m_edges, true);
}

template<IsEdge Edge>
EdgeIndexIterator<Edge>::const_iterator EdgeIndexIterator<Edge>::end() const
{
    return const_iterator(m_edges, false);
}

/* AdjacentVertexConstIterator */

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
void AdjacentVertexConstIterator<Vertex, Edge, Direction>::AdjacentVertexConstIterator::advance()
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
                          "AdjacentVertexConstIterator<Vertex, Edge, Direction>::AdjacentVertexConstIterator::advance(): Missing implementation for "
                          "IsTraversalDirection.");
        }
    } while (true);
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
AdjacentVertexConstIterator<Vertex, Edge, Direction>::AdjacentVertexConstIterator() : m_pos(-1), m_vertex(-1), m_vertices(nullptr), m_edges(nullptr), m_slice()
{
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
AdjacentVertexConstIterator<Vertex, Edge, Direction>::AdjacentVertexConstIterator(VertexIndex vertex,
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
                          "AdjacentVertexConstIterator<Vertex, Edge, Direction>::AdjacentVertexConstIterator(...): Missing implementation for "
                          "IsTraversalDirection.");
        }

        advance();
    }
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
AdjacentVertexConstIterator<Vertex, Edge, Direction>::reference AdjacentVertexConstIterator<Vertex, Edge, Direction>::operator*() const
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
                      "AdjacentVertexConstIterator<Vertex, Edge, Direction>::operator*(): Missing implementation for IsTraversalDirection.");
    }
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
AdjacentVertexConstIterator<Vertex, Edge, Direction>& AdjacentVertexConstIterator<Vertex, Edge, Direction>::operator++()
{
    advance();
    return *this;
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
AdjacentVertexConstIterator<Vertex, Edge, Direction> AdjacentVertexConstIterator<Vertex, Edge, Direction>::operator++(int)
{
    AdjacentVertexConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
bool AdjacentVertexConstIterator<Vertex, Edge, Direction>::operator==(const AdjacentVertexConstIterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
bool AdjacentVertexConstIterator<Vertex, Edge, Direction>::operator!=(const AdjacentVertexConstIterator& other) const
{
    return !(*this == other);
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

/* AdjacentEdgeIterator */

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
                          "AdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_>::advance(): Missing implementation for IsTraversalDirection.");
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
                          "AdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_>::const_iterator(...): Missing implementation for "
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
                      "AdjacentEdgeIterator<Edge, Direction>::const_iterator<Direction_>::operator*(): Missing implementation for IsTraversalDirection.");
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
