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
class VertexIndexConstIterator
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

    VertexIndexConstIterator();
    VertexIndexConstIterator(const std::vector<Vertex>& vertices, bool begin);
    value_type operator*() const;
    VertexIndexConstIterator& operator++();
    VertexIndexConstIterator operator++(int);
    bool operator==(const VertexIndexConstIterator& other) const;
    bool operator!=(const VertexIndexConstIterator& other) const;
};

template<IsEdge Edge>
class EdgeIndexConstIterator
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

    EdgeIndexConstIterator();
    EdgeIndexConstIterator(const std::vector<Edge>& edges, bool begin);
    value_type operator*() const;
    EdgeIndexConstIterator& operator++();
    EdgeIndexConstIterator operator++(int);
    bool operator==(const EdgeIndexConstIterator& other) const;
    bool operator!=(const EdgeIndexConstIterator& other) const;
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
class AdjacentVertexIndexConstIterator
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

    AdjacentVertexIndexConstIterator();
    AdjacentVertexIndexConstIterator(VertexIndex vertex, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool begin);
    value_type operator*() const;
    AdjacentVertexIndexConstIterator& operator++();
    AdjacentVertexIndexConstIterator operator++(int);
    bool operator==(const AdjacentVertexIndexConstIterator& other) const;
    bool operator!=(const AdjacentVertexIndexConstIterator& other) const;
};

template<IsEdge Edge, IsTraversalDirection Direction>
class AdjacentEdgeConstIterator
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

    AdjacentEdgeConstIterator();
    AdjacentEdgeConstIterator(VertexIndex vertex, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool begin);
    reference operator*() const;
    AdjacentEdgeConstIterator& operator++();
    AdjacentEdgeConstIterator operator++(int);
    bool operator==(const AdjacentEdgeConstIterator& other) const;
    bool operator!=(const AdjacentEdgeConstIterator& other) const;
};

template<IsEdge Edge, IsTraversalDirection Direction>
class AdjacentEdgeIndexConstIterator
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

    AdjacentEdgeIndexConstIterator();
    AdjacentEdgeIndexConstIterator(VertexIndex vertex, const std::vector<Edge>& edges, std::span<const EdgeIndex> slice, bool begin);
    value_type operator*() const;
    AdjacentEdgeIndexConstIterator& operator++();
    AdjacentEdgeIndexConstIterator operator++(int);
    bool operator==(const AdjacentEdgeIndexConstIterator& other) const;
    bool operator!=(const AdjacentEdgeIndexConstIterator& other) const;
};

/**
 * Implementations
 */

/* VertexIndexIterator */

template<IsVertex Vertex>
void VertexIndexConstIterator<Vertex>::advance()
{
    ++m_pos;
}

template<IsVertex Vertex>
VertexIndexConstIterator<Vertex>::VertexIndexConstIterator() : m_pos(-1), m_vertices(nullptr)
{
}

template<IsVertex Vertex>
VertexIndexConstIterator<Vertex>::VertexIndexConstIterator(const std::vector<Vertex>& vertices, bool begin) :
    m_pos(begin ? 0 : vertices.size()),
    m_vertices(&vertices)
{
}

template<IsVertex Vertex>
VertexIndexConstIterator<Vertex>::value_type VertexIndexConstIterator<Vertex>::operator*() const
{
    assert(m_vertices);
    assert(m_pos < m_vertices->size());
    return m_vertices->at(m_pos).get_index();
}

template<IsVertex Vertex>
VertexIndexConstIterator<Vertex>& VertexIndexConstIterator<Vertex>::operator++()
{
    advance();
    return *this;
}

