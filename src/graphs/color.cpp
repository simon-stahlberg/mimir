/*
 * Copyright (C) 2023 Dominik Drexler
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

#include "mimir/graphs/color.hpp"

namespace mimir::graphs
{
/**
 * IColor
 */

std::ostream& operator<<(std::ostream& out, const IColor& color)
{
    out << color.str();
    return out;
}

/**
 * Color
 */

Color::Color(std::shared_ptr<IColor> color) : m_color(color) {}

bool Color::operator==(const Color& other) const { return loki::EqualTo<IColor>()(*m_color, *other.m_color); }

bool Color::operator<(const Color& other) const { return *m_color < *other.m_color; }

std::string Color::str() const { return m_color->str(); }

size_t Color::hash() const { return m_color->hash(); }

std::ostream& operator<<(std::ostream& out, const Color& color)
{
    out << color.str();
    return out;
}
}

size_t loki::Hash<mimir::graphs::IColor>::operator()(const mimir::graphs::IColor& color) const { return color.hash(); }

bool loki::EqualTo<mimir::graphs::IColor>::operator()(const mimir::graphs::IColor& lhs, const mimir::graphs::IColor& rhs) const { return lhs == rhs; }

size_t loki::Hash<mimir::graphs::Color>::operator()(const mimir::graphs::Color& color) const { return color.hash(); }
