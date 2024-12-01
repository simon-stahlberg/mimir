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

#include "mimir/formalism/numeric_fluent.hpp"

#include "formatter.hpp"
#include "mimir/formalism/ground_function.hpp"

namespace mimir
{
NumericFluentImpl::NumericFluentImpl(Index index, GroundFunction function, double number) : m_index(index), m_function(std::move(function)), m_number(number) {}

Index NumericFluentImpl::get_index() const { return m_index; }

const GroundFunction& NumericFluentImpl::get_function() const { return m_function; }

double NumericFluentImpl::get_number() const { return m_number; }

std::ostream& operator<<(std::ostream& out, const NumericFluentImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, NumericFluent element)
{
    out << *element;
    return out;
}

}
