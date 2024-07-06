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

#include "mimir/graphs/digraph.hpp"

#include <cassert>
#include <loki/loki.hpp>
#include <stdexcept>
#include <string>

using namespace std::string_literals;

namespace mimir
{
/* DigraphTransitions */

DigraphEdge::DigraphEdge(DigraphEdgeIndex index, DigraphVertexIndex source, DigraphVertexIndex target, DigraphEdgeWeight weight) :
    m_index(index),
    m_source(source),
    m_target(target),
    m_weight(weight)
{
}

bool DigraphEdge::operator==(const DigraphEdge& other) const
{
    if (this != &other)
    {
        return (m_source == other.m_source) && (m_target == other.m_target) && (m_weight == other.m_weight);
    }
    return true;
}

size_t DigraphEdge::hash() const { return loki::hash_combine(m_source, m_target, m_weight); }

DigraphEdgeIndex DigraphEdge::get_index() const { return m_index; }

DigraphVertexIndex DigraphEdge::get_source() const { return m_source; }

DigraphVertexIndex DigraphEdge::get_target() const { return m_target; }

DigraphEdgeWeight DigraphEdge::get_weight() const { return m_weight; }

/* DigraphTargetIndexIterator */

DigraphTargetIndexIterator::DigraphTargetIndexIterator(DigraphVertexIndex source, std::span<const DigraphEdge> edges) : m_source(source), m_edges(edges) {}

void DigraphTargetIndexIterator::const_iterator::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;
    } while (m_pos < m_edges.size() && m_edges[m_pos].get_source() != m_source);
}

DigraphTargetIndexIterator::const_iterator::const_iterator() {}

DigraphTargetIndexIterator::const_iterator::const_iterator(DigraphVertexIndex source, std::span<const DigraphEdge> edges, bool begin) :
    m_source(source),
    m_pos(begin ? 0 : edges.size()),
    m_edges(edges)
{
    if (begin && m_edges.size() > 0 && m_edges[0].get_source() != m_source)
    {
        advance();
    }
}

DigraphTargetIndexIterator::const_iterator::value_type DigraphTargetIndexIterator::const_iterator::operator*() const
{
    assert(m_pos < m_edges.size());
    return m_edges[m_pos].get_target();
}

DigraphTargetIndexIterator::const_iterator& DigraphTargetIndexIterator::const_iterator::operator++()
{
    advance();
    return *this;
}

DigraphTargetIndexIterator::const_iterator DigraphTargetIndexIterator::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

bool DigraphTargetIndexIterator::const_iterator::operator==(const const_iterator& other) const { return (m_pos == other.m_pos); }

bool DigraphTargetIndexIterator::const_iterator::operator!=(const const_iterator& other) const { return !(*this == other); }

DigraphTargetIndexIterator::const_iterator DigraphTargetIndexIterator::begin() const { return const_iterator(m_source, m_edges, true); }

DigraphTargetIndexIterator::const_iterator DigraphTargetIndexIterator::end() const { return const_iterator(m_source, m_edges, false); }

/* SourceStateIndexIterator */

DigraphSourceIndexIterator::DigraphSourceIndexIterator(DigraphVertexIndex target, std::span<const DigraphEdge> edges) : m_target(target), m_edges(edges) {}

void DigraphSourceIndexIterator::const_iterator::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;
    } while (m_pos < m_edges.size() && m_edges[m_pos].get_target() != m_target);
}

DigraphSourceIndexIterator::const_iterator::const_iterator() {}

DigraphSourceIndexIterator::const_iterator::const_iterator(DigraphVertexIndex target, std::span<const DigraphEdge> edges, bool begin) :
    m_target(target),
    m_pos(begin ? 0 : edges.size()),
    m_edges(edges)
{
    if (begin && m_edges.size() > 0 && m_edges[0].get_target() != m_target)
    {
        advance();
    }
}

DigraphSourceIndexIterator::const_iterator::value_type DigraphSourceIndexIterator::const_iterator::operator*() const
{
    assert(m_pos < m_edges.size());
    return m_edges[m_pos].get_source();
}

DigraphSourceIndexIterator::const_iterator& DigraphSourceIndexIterator::const_iterator::operator++()
{
    advance();
    return *this;
}

DigraphSourceIndexIterator::const_iterator DigraphSourceIndexIterator::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

bool DigraphSourceIndexIterator::const_iterator::operator==(const const_iterator& other) const { return (m_pos == other.m_pos); }

bool DigraphSourceIndexIterator::const_iterator::operator!=(const const_iterator& other) const { return !(*this == other); }

DigraphSourceIndexIterator::const_iterator DigraphSourceIndexIterator::begin() const { return const_iterator(m_target, m_edges, true); }

DigraphSourceIndexIterator::const_iterator DigraphSourceIndexIterator::end() const { return const_iterator(m_target, m_edges, false); }

/* DigraphForwardEdgeIndexIterator */

DigraphForwardEdgeIndexIterator::DigraphForwardEdgeIndexIterator(DigraphVertexIndex source, std::span<const DigraphEdge> edges) :
    m_source(source),
    m_edges(edges)
{
}

