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

#include "mimir/graphs/digraph_edge.hpp"

#include <cassert>
#include <loki/loki.hpp>
#include <stdexcept>
#include <string>

namespace mimir
{
/* DigraphEdge */

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

}
