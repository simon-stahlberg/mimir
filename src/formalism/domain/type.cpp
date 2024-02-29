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

#include <loki/common/collections.hpp>
#include <loki/common/hash.hpp>
#include <mimir/formalism/domain/type.hpp>

namespace mimir
{
TypeImpl::TypeImpl(int identifier, std::string name, TypeList bases) : Base(identifier), m_name(std::move(name)), m_bases(std::move(bases)) {}

bool TypeImpl::is_structurally_equivalent_to_impl(const TypeImpl& other) const
{
    return (m_name == other.m_name) && (loki::get_sorted_vector(m_bases) == loki::get_sorted_vector(other.m_bases));
}

size_t TypeImpl::hash_impl() const { return loki::hash_combine(m_name, loki::hash_container(loki::get_sorted_vector(m_bases))); }

void TypeImpl::str_impl(std::ostringstream& out, const loki::FormattingOptions& /*options*/) const { out << m_name; }

const std::string& TypeImpl::get_name() const { return m_name; }

const TypeList& TypeImpl::get_bases() const { return m_bases; }
}

namespace std
{
bool less<mimir::Type>::operator()(const mimir::Type& left_type, const mimir::Type& right_type) const { return *left_type < *right_type; }

std::size_t hash<mimir::TypeImpl>::operator()(const mimir::TypeImpl& type) const { return type.hash(); }
}