void DigraphForwardEdgeIndexIterator::const_iterator::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;
    } while (m_pos < m_edges.size() && m_edges[m_pos].get_source() != m_source);
}

DigraphForwardEdgeIndexIterator::const_iterator::const_iterator() {}

DigraphForwardEdgeIndexIterator::const_iterator::const_iterator(DigraphVertexIndex source, std::span<const DigraphEdge> edges, bool begin) :
    m_source(source),
    m_pos(begin ? 0 : edges.size()),
    m_edges(edges)
{
    if (begin && m_edges.size() > 0 && m_edges[0].get_source() != m_source)
    {
        advance();
    }
}

DigraphForwardEdgeIndexIterator::const_iterator::value_type DigraphForwardEdgeIndexIterator::const_iterator::operator*() const
{
    assert(m_pos < m_edges.size());
    return m_edges[m_pos].get_index();
}

DigraphForwardEdgeIndexIterator::const_iterator& DigraphForwardEdgeIndexIterator::const_iterator::operator++()
{
    advance();
    return *this;
}

DigraphForwardEdgeIndexIterator::const_iterator DigraphForwardEdgeIndexIterator::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

bool DigraphForwardEdgeIndexIterator::const_iterator::operator==(const const_iterator& other) const { return (m_pos == other.m_pos); }

bool DigraphForwardEdgeIndexIterator::const_iterator::operator!=(const const_iterator& other) const { return !(*this == other); }

DigraphForwardEdgeIndexIterator::const_iterator DigraphForwardEdgeIndexIterator::begin() const { return const_iterator(m_source, m_edges, true); }

DigraphForwardEdgeIndexIterator::const_iterator DigraphForwardEdgeIndexIterator::end() const { return const_iterator(m_source, m_edges, false); }

/* DigraphBackwardEdgeIndexIterator */

DigraphBackwardEdgeIndexIterator::DigraphBackwardEdgeIndexIterator(DigraphVertexIndex target, std::span<const DigraphEdge> edges) :
    m_target(target),
    m_edges(edges)
{
}

void DigraphBackwardEdgeIndexIterator::const_iterator::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;
    } while (m_pos < m_edges.size() && m_edges[m_pos].get_target() != m_target);
}

DigraphBackwardEdgeIndexIterator::const_iterator::const_iterator() {}

DigraphBackwardEdgeIndexIterator::const_iterator::const_iterator(DigraphVertexIndex target, std::span<const DigraphEdge> edges, bool begin) :
    m_target(target),
    m_pos(begin ? 0 : edges.size()),
    m_edges(edges)
{
    if (begin && m_edges.size() > 0 && m_edges[0].get_target() != m_target)
    {
        advance();
    }
}

DigraphBackwardEdgeIndexIterator::const_iterator::value_type DigraphBackwardEdgeIndexIterator::const_iterator::operator*() const
{
    assert(m_pos < m_edges.size());
    return m_edges[m_pos].get_index();
}

DigraphBackwardEdgeIndexIterator::const_iterator& DigraphBackwardEdgeIndexIterator::const_iterator::operator++()
{
    advance();
    return *this;
}

DigraphBackwardEdgeIndexIterator::const_iterator DigraphBackwardEdgeIndexIterator::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

bool DigraphBackwardEdgeIndexIterator::const_iterator::operator==(const const_iterator& other) const { return (m_pos == other.m_pos); }

bool DigraphBackwardEdgeIndexIterator::const_iterator::operator!=(const const_iterator& other) const { return !(*this == other); }

DigraphBackwardEdgeIndexIterator::const_iterator DigraphBackwardEdgeIndexIterator::begin() const { return const_iterator(m_target, m_edges, true); }

DigraphBackwardEdgeIndexIterator::const_iterator DigraphBackwardEdgeIndexIterator::end() const { return const_iterator(m_target, m_edges, false); }

/* DigraphForwardEdgeIterator */

DigraphForwardEdgeIterator::DigraphForwardEdgeIterator(DigraphVertexIndex source, std::span<const DigraphEdge> edges) : m_source(source), m_edges(edges) {}

void DigraphForwardEdgeIterator::const_iterator::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;
    } while (m_pos < m_edges.size() && m_edges[m_pos].get_source() != m_source);
}

DigraphForwardEdgeIterator::const_iterator::const_iterator() {}

DigraphForwardEdgeIterator::const_iterator::const_iterator(DigraphVertexIndex source, std::span<const DigraphEdge> edges, bool begin) :
    m_source(source),
    m_pos(begin ? 0 : edges.size()),
    m_edges(edges)
{
    if (begin && m_edges.size() > 0 && m_edges[0].get_source() != m_source)
    {
        advance();
    }
}

DigraphForwardEdgeIterator::const_iterator::reference DigraphForwardEdgeIterator::const_iterator::operator*() const
{
    assert(m_pos < m_edges.size());
    return m_edges[m_pos];
}

