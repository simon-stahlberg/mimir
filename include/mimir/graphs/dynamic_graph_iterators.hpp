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
#include "mimir/graphs/declarations.hpp"
#include "mimir/graphs/graph_edge_interface.hpp"
#include "mimir/graphs/graph_traversal_interface.hpp"
#include "mimir/graphs/graph_vertex_interface.hpp"

#include <cassert>
#include <unordered_map>
#include <unordered_set>

namespace mimir
{

template<IsVertex Vertex>
class DynamicVertexIndexConstIterator
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

    DynamicVertexIndexConstIterator();
    DynamicVertexIndexConstIterator(const std::unordered_map<VertexIndex, Vertex>& vertices, bool begin);
    value_type operator*() const;
    DynamicVertexIndexConstIterator& operator++();
    DynamicVertexIndexConstIterator operator++(int);
    bool operator==(const DynamicVertexIndexConstIterator& other) const;
    bool operator!=(const DynamicVertexIndexConstIterator& other) const;
};

template<IsEdge Edge>
class DynamicEdgeIndexConstIterator
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

    DynamicEdgeIndexConstIterator();
    DynamicEdgeIndexConstIterator(const std::unordered_map<EdgeIndex, Edge>& edges, bool begin);
    value_type operator*() const;
    DynamicEdgeIndexConstIterator& operator++();
    DynamicEdgeIndexConstIterator operator++(int);
    bool operator==(const DynamicEdgeIndexConstIterator& other) const;
    bool operator!=(const DynamicEdgeIndexConstIterator& other) const;
};

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
class DynamicAdjacentVertexConstIterator
{
private:
    const std::unordered_map<VertexIndex, Vertex>* m_vertices;
    const std::unordered_map<EdgeIndex, Edge>* m_edges;
    std::unordered_set<EdgeIndex>::const_iterator m_slice_iter;

    void advance();

public:
    using difference_type = std::ptrdiff_t;
    using value_type = const Vertex;
    using pointer = value_type*;
    using reference = const value_type&;
    using iterator_category = std::forward_iterator_tag;

    DynamicAdjacentVertexConstIterator();
    DynamicAdjacentVertexConstIterator(const std::unordered_map<VertexIndex, Vertex>& vertices,
                                       const std::unordered_map<EdgeIndex, Edge>& edges,
                                       const std::unordered_set<EdgeIndex>& slice,
                                       bool begin);
    reference operator*() const;
    DynamicAdjacentVertexConstIterator& operator++();
    DynamicAdjacentVertexConstIterator operator++(int);
    bool operator==(const DynamicAdjacentVertexConstIterator& other) const;
    bool operator!=(const DynamicAdjacentVertexConstIterator& other) const;
};

template<IsEdge Edge, IsTraversalDirection Direction>
class DynamicAdjacentVertexIndexConstIterator
{
private:
    size_t m_pos;
    const std::unordered_map<EdgeIndex, Edge>* m_edges;
    std::unordered_set<EdgeIndex>::const_iterator m_slice_iter;

    void advance();

public:
    using difference_type = std::ptrdiff_t;
    using value_type = VertexIndex;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::forward_iterator_tag;

    DynamicAdjacentVertexIndexConstIterator();
    DynamicAdjacentVertexIndexConstIterator(const std::unordered_map<EdgeIndex, Edge>& edges, const std::unordered_set<EdgeIndex>& slice, bool begin);
    value_type operator*() const;
    DynamicAdjacentVertexIndexConstIterator& operator++();
    DynamicAdjacentVertexIndexConstIterator operator++(int);
    bool operator==(const DynamicAdjacentVertexIndexConstIterator& other) const;
    bool operator!=(const DynamicAdjacentVertexIndexConstIterator& other) const;
};

template<IsEdge Edge, IsTraversalDirection Direction>
class DynamicAdjacentEdgeConstIterator
{
private:
    size_t m_pos;
    const std::unordered_map<EdgeIndex, Edge>* m_edges;
    std::unordered_set<EdgeIndex>::const_iterator m_slice_iter;

    void advance();

public:
    using difference_type = std::ptrdiff_t;
    using value_type = Edge;
    using pointer = value_type*;
    using reference = const value_type&;
    using iterator_category = std::forward_iterator_tag;

