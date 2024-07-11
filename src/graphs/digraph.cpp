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

namespace mimir
{

/* DigraphVertex */

DigraphVertex::DigraphVertex(VertexIndex index) : m_index(index) {}

bool DigraphVertex::operator==(const DigraphVertex& other) const { return m_index == other.m_index; }

size_t DigraphVertex::hash() const { return loki::hash_combine(m_index); }

VertexIndex DigraphVertex::get_index() const { return m_index; }

/* DigraphEdge */

DigraphEdge::DigraphEdge(EdgeIndex index, VertexIndex source, VertexIndex target) : m_index(index), m_source(source), m_target(target) {}

bool DigraphEdge::operator==(const DigraphEdge& other) const
{
    if (this != &other)
    {
        return (m_index == other.m_index) && (m_source == other.m_source) && (m_target == other.m_target);
    }
    return true;
}

size_t DigraphEdge::hash() const { return loki::hash_combine(m_source, m_target); }

EdgeIndex DigraphEdge::get_index() const { return m_index; }

VertexIndex DigraphEdge::get_source() const { return m_source; }

VertexIndex DigraphEdge::get_target() const { return m_target; }

/**
 * Pretty printing
 */

std::ostream& operator<<(std::ostream& out, const Digraph& digraph)
{
    out << "digraph {\n";
    for (const auto& vertex : digraph.get_vertices())
    {
        out << "t" << vertex.get_index() << "[";
        out << "label=\"" << vertex.get_index() << "\"]\n";
    }
    for (const auto& vertex : digraph.get_vertices())
    {
        for (const auto& succ_vertex : digraph.get_targets(vertex.get_index()))
        {
            out << "t" << vertex.get_index() << "->"
                << "t" << succ_vertex.get_index() << "\n";
        }
    }
    out << "}";  // end digraph

    return out;
}

}
