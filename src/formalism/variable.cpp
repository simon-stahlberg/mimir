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

#include <loki/loki.hpp>

namespace mimir
{
VariableImpl::VariableImpl(int identifier, std::string name, size_t parameter_index) :
    Base(identifier),
    m_name(std::move(name)),
    m_parameter_index(parameter_index)
{
}

bool VariableImpl::is_structurally_equivalent_to_impl(const VariableImpl& other) const
{
    if (this != &other)
    {
        return (m_name == other.m_name && m_parameter_index == other.m_parameter_index);
    }
    return true;
}

size_t VariableImpl::hash_impl() const { return loki::hash_combine(m_name, m_parameter_index); }

void VariableImpl::str_impl(std::ostream& out, const loki::FormattingOptions& /*options*/) const { out << m_name; }

const std::string& VariableImpl::get_name() const { return m_name; }

const size_t VariableImpl::get_parameter_index() const { return m_parameter_index; }
}
