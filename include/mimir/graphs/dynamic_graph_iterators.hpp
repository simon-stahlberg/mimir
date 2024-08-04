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
    size_t m_pos;
    const std::unordered_map<VertexIndex, Vertex>* m_vertices;

    void advance();

public:
    using difference_type = std::ptrdiff_t;
    using value_type = VertexIndex;
    using pointer = value_type*;
    using reference = const value_type&;
    using iterator_category = std::forward_iterator_tag;

    DynamicVertexIndexIterator();
    DynamicVertexIndexIterator(const std::unordered_map<VertexIndex, Vertex>& vertices, bool begin);
    value_type operator*() const;
    DynamicVertexIndexIterator& operator++();
    DynamicVertexIndexIterator operator++(int);
    bool operator==(const DynamicVertexIndexIterator& other) const;
    bool operator!=(const DynamicVertexIndexIterator& other) const;
};

template<IsEdge Edge>
class DynamicEdgeIndexIterator
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

    DynamicEdgeIndexIterator();
    DynamicEdgeIndexIterator(const std::unordered_map<EdgeIndex, Edge>& edges, bool begin);
    value_type operator*() const;
    DynamicEdgeIndexIterator& operator++();
    DynamicEdgeIndexIterator operator++(int);
    bool operator==(const DynamicEdgeIndexIterator& other) const;
    bool operator!=(const DynamicEdgeIndexIterator& other) const;
};

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
class DynamicAdjacentVertexIterator
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

    DynamicAdjacentVertexIterator();
    DynamicAdjacentVertexIterator(const std::unordered_map<VertexIndex, Vertex>& vertices,
                                  const std::unordered_map<EdgeIndex, Edge>& edges,
                                  const std::unordered_set<EdgeIndex>& slice,
                                  bool begin);
    reference operator*() const;
    DynamicAdjacentVertexIterator& operator++();
    DynamicAdjacentVertexIterator operator++(int);
    bool operator==(const DynamicAdjacentVertexIterator& other) const;
    bool operator!=(const DynamicAdjacentVertexIterator& other) const;
};

template<IsEdge Edge, IsTraversalDirection Direction>
class DynamicAdjacentVertexIndexIterator
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

    DynamicAdjacentVertexIndexIterator();
    DynamicAdjacentVertexIndexIterator(const std::unordered_map<EdgeIndex, Edge>& edges, const std::unordered_set<EdgeIndex>& slice, bool begin);
    value_type operator*() const;
    DynamicAdjacentVertexIndexIterator& operator++();
    DynamicAdjacentVertexIndexIterator operator++(int);
    bool operator==(const DynamicAdjacentVertexIndexIterator& other) const;
    bool operator!=(const DynamicAdjacentVertexIndexIterator& other) const;
};

template<IsEdge Edge, IsTraversalDirection Direction>
class DynamicAdjacentEdgeIterator
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

    DynamicAdjacentEdgeIterator();
    DynamicAdjacentEdgeIterator(const std::unordered_map<EdgeIndex, Edge>& edges, const std::unordered_set<EdgeIndex>& slice, bool begin);
    reference operator*() const;
    DynamicAdjacentEdgeIterator& operator++();
    DynamicAdjacentEdgeIterator operator++(int);
    bool operator==(const DynamicAdjacentEdgeIterator& other) const;
    bool operator!=(const DynamicAdjacentEdgeIterator& other) const;
};

template<IsEdge Edge, IsTraversalDirection Direction>
class DynamicAdjacentEdgeIndexIterator
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

    DynamicAdjacentEdgeIndexIterator();
    DynamicAdjacentEdgeIndexIterator(const std::unordered_map<EdgeIndex, Edge>& edges, const std::unordered_set<EdgeIndex>& slice, bool begin);
    value_type operator*() const;
    DynamicAdjacentEdgeIndexIterator& operator++();
    DynamicAdjacentEdgeIndexIterator operator++(int);
    bool operator==(const DynamicAdjacentEdgeIndexIterator& other) const;
    bool operator!=(const DynamicAdjacentEdgeIndexIterator& other) const;
};

/**
 * Implementations
 */

/* DynamicVertexIndexIterator */

template<IsVertex Vertex>
void DynamicVertexIndexIterator<Vertex>::advance()
{
    ++m_pos;
}

template<IsVertex Vertex>
DynamicVertexIndexIterator<Vertex>::DynamicVertexIndexIterator() : m_pos(-1), m_vertices(nullptr)
{
}