    DynamicAdjacentEdgeConstIterator();
    DynamicAdjacentEdgeConstIterator(const std::unordered_map<EdgeIndex, Edge>& edges, const std::unordered_set<EdgeIndex>& slice, bool begin);
    reference operator*() const;
    DynamicAdjacentEdgeConstIterator& operator++();
    DynamicAdjacentEdgeConstIterator operator++(int);
    bool operator==(const DynamicAdjacentEdgeConstIterator& other) const;
    bool operator!=(const DynamicAdjacentEdgeConstIterator& other) const;
};

template<IsEdge Edge, IsTraversalDirection Direction>
class DynamicAdjacentEdgeIndexConstIterator
{
private:
    size_t m_pos;
    const std::unordered_map<EdgeIndex, Edge>* m_edges;
    std::unordered_set<EdgeIndex>::const_iterator m_slice_iter;

    void advance();

public:
    using difference_type = std::ptrdiff_t;
    using value_type = EdgeIndex;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::forward_iterator_tag;

    DynamicAdjacentEdgeIndexConstIterator();
    DynamicAdjacentEdgeIndexConstIterator(const std::unordered_map<EdgeIndex, Edge>& edges, const std::unordered_set<EdgeIndex>& slice, bool begin);
    value_type operator*() const;
    DynamicAdjacentEdgeIndexConstIterator& operator++();
    DynamicAdjacentEdgeIndexConstIterator operator++(int);
    bool operator==(const DynamicAdjacentEdgeIndexConstIterator& other) const;
    bool operator!=(const DynamicAdjacentEdgeIndexConstIterator& other) const;
};

/**
 * Implementations
 */

/* DynamicVertexIndexConstIterator */

template<IsVertex Vertex>
void DynamicVertexIndexConstIterator<Vertex>::advance()
{
    ++m_pos;
}

template<IsVertex Vertex>
DynamicVertexIndexConstIterator<Vertex>::DynamicVertexIndexConstIterator() : m_pos(-1), m_vertices(nullptr)
{
}

template<IsVertex Vertex>
DynamicVertexIndexConstIterator<Vertex>::DynamicVertexIndexConstIterator(const std::unordered_map<VertexIndex, Vertex>& vertices, bool begin) :
    m_pos(begin ? 0 : vertices.size()),
    m_vertices(&vertices)
{
}

template<IsVertex Vertex>
DynamicVertexIndexConstIterator<Vertex>::value_type DynamicVertexIndexConstIterator<Vertex>::operator*() const
{
    assert(m_vertices);
    assert(m_pos < m_vertices->size());
    return m_vertices->at(m_pos).get_index();
}

template<IsVertex Vertex>
DynamicVertexIndexConstIterator<Vertex>& DynamicVertexIndexConstIterator<Vertex>::operator++()
{
    advance();
    return *this;
}

