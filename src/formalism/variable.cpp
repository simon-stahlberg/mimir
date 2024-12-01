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

#include "mimir/formalism/variable.hpp"

#include "formatter.hpp"
#include "mimir/common/hash.hpp"

namespace mimir
{
VariableImpl::VariableImpl(Index index, std::string name, size_t parameter_index) : m_index(index), m_name(std::move(name)), m_parameter_index(parameter_index)
{
}

Index VariableImpl::get_index() const { return m_index; }

const std::string& VariableImpl::get_name() const { return m_name; }

const size_t VariableImpl::get_parameter_index() const { return m_parameter_index; }

std::ostream& operator<<(std::ostream& out, const VariableImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, Variable element)
{
    out << *element;
    return out;
}
}
