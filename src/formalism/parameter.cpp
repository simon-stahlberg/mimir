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

#include "mimir/formalism/parameter.hpp"

#include "formatter.hpp"

#include <cassert>

namespace mimir::formalism
{
ParameterImpl::ParameterImpl(Index index, Variable variable, TypeList types) : m_index(index), m_variable(std::move(variable)), m_types(std::move(types)) {}

Index ParameterImpl::get_index() const { return m_index; }

Variable ParameterImpl::get_variable() const { return m_variable; }

const TypeList& ParameterImpl::get_bases() const { return m_types; }

std::ostream& operator<<(std::ostream& out, const ParameterImpl& element)
{
    write(element, StringFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, Parameter element)
{
    write(*element, StringFormatter(), out);
    return out;
}

}