template<IsVertex Vertex>
VertexIndexConstIterator<Vertex> VertexIndexConstIterator<Vertex>::operator++(int)
{
    VertexIndexConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsVertex Vertex>
bool VertexIndexConstIterator<Vertex>::operator==(const VertexIndexConstIterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsVertex Vertex>
bool VertexIndexConstIterator<Vertex>::operator!=(const VertexIndexConstIterator& other) const
{
    return !(*this == other);
}

/* EdgeIndexIterator */

template<IsEdge Edge>
void EdgeIndexConstIterator<Edge>::advance()
{
    ++m_pos;
}

template<IsEdge Edge>
EdgeIndexConstIterator<Edge>::EdgeIndexConstIterator() : m_pos(-1), m_edges(nullptr)
{
}

template<IsEdge Edge>
EdgeIndexConstIterator<Edge>::EdgeIndexConstIterator(const std::vector<Edge>& edges, bool begin) : m_pos(begin ? 0 : edges.size()), m_edges(&edges)
{
}

template<IsEdge Edge>
EdgeIndexConstIterator<Edge>::value_type EdgeIndexConstIterator<Edge>::operator*() const
{
    assert(m_edges);
    assert(m_pos < m_edges->size());
    return m_edges->at(m_pos).get_index();
}

template<IsEdge Edge>
EdgeIndexConstIterator<Edge>& EdgeIndexConstIterator<Edge>::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge>
EdgeIndexConstIterator<Edge> EdgeIndexConstIterator<Edge>::operator++(int)
{
    EdgeIndexConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge>
bool EdgeIndexConstIterator<Edge>::operator==(const EdgeIndexConstIterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge>
bool EdgeIndexConstIterator<Edge>::operator!=(const EdgeIndexConstIterator& other) const
{
    return !(*this == other);
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
                          "AdjacentVertexConstIterator<Vertex, Edge, Direction>::advance(): Missing implementation for "
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

/* AdjacentVertexIndexConstIterator */

template<IsEdge Edge, IsTraversalDirection Direction>
void AdjacentVertexIndexConstIterator<Edge, Direction>::advance()
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
                          "AdjacentVertexIndexConstIterator<Edge, Direction>::advance(): Missing implementation for IsTraversalDirection.");
        }
    } while (true);
}

template<IsEdge Edge, IsTraversalDirection Direction>
AdjacentVertexIndexConstIterator<Edge, Direction>::AdjacentVertexIndexConstIterator() : m_pos(-1), m_vertex(-1), m_edges(nullptr), m_slice()
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
AdjacentVertexIndexConstIterator<Edge, Direction>::AdjacentVertexIndexConstIterator(VertexIndex vertex,
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
                          "AdjacentVertexIndexConstIterator<Edge, Direction>::AdjacentEdgeConstIterator(...): Missing implementation for "
                          "IsTraversalDirection.");
        }

        advance();
    }
}

template<IsEdge Edge, IsTraversalDirection Direction>
AdjacentVertexIndexConstIterator<Edge, Direction>::value_type AdjacentVertexIndexConstIterator<Edge, Direction>::operator*() const
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
                      "AdjacentVertexIndexConstIterator<Edge, Direction>::operator*(): Missing implementation for IsTraversalDirection.");
    }
}

template<IsEdge Edge, IsTraversalDirection Direction>
AdjacentVertexIndexConstIterator<Edge, Direction>& AdjacentVertexIndexConstIterator<Edge, Direction>::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge, IsTraversalDirection Direction>
AdjacentVertexIndexConstIterator<Edge, Direction> AdjacentVertexIndexConstIterator<Edge, Direction>::operator++(int)
{
    AdjacentVertexIndexConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge, IsTraversalDirection Direction>
bool AdjacentVertexIndexConstIterator<Edge, Direction>::operator==(const AdjacentVertexIndexConstIterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge, IsTraversalDirection Direction>
bool AdjacentVertexIndexConstIterator<Edge, Direction>::operator!=(const AdjacentVertexIndexConstIterator& other) const
{
    return !(*this == other);
}

/* AdjacentEdgeConstIterator */

template<IsEdge Edge, IsTraversalDirection Direction>
void AdjacentEdgeConstIterator<Edge, Direction>::advance()
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
                          "AdjacentEdgeConstIterator<Edge, Direction>::advance(): Missing implementation for IsTraversalDirection.");
        }
    } while (true);
}

