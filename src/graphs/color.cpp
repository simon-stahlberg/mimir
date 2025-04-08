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
 * AbstractColor
 */

bool AbstractColor::operator==(const AbstractColor& other) const { return this->is_equal_to(other); }

bool AbstractColor::operator<(const AbstractColor& other) const { return this->is_less_than(other); }

std::ostream& operator<<(std::ostream& out, const AbstractColor& color)
{
    color.to_ostream(out);

    return out;
}

}

size_t loki::Hash<mimir::graphs::AbstractColor>::operator()(const mimir::graphs::AbstractColor& color) const { return color.hash(); }

bool loki::EqualTo<mimir::graphs::AbstractColor>::operator()(const mimir::graphs::AbstractColor& lhs, const mimir::graphs::AbstractColor& rhs) const
{
    return lhs == rhs;
}
