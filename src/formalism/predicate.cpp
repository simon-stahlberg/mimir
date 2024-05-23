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

#include "mimir/formalism/predicate.hpp"

#include "mimir/common/collections.hpp"
#include "mimir/formalism/variable.hpp"

#include <cassert>
#include <loki/loki.hpp>
#include <memory>

namespace mimir
{
StaticPredicateImpl::StaticPredicateImpl(int identifier, std::string name, VariableList parameters) :
    Base(identifier),
    m_name(std::move(name)),
    m_parameters(std::move(parameters))
{
    assert(is_all_unique(m_parameters));
}

bool StaticPredicateImpl::is_structurally_equivalent_to_impl(const StaticPredicateImpl& other) const
{
    if (this != &other)
    {
        return (m_name == other.m_name) && (m_parameters == other.m_parameters);
    }
    return true;
}

size_t StaticPredicateImpl::hash_impl() const { return loki::hash_combine(m_name, loki::hash_container(m_parameters)); }

void StaticPredicateImpl::str_impl(std::ostream& out, const loki::FormattingOptions& options) const
{
    out << "(" << m_name;
    for (size_t i = 0; i < m_parameters.size(); ++i)
    {
        out << " ";
        m_parameters[i]->str(out, options);
    }
    out << ")";
}

const std::string& StaticPredicateImpl::get_name() const { return m_name; }

const VariableList& StaticPredicateImpl::get_parameters() const { return m_parameters; }

size_t StaticPredicateImpl::get_arity() const { return m_parameters.size(); }

FluentPredicateImpl::FluentPredicateImpl(int identifier, std::string name, VariableList parameters) :
    Base(identifier),
    m_name(std::move(name)),
    m_parameters(std::move(parameters))
{
    assert(is_all_unique(m_parameters));
}

bool FluentPredicateImpl::is_structurally_equivalent_to_impl(const FluentPredicateImpl& other) const
{
    if (this != &other)
    {
        return (m_name == other.m_name) && (m_parameters == other.m_parameters);
    }
    return true;
}

size_t FluentPredicateImpl::hash_impl() const { return loki::hash_combine(m_name, loki::hash_container(m_parameters)); }

void FluentPredicateImpl::str_impl(std::ostream& out, const loki::FormattingOptions& options) const
{
    out << "(" << m_name;
    for (size_t i = 0; i < m_parameters.size(); ++i)
    {
        out << " ";
        m_parameters[i]->str(out, options);
    }
    out << ")";
}

const std::string& FluentPredicateImpl::get_name() const { return m_name; }

const VariableList& FluentPredicateImpl::get_parameters() const { return m_parameters; }

size_t FluentPredicateImpl::get_arity() const { return m_parameters.size(); }
}
