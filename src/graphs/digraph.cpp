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

#include <stdexcept>

namespace mimir
{

Digraph::Digraph(bool is_directed) : m_num_vertices(0), m_is_directed(is_directed), m_edges() {}

Digraph::Digraph(size_t num_vertices, bool is_directed) : m_num_vertices(num_vertices), m_is_directed(is_directed) {}

void Digraph::increase_num_vertices(size_t new_num_vertices)
{
    if (new_num_vertices < m_num_vertices)
    {
        throw std::out_of_range("Digraph::increase_num_vertices: new_num_vertices must be greater or equal to m_num_vertices.");
    }
    m_num_vertices = new_num_vertices;
}

DigraphEdgeIndex Digraph::add_edge(DigraphVertexIndex source, DigraphVertexIndex target, DigraphEdgeWeight weight)
{
    if (source >= m_num_vertices || target >= m_num_vertices || source < 0 || target < 0)
    {
        throw std::out_of_range("Digraph::add_edge: Source or destination vertex out of range");
    }
    const auto index = static_cast<DigraphEdgeIndex>(m_edges.size());
    m_edges.emplace_back(index, source, target, weight);
    if (!m_is_directed)
    {
        m_edges.emplace_back(index, target, source, weight);
    }
    return index;
}

void Digraph::reset(size_t num_vertices, bool is_directed)
{
    m_num_vertices = num_vertices;
    m_is_directed = is_directed;
    m_edges.clear();
}

DigraphTargetIndexIterator<DigraphEdge> Digraph::get_targets(DigraphVertexIndex source) const
{
    return DigraphTargetIndexIterator<DigraphEdge>(source, m_edges);
}

DigraphSourceIndexIterator<DigraphEdge> Digraph::get_sources(DigraphVertexIndex target) const
{
    return DigraphSourceIndexIterator<DigraphEdge>(target, m_edges);
}

DigraphForwardEdgeIndexIterator<DigraphEdge> Digraph::get_forward_edge_indices(DigraphVertexIndex source) const
{
    return DigraphForwardEdgeIndexIterator<DigraphEdge>(source, m_edges);
}

DigraphBackwardEdgeIndexIterator<DigraphEdge> Digraph::get_backward_edge_indices(DigraphVertexIndex target) const
{
    return DigraphBackwardEdgeIndexIterator<DigraphEdge>(target, m_edges);
}

DigraphForwardEdgeIterator<DigraphEdge> Digraph::get_forward_edges(DigraphVertexIndex source) const
{
    return DigraphForwardEdgeIterator<DigraphEdge>(source, m_edges);
}

DigraphBackwardEdgeIterator<DigraphEdge> Digraph::get_backward_edges(DigraphVertexIndex target) const
{
    return DigraphBackwardEdgeIterator<DigraphEdge>(target, m_edges);
}

size_t Digraph::get_num_vertices() const { return m_num_vertices; }

size_t Digraph::get_num_edges() const { return m_edges.size(); }

bool Digraph::is_directed() const { return m_is_directed; }

const DigraphEdgeList& Digraph::get_edges() const { return m_edges; }

}