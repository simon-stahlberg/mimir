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

#include "mimir/graphs/graph_vertices.hpp"

namespace mimir
{

/* EmptyPropertiesVertex */

EmptyPropertiesVertex::EmptyPropertiesVertex(VertexIndex index) : BaseVertex<EmptyPropertiesVertex>(index) {}

bool EmptyPropertiesVertex::is_equal_impl(const BaseVertex<EmptyPropertiesVertex>& other) const { return true; }

size_t EmptyPropertiesVertex::hash_impl() const { return loki::hash_combine(0); }

/* ColoredVertex */

ColoredVertex::ColoredVertex(VertexIndex index, Color color) : BaseVertex<ColoredVertex>(index), m_color(color) {}

bool ColoredVertex::is_equal_impl(const BaseVertex<ColoredVertex>& other) const
{
    if (this != &other)
    {
        const auto& otherDerived = static_cast<const ColoredVertex&>(other);
        return (m_color == otherDerived.m_color);
    }
    return true;
}

size_t ColoredVertex::hash_impl() const { return loki::hash_combine(m_color); }

Color ColoredVertex::get_color() const { return m_color; }

}