DigraphForwardEdgeIterator::const_iterator& DigraphForwardEdgeIterator::const_iterator::operator++()
{
    advance();
    return *this;
}

DigraphForwardEdgeIterator::const_iterator DigraphForwardEdgeIterator::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

bool DigraphForwardEdgeIterator::const_iterator::operator==(const const_iterator& other) const { return (m_pos == other.m_pos); }

bool DigraphForwardEdgeIterator::const_iterator::operator!=(const const_iterator& other) const { return !(*this == other); }

DigraphForwardEdgeIterator::const_iterator DigraphForwardEdgeIterator::begin() const { return const_iterator(m_source, m_edges, true); }

DigraphForwardEdgeIterator::const_iterator DigraphForwardEdgeIterator::end() const { return const_iterator(m_source, m_edges, false); }

/* DigraphBackwardEdgeIterator */

DigraphBackwardEdgeIterator::DigraphBackwardEdgeIterator(DigraphVertexIndex target, std::span<const DigraphEdge> edges) : m_target(target), m_edges(edges) {}

void DigraphBackwardEdgeIterator::const_iterator::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;
    } while (m_pos < m_edges.size() && m_edges[m_pos].get_target() != m_target);
}

DigraphBackwardEdgeIterator::const_iterator::const_iterator() {}

DigraphBackwardEdgeIterator::const_iterator::const_iterator(DigraphVertexIndex target, std::span<const DigraphEdge> edges, bool begin) :
    m_target(target),
    m_pos(begin ? 0 : edges.size()),
    m_edges(edges)
{
    if (begin && m_edges.size() > 0 && m_edges[0].get_target() != m_target)
    {
        advance();
    }
}

DigraphBackwardEdgeIterator::const_iterator::reference DigraphBackwardEdgeIterator::const_iterator::operator*() const
{
    assert(m_pos < m_edges.size());
    return m_edges[m_pos];
}

DigraphBackwardEdgeIterator::const_iterator& DigraphBackwardEdgeIterator::const_iterator::operator++()
{
    advance();
    return *this;
}

DigraphBackwardEdgeIterator::const_iterator DigraphBackwardEdgeIterator::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

bool DigraphBackwardEdgeIterator::const_iterator::operator==(const const_iterator& other) const { return (m_pos == other.m_pos); }

bool DigraphBackwardEdgeIterator::const_iterator::operator!=(const const_iterator& other) const { return !(*this == other); }

DigraphBackwardEdgeIterator::const_iterator DigraphBackwardEdgeIterator::begin() const { return const_iterator(m_target, m_edges, true); }

DigraphBackwardEdgeIterator::const_iterator DigraphBackwardEdgeIterator::end() const { return const_iterator(m_target, m_edges, false); }

/* Digraph */

Digraph::Digraph(bool is_directed) : m_num_vertices(0), m_is_directed(is_directed), m_edges() {}

Digraph::Digraph(int num_vertices, bool is_directed) : m_num_vertices(num_vertices), m_is_directed(is_directed) {}

DigraphEdgeIndex Digraph::add_edge(DigraphVertexIndex source, DigraphVertexIndex target, DigraphEdgeWeight weight)
{
    if (source >= m_num_vertices || target >= m_num_vertices || source < 0 || target < 0)
    {
        throw std::out_of_range("Source or destination vertex out of range");
    }
    const auto index = static_cast<DigraphEdgeIndex>(m_edges.size());
    m_edges.emplace_back(index, source, target, weight);
    if (!m_is_directed)
    {
        m_edges.emplace_back(index, target, source, weight);
    }
    return index;
}

void Digraph::reset(int num_vertices, bool is_directed)
{
    m_num_vertices = num_vertices;
    m_is_directed = is_directed;
    m_edges.clear();
}

DigraphTargetIndexIterator Digraph::get_targets(DigraphVertexIndex source) const { return DigraphTargetIndexIterator(source, m_edges); }

DigraphSourceIndexIterator Digraph::get_sources(DigraphVertexIndex target) const { return DigraphSourceIndexIterator(target, m_edges); }

DigraphForwardEdgeIndexIterator Digraph::get_forward_edge_indices(DigraphVertexIndex source) const { return DigraphForwardEdgeIndexIterator(source, m_edges); }

DigraphBackwardEdgeIndexIterator Digraph::get_backward_edge_indices(DigraphVertexIndex target) const
{
    return DigraphBackwardEdgeIndexIterator(target, m_edges);
}

DigraphForwardEdgeIterator Digraph::get_forward_edges(DigraphVertexIndex source) const { return DigraphForwardEdgeIterator(source, m_edges); }

DigraphBackwardEdgeIterator Digraph::get_backward_edges(DigraphVertexIndex target) const { return DigraphBackwardEdgeIterator(target, m_edges); }

int Digraph::get_num_vertices() const { return m_num_vertices; }

int Digraph::get_num_edges() const { return m_edges.size(); }

bool Digraph::is_directed() const { return m_is_directed; }

const DigraphEdgeList& Digraph::get_edges() const { return m_edges; }

}