template<IsVertex Vertex>
DynamicVertexIndexIterator<Vertex>::DynamicVertexIndexIterator(const std::unordered_map<VertexIndex, Vertex>& vertices, bool begin) :
    m_pos(begin ? 0 : vertices.size()),
    m_vertices(&vertices)
{
}

template<IsVertex Vertex>
DynamicVertexIndexIterator<Vertex>::value_type DynamicVertexIndexIterator<Vertex>::operator*() const
{
    assert(m_vertices);
    assert(m_pos < m_vertices->size());
    return m_vertices->at(m_pos).get_index();
}

template<IsVertex Vertex>
DynamicVertexIndexIterator<Vertex>& DynamicVertexIndexIterator<Vertex>::operator++()
{
    advance();
    return *this;
}

template<IsVertex Vertex>
DynamicVertexIndexIterator<Vertex> DynamicVertexIndexIterator<Vertex>::operator++(int)
{
    DynamicVertexIndexIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsVertex Vertex>
bool DynamicVertexIndexIterator<Vertex>::operator==(const DynamicVertexIndexIterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsVertex Vertex>
bool DynamicVertexIndexIterator<Vertex>::operator!=(const DynamicVertexIndexIterator& other) const
{
    return !(*this == other);
}

/* DynamicEdgeIndexIterator */

template<IsEdge Edge>
void DynamicEdgeIndexIterator<Edge>::advance()
{
    ++m_pos;
}

template<IsEdge Edge>
DynamicEdgeIndexIterator<Edge>::DynamicEdgeIndexIterator() : m_pos(-1), m_edges(nullptr)
{
}

template<IsEdge Edge>
DynamicEdgeIndexIterator<Edge>::DynamicEdgeIndexIterator(const std::unordered_map<EdgeIndex, Edge>& edges, bool begin) :
    m_pos(begin ? 0 : edges.size()),
    m_edges(&edges)
{
}

template<IsEdge Edge>
DynamicEdgeIndexIterator<Edge>::value_type DynamicEdgeIndexIterator<Edge>::operator*() const
{
    assert(m_edges);
    assert(m_pos < m_edges->size());
    return m_edges->at(m_pos).get_index();
}

template<IsEdge Edge>
DynamicEdgeIndexIterator<Edge>& DynamicEdgeIndexIterator<Edge>::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge>
DynamicEdgeIndexIterator<Edge> DynamicEdgeIndexIterator<Edge>::operator++(int)
{
    DynamicEdgeIndexIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge>
bool DynamicEdgeIndexIterator<Edge>::operator==(const DynamicEdgeIndexIterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge>
bool DynamicEdgeIndexIterator<Edge>::operator!=(const DynamicEdgeIndexIterator& other) const
{
    return !(*this == other);
}

/* DynamicAdjacentVertexIterator */

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
void DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::advance()
{
    ++m_pos;
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::DynamicAdjacentVertexIterator() : m_pos(-1), m_vertices(nullptr), m_edges(nullptr), m_slice()
{
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::DynamicAdjacentVertexIterator(const std::unordered_map<VertexIndex, Vertex>& vertices,
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
DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::reference DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::operator*() const
{
    assert(m_vertices);
    assert(m_edges);
    assert(m_pos < m_slice.size());

    return m_vertices->at(m_edges->at(m_slice[m_pos]).get_source());
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexIterator<Vertex, Edge, Direction>& DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::operator++()
{
    advance();
    return *this;
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexIterator<Vertex, Edge, Direction> DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
bool DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::operator==(const DynamicAdjacentVertexIterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsVertex Vertex, IsEdge Edge, IsTraversalDirection Direction>
bool DynamicAdjacentVertexIterator<Vertex, Edge, Direction>::operator!=(const DynamicAdjacentVertexIterator& other) const
{
    return !(*this == other);
}

/* DynamicAdjacentVertexIndexIterator */

template<IsEdge Edge, IsTraversalDirection Direction>
void DynamicAdjacentVertexIndexIterator<Edge, Direction>::advance()
{
    ++m_pos;
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexIndexIterator<Edge, Direction>::DynamicAdjacentVertexIndexIterator() : m_pos(-1), m_edges(nullptr), m_slice()
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexIndexIterator<Edge, Direction>::DynamicAdjacentVertexIndexIterator(const std::unordered_map<EdgeIndex, Edge>& edges,
                                                                                        const std::unordered_set<EdgeIndex>& slice,
                                                                                        bool begin) :
    m_pos(begin ? 0 : slice.size()),
    m_edges(&edges),
    m_slice(slice)
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexIndexIterator<Edge, Direction>::value_type DynamicAdjacentVertexIndexIterator<Edge, Direction>::operator*() const
{
    assert(m_edges);
    assert(m_pos < m_slice.size());

    return m_edges->at(m_slice[m_pos]).get_source();
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexIndexIterator<Edge, Direction>& DynamicAdjacentVertexIndexIterator<Edge, Direction>::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentVertexIndexIterator<Edge, Direction> DynamicAdjacentVertexIndexIterator<Edge, Direction>::operator++(int)
{
    DynamicAdjacentVertexIndexIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge, IsTraversalDirection Direction>
bool DynamicAdjacentVertexIndexIterator<Edge, Direction>::operator==(const DynamicAdjacentVertexIndexIterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge, IsTraversalDirection Direction>
bool DynamicAdjacentVertexIndexIterator<Edge, Direction>::operator!=(const DynamicAdjacentVertexIndexIterator& other) const
{
    return !(*this == other);
}

/* DynamicAdjacentEdgeIterator */

template<IsEdge Edge, IsTraversalDirection Direction>
void DynamicAdjacentEdgeIterator<Edge, Direction>::advance()
{
    ++m_pos;
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeIterator<Edge, Direction>::DynamicAdjacentEdgeIterator() : m_pos(-1), m_edges(nullptr), m_slice()
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeIterator<Edge, Direction>::DynamicAdjacentEdgeIterator(const std::unordered_map<EdgeIndex, Edge>& edges,
                                                                          const std::unordered_set<EdgeIndex>& slice,
                                                                          bool begin) :
    m_pos(begin ? 0 : slice.size()),
    m_edges(&edges),
    m_slice(slice)
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeIterator<Edge, Direction>::reference DynamicAdjacentEdgeIterator<Edge, Direction>::operator*() const
{
    assert(m_edges);
    assert(m_pos < m_slice.size());

    return m_edges->at(m_slice[m_pos]);
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeIterator<Edge, Direction>& DynamicAdjacentEdgeIterator<Edge, Direction>::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeIterator<Edge, Direction> DynamicAdjacentEdgeIterator<Edge, Direction>::operator++(int)
{
    DynamicAdjacentEdgeIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge, IsTraversalDirection Direction>
bool DynamicAdjacentEdgeIterator<Edge, Direction>::operator==(const DynamicAdjacentEdgeIterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge, IsTraversalDirection Direction>
bool DynamicAdjacentEdgeIterator<Edge, Direction>::operator!=(const DynamicAdjacentEdgeIterator& other) const
{
    return !(*this == other);
}

/* DynamicAdjacentEdgeIndexIterator */

template<IsEdge Edge, IsTraversalDirection Direction>
void DynamicAdjacentEdgeIndexIterator<Edge, Direction>::advance()
{
    ++m_pos;
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeIndexIterator<Edge, Direction>::DynamicAdjacentEdgeIndexIterator() : m_pos(-1), m_edges(nullptr), m_slice()
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeIndexIterator<Edge, Direction>::DynamicAdjacentEdgeIndexIterator(const std::unordered_map<EdgeIndex, Edge>& edges,
                                                                                    const std::unordered_set<EdgeIndex>& slice,
                                                                                    bool begin) :
    m_pos(begin ? 0 : slice.size()),
    m_edges(&edges),
    m_slice(slice)
{
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeIndexIterator<Edge, Direction>::value_type DynamicAdjacentEdgeIndexIterator<Edge, Direction>::operator*() const
{
    assert(m_edges);
    assert(m_pos < m_slice.size());

    return m_edges->at(m_slice[m_pos]).get_index();
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeIndexIterator<Edge, Direction>& DynamicAdjacentEdgeIndexIterator<Edge, Direction>::operator++()
{
    advance();
    return *this;
}

template<IsEdge Edge, IsTraversalDirection Direction>
DynamicAdjacentEdgeIndexIterator<Edge, Direction> DynamicAdjacentEdgeIndexIterator<Edge, Direction>::operator++(int)
{
    DynamicAdjacentEdgeIndexIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsEdge Edge, IsTraversalDirection Direction>
bool DynamicAdjacentEdgeIndexIterator<Edge, Direction>::operator==(const DynamicAdjacentEdgeIndexIterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsEdge Edge, IsTraversalDirection Direction>
bool DynamicAdjacentEdgeIndexIterator<Edge, Direction>::operator!=(const DynamicAdjacentEdgeIndexIterator& other) const
{
    return !(*this == other);
}

}

#endif