template<IsVertex Vertex>
DynamicVertexIndexConstIterator<Vertex> DynamicVertexIndexConstIterator<Vertex>::operator++(int)
{
    DynamicVertexIndexConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsVertex Vertex>
bool DynamicVertexIndexConstIterator<Vertex>::operator==(const DynamicVertexIndexConstIterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsVertex Vertex>
bool DynamicVertexIndexConstIterator<Vertex>::operator!=(const DynamicVertexIndexConstIterator& other) const
{
    return !(*this == other);
}

/* DynamicEdgeIndexConstIterator */

template<IsEdge Edge>
void DynamicEdgeIndexConstIterator<Edge>::advance()
{
    ++m_pos;
}

template<IsEdge Edge>
DynamicEdgeIndexConstIterator<Edge>::DynamicEdgeIndexConstIterator() : m_pos(-1), m_edges(nullptr)
{
}

template<IsEdge Edge>
DynamicEdgeIndexConstIterator<Edge>::DynamicEdgeIndexConstIterator(const std::unordered_map<EdgeIndex, Edge>& edges, bool begin) :
    m_pos(begin ? 0 : edges.size()),
    m_edges(&edges)
{
}

template<IsEdge Edge>
DynamicEdgeIndexConstIterator<Edge>::value_type DynamicEdgeIndexConstIterator<Edge>::operator*() const
{
    assert(m_edges);
    assert(m_pos < m_edges->size());
    return m_edges->at(m_pos).get_index();
}

template<IsEdge Edge>
DynamicEdgeIndexConstIterator<Edge>& DynamicEdgeIndexConstIterator<Edge>::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge>
DynamicEdgeIndexConstIterator<Edge> DynamicEdgeIndexConstIterator<Edge>::operator++(int)
{
    DynamicEdgeIndexConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge>
bool DynamicEdgeIndexConstIterator<Edge>::operator==(const DynamicEdgeIndexConstIterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge>
bool DynamicEdgeIndexConstIterator<Edge>::operator!=(const DynamicEdgeIndexConstIterator& other) const
{
    return !(*this == other);
}

/* DynamicAdjacentVertexConstIterator */

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
void DynamicAdjacentVertexConstIterator<Vertex, Edge, Direction>::advance()
{
    ++m_slice_iter;
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexConstIterator<Vertex, Edge, Direction>::DynamicAdjacentVertexConstIterator() : m_vertices(nullptr), m_edges(nullptr), m_slice_iter()
{
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexConstIterator<Vertex, Edge, Direction>::DynamicAdjacentVertexConstIterator(const std::unordered_map<VertexIndex, Vertex>& vertices,
                                                                                                const std::unordered_map<EdgeIndex, Edge>& edges,
                                                                                                const std::unordered_set<EdgeIndex>& slice,
                                                                                                bool begin) :
    m_vertices(&vertices),
    m_edges(&edges),
    m_slice_iter(begin ? slice.begin() : slice.end())
{
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexConstIterator<Vertex, Edge, Direction>::reference DynamicAdjacentVertexConstIterator<Vertex, Edge, Direction>::operator*() const
{
    assert(m_vertices);
    assert(m_edges);

    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        return m_vertices->at(m_edges->at(*m_slice_iter).get_target());
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        return m_vertices->at(m_edges->at(*m_slice_iter).get_source());
    }
    else
    {
        static_assert(dependent_false<Direction>::value,
                      "DynamicAdjacentVertexConstIterator<Edge, Direction>::operator*(): Missing implementation for IsTraversalDirection.");
    }
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexConstIterator<Vertex, Edge, Direction>& DynamicAdjacentVertexConstIterator<Vertex, Edge, Direction>::operator++()
{
    advance();
    return *this;
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexConstIterator<Vertex, Edge, Direction> DynamicAdjacentVertexConstIterator<Vertex, Edge, Direction>::operator++(int)
{
    DynamicAdjacentVertexConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
bool DynamicAdjacentVertexConstIterator<Vertex, Edge, Direction>::operator==(const DynamicAdjacentVertexConstIterator& other) const
{
    return (m_slice_iter == other.m_slice_iter);
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
bool DynamicAdjacentVertexConstIterator<Vertex, Edge, Direction>::operator!=(const DynamicAdjacentVertexConstIterator& other) const
{
    return !(*this == other);
}

/* DynamicAdjacentVertexIndexConstIterator */

template<IsEdge Edge, IsTraversalDirection Direction>
void DynamicAdjacentVertexIndexConstIterator<Edge, Direction>::advance()
{
    ++m_slice_iter;
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexIndexConstIterator<Edge, Direction>::DynamicAdjacentVertexIndexConstIterator() : m_edges(nullptr), m_slice_iter()
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexIndexConstIterator<Edge, Direction>::DynamicAdjacentVertexIndexConstIterator(const std::unordered_map<EdgeIndex, Edge>& edges,
                                                                                                  const std::unordered_set<EdgeIndex>& slice,
                                                                                                  bool begin) :
    m_edges(&edges),
    m_slice_iter(begin ? slice.begin() : slice.end())
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexIndexConstIterator<Edge, Direction>::value_type DynamicAdjacentVertexIndexConstIterator<Edge, Direction>::operator*() const
{
    assert(m_edges);

    if constexpr (std::is_same_v<Direction, ForwardTraversal>)
    {
        return m_edges->at(*m_slice_iter).get_target();
    }
    else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
    {
        return m_edges->at(*m_slice_iter).get_source();
    }
    else
    {
        static_assert(dependent_false<Direction>::value,
                      "DynamicAdjacentVertexIndexConstIterator<Edge, Direction>::operator*(): Missing implementation for IsTraversalDirection.");
    }
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexIndexConstIterator<Edge, Direction>& DynamicAdjacentVertexIndexConstIterator<Edge, Direction>::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexIndexConstIterator<Edge, Direction> DynamicAdjacentVertexIndexConstIterator<Edge, Direction>::operator++(int)
{
    DynamicAdjacentVertexIndexConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge, IsTraversalDirection Direction>
bool DynamicAdjacentVertexIndexConstIterator<Edge, Direction>::operator==(const DynamicAdjacentVertexIndexConstIterator& other) const
{
    return (m_slice_iter == other.m_slice_iter);
}

template<IsEdge Edge, IsTraversalDirection Direction>
bool DynamicAdjacentVertexIndexConstIterator<Edge, Direction>::operator!=(const DynamicAdjacentVertexIndexConstIterator& other) const
{
    return !(*this == other);
}

/* DynamicAdjacentEdgeConstIterator */

template<IsEdge Edge, IsTraversalDirection Direction>
void DynamicAdjacentEdgeConstIterator<Edge, Direction>::advance()
{
    ++m_slice_iter;
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeConstIterator<Edge, Direction>::DynamicAdjacentEdgeConstIterator() : m_edges(nullptr), m_slice_iter()
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeConstIterator<Edge, Direction>::DynamicAdjacentEdgeConstIterator(const std::unordered_map<EdgeIndex, Edge>& edges,
                                                                                    const std::unordered_set<EdgeIndex>& slice,
                                                                                    bool begin) :
    m_edges(&edges),
    m_slice_iter(begin ? slice.begin() : slice.end())
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeConstIterator<Edge, Direction>::reference DynamicAdjacentEdgeConstIterator<Edge, Direction>::operator*() const
{
    assert(m_edges);

    return m_edges->at(*m_slice_iter);
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeConstIterator<Edge, Direction>& DynamicAdjacentEdgeConstIterator<Edge, Direction>::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeConstIterator<Edge, Direction> DynamicAdjacentEdgeConstIterator<Edge, Direction>::operator++(int)
{
    DynamicAdjacentEdgeConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge, IsTraversalDirection Direction>
bool DynamicAdjacentEdgeConstIterator<Edge, Direction>::operator==(const DynamicAdjacentEdgeConstIterator& other) const
{
    return (m_slice_iter == other.m_slice_iter);
}

template<IsEdge Edge, IsTraversalDirection Direction>
bool DynamicAdjacentEdgeConstIterator<Edge, Direction>::operator!=(const DynamicAdjacentEdgeConstIterator& other) const
{
    return !(*this == other);
}

/* DynamicAdjacentEdgeIndexConstIterator */

template<IsEdge Edge, IsTraversalDirection Direction>
void DynamicAdjacentEdgeIndexConstIterator<Edge, Direction>::advance()
{
    ++m_slice_iter;
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeIndexConstIterator<Edge, Direction>::DynamicAdjacentEdgeIndexConstIterator() : m_edges(nullptr), m_slice_iter()
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeIndexConstIterator<Edge, Direction>::DynamicAdjacentEdgeIndexConstIterator(const std::unordered_map<EdgeIndex, Edge>& edges,
                                                                                              const std::unordered_set<EdgeIndex>& slice,
                                                                                              bool begin) :
    m_edges(&edges),
    m_slice_iter(begin ? slice.begin() : slice.end())
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeIndexConstIterator<Edge, Direction>::value_type DynamicAdjacentEdgeIndexConstIterator<Edge, Direction>::operator*() const
{
    assert(m_edges);

    return m_edges->at(*m_slice_iter).get_index();
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeIndexConstIterator<Edge, Direction>& DynamicAdjacentEdgeIndexConstIterator<Edge, Direction>::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeIndexConstIterator<Edge, Direction> DynamicAdjacentEdgeIndexConstIterator<Edge, Direction>::operator++(int)
{
    DynamicAdjacentEdgeIndexConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge, IsTraversalDirection Direction>
bool DynamicAdjacentEdgeIndexConstIterator<Edge, Direction>::operator==(const DynamicAdjacentEdgeIndexConstIterator& other) const
{
    return (m_slice_iter == other.m_slice_iter);
}

template<IsEdge Edge, IsTraversalDirection Direction>
bool DynamicAdjacentEdgeIndexConstIterator<Edge, Direction>::operator!=(const DynamicAdjacentEdgeIndexConstIterator& other) const
{
    return !(*this == other);
}

}

#endif