template<IsEdge Edge, IsTraversalDirection Direction>
AdjacentEdgeConstIterator<Edge, Direction>::AdjacentEdgeConstIterator() : m_pos(-1), m_vertex(-1), m_edges(nullptr), m_slice()
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
AdjacentEdgeConstIterator<Edge, Direction>::AdjacentEdgeConstIterator(VertexIndex vertex,
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
                          "AdjacentEdgeConstIterator<Edge, Direction>::AdjacentEdgeConstIterator(...): Missing implementation for IsTraversalDirection.");
        }

        advance();
    }
}

template<IsEdge Edge, IsTraversalDirection Direction>
AdjacentEdgeConstIterator<Edge, Direction>::reference AdjacentEdgeConstIterator<Edge, Direction>::operator*() const
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
                      "AdjacentEdgeConstIterator<Edge, Direction>::operator*(): Missing implementation for IsTraversalDirection.");
    }
}

template<IsEdge Edge, IsTraversalDirection Direction>
AdjacentEdgeConstIterator<Edge, Direction>& AdjacentEdgeConstIterator<Edge, Direction>::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge, IsTraversalDirection Direction>
AdjacentEdgeConstIterator<Edge, Direction> AdjacentEdgeConstIterator<Edge, Direction>::operator++(int)
{
    AdjacentEdgeConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge, IsTraversalDirection Direction>
bool AdjacentEdgeConstIterator<Edge, Direction>::operator==(const AdjacentEdgeConstIterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge, IsTraversalDirection Direction>
bool AdjacentEdgeConstIterator<Edge, Direction>::operator!=(const AdjacentEdgeConstIterator& other) const
{
    return !(*this == other);
}

/* AdjacentEdgeIndexConstIterator */

template<IsEdge Edge, IsTraversalDirection Direction>
void AdjacentEdgeIndexConstIterator<Edge, Direction>::advance()
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
                          "AdjacentEdgeIndexConstIterator<Edge, Direction>::advance(): Missing implementation for IsTraversalDirection.");
        }
    } while (true);
}

template<IsEdge Edge, IsTraversalDirection Direction>
AdjacentEdgeIndexConstIterator<Edge, Direction>::AdjacentEdgeIndexConstIterator() : m_pos(-1), m_vertex(-1), m_edges(nullptr), m_slice()
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
AdjacentEdgeIndexConstIterator<Edge, Direction>::AdjacentEdgeIndexConstIterator(VertexIndex vertex,
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
                          "AdjacentEdgeIndexConstIterator<Edge, Direction>::const_iterator(...): Missing implementation for IsTraversalDirection.");
        }

        advance();
    }
}

template<IsEdge Edge, IsTraversalDirection Direction>
AdjacentEdgeIndexConstIterator<Edge, Direction>::value_type AdjacentEdgeIndexConstIterator<Edge, Direction>::operator*() const
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
                      "AdjacentEdgeIndexConstIterator<Edge, Direction>::operator*(): Missing implementation for IsTraversalDirection.");
    }
}

template<IsEdge Edge, IsTraversalDirection Direction>
AdjacentEdgeIndexConstIterator<Edge, Direction>& AdjacentEdgeIndexConstIterator<Edge, Direction>::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge, IsTraversalDirection Direction>
AdjacentEdgeIndexConstIterator<Edge, Direction> AdjacentEdgeIndexConstIterator<Edge, Direction>::operator++(int)
{
    AdjacentEdgeIndexConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge, IsTraversalDirection Direction>
bool AdjacentEdgeIndexConstIterator<Edge, Direction>::operator==(const AdjacentEdgeIndexConstIterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge, IsTraversalDirection Direction>
bool AdjacentEdgeIndexConstIterator<Edge, Direction>::operator!=(const AdjacentEdgeIndexConstIterator& other) const
{
    return !(*this == other);
}

}

#endif
