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

#include <loki/utils/collections.hpp>
#include <loki/utils/hash.hpp>

namespace mimir
{
TypeImpl::TypeImpl(int identifier, std::string name, TypeList bases) : Base(identifier), m_name(std::move(name)), m_bases(std::move(bases)) {}

bool TypeImpl::is_structurally_equivalent_to_impl(const TypeImpl& other) const
{
    return (m_name == other.m_name) && (loki::get_sorted_vector(m_bases) == loki::get_sorted_vector(other.m_bases));
}

size_t TypeImpl::hash_impl() const { return loki::hash_combine(m_name, loki::hash_container(loki::get_sorted_vector(m_bases))); }

void TypeImpl::str_impl(std::ostream& out, const loki::FormattingOptions& /*options*/) const
{
    out << m_name;
    if (!m_bases.empty())
    {
        out << " - ";
        if (m_bases.size() > 1)
        {
            out << "(either ";
            for (size_t i = 0; i < m_bases.size(); ++i)
            {
                if (i != 0)
                    out << " ";
                out << m_bases[i]->get_name();
            }
            out << ")";
        }
        else if (m_bases.size() == 1)
        {
            out << m_bases.front()->get_name();
        }
    }
}

const std::string& TypeImpl::get_name() const { return m_name; }

const TypeList& TypeImpl::get_bases() const { return m_bases; }

bool TypeImpl::is_subtype_of(const Type type) const
{
    if (type == nullptr)
    {
        return false;
    }

    if (type == this)
    {
        return true;
    }

    return is_subtype_of(type->get_bases());
}

bool TypeImpl::is_subtype_of(const TypeList& types) const
{
    for (const auto& type : types)
    {
        if (is_subtype_of(type))
        {
            return true;
        }
    }

    return false;
}

bool is_any_subtype_of(const TypeList& subtypes, const TypeList& types)
{
    for (const auto& subtype : subtypes)
    {
        if (subtype->is_subtype_of(types))
        {
            return true;
        }
    }

    return false;
}

}
