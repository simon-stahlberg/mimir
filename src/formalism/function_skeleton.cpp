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

#include "mimir/formalism/function_skeleton.hpp"

#include "mimir/formalism/variable.hpp"

#include <loki/loki.hpp>

namespace mimir
{
FunctionSkeletonImpl::FunctionSkeletonImpl(int identifier, std::string name, VariableList parameters) :
    Base(identifier),
    m_name(std::move(name)),
    m_parameters(std::move(parameters))
{
}

bool FunctionSkeletonImpl::is_structurally_equivalent_to_impl(const FunctionSkeletonImpl& other) const
{
    if (this != &other)
    {
        return (m_name == other.m_name) && (m_parameters == other.m_parameters);
    }
    return true;
}

size_t FunctionSkeletonImpl::hash_impl() const { return loki::hash_combine(m_name, loki::hash_container(m_parameters)); }

void FunctionSkeletonImpl::str_impl(std::ostream& out, const loki::FormattingOptions& options) const
{
    out << "(" << m_name;
    for (size_t i = 0; i < m_parameters.size(); ++i)
    {
        out << " ";
        m_parameters[i]->str(out, options);
    }
    out << ")";
}

const std::string& FunctionSkeletonImpl::get_name() const { return m_name; }

const VariableList& FunctionSkeletonImpl::get_parameters() const { return m_parameters; }
}
