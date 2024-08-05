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

#include "mimir/graphs/graph_edges.hpp"

namespace mimir
{

/* EmptyPropertiesEdge */

EmptyPropertiesEdge::EmptyPropertiesEdge(EdgeIndex index, VertexIndex source, VertexIndex target) : BaseEdge<EmptyPropertiesEdge>(index, source, target) {}

bool EmptyPropertiesEdge::is_equal_impl(const BaseEdge<EmptyPropertiesEdge>& other) const { return true; }

size_t EmptyPropertiesEdge::hash_impl() const { return loki::hash_combine(0); }

/* ColoredEdge */

ColoredEdge::ColoredEdge(EdgeIndex index, VertexIndex source, VertexIndex target, Color color) : BaseEdge<ColoredEdge>(index, source, target), m_color(color) {}

bool ColoredEdge::is_equal_impl(const BaseEdge<ColoredEdge>& other) const
{
    if (this != &other)
    {
        const auto& otherDerived = static_cast<const ColoredEdge&>(other);
        return (m_color == otherDerived.m_color);
    }
    return true;
}

size_t ColoredEdge::hash_impl() const { return loki::hash_combine(m_color); }

Color ColoredEdge::get_color() const { return m_color; }

}