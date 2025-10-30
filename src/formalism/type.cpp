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
#include "mimir/formalism/type.hpp"

namespace mimir::formalism
{
TypeImpl::TypeImpl(Index index, std::string name, TypeList bases) : m_index(index), m_name(std::move(name)), m_bases(std::move(bases)) {}

Index TypeImpl::get_index() const { return m_index; }

const std::string& TypeImpl::get_name() const { return m_name; }

const TypeList& TypeImpl::get_bases() const { return m_bases; }

TypesHierarchy::TypesHierarchy(TypeList types) : m_types(std::move(types)) {}

const TypeList& TypesHierarchy::get_types() const { return m_types; }

bool is_subtypeeq(Type lhs, Type rhs)
{
    if (lhs == rhs)
        return true;

    for (const auto& base : lhs->get_bases())
    {
        if (is_subtypeeq(base, rhs))
            return true;
    }

    return false;
}

bool is_subtypeeq(const TypeList& lhs, const TypeList& rhs)
{
    for (const auto& l : lhs)
    {
        for (const auto& r : rhs)
        {
            if (is_subtypeeq(l, r))
                return true;
        }
    }
    return false;
}
}
