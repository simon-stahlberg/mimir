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

#include <loki/utils/hash.hpp>
#include <memory>
#include <mimir/formalism/parameter.hpp>
#include <mimir/formalism/predicate.hpp>
#include <mimir/formalism/type.hpp>
#include <mimir/formalism/variable.hpp>

namespace mimir
{
PredicateImpl::PredicateImpl(int identifier, std::string name, ParameterList parameters) :
    Base(identifier),
    m_name(std::move(name)),
    m_parameters(std::move(parameters))
{
}

bool PredicateImpl::is_structurally_equivalent_to_impl(const PredicateImpl& other) const
{
    return (m_name == other.m_name) && (m_parameters == other.m_parameters);
}

size_t PredicateImpl::hash_impl() const { return loki::hash_combine(m_name, loki::hash_container(m_parameters)); }

void PredicateImpl::str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const { str(out, options, true); }

void PredicateImpl::str(std::ostringstream& out, const loki::FormattingOptions& options, bool typing_enabled) const
{
    out << "(" << m_name;
    for (size_t i = 0; i < m_parameters.size(); ++i)
    {
        out << " ";
        m_parameters[i]->str(out, options, typing_enabled);
    }
    out << ")";
}

const std::string& PredicateImpl::get_name() const { return m_name; }

const ParameterList& PredicateImpl::get_parameters() const { return m_parameters; }

size_t PredicateImpl::get_arity() const { return m_parameters.size(); }
}

namespace std
{
bool less<mimir::Predicate>::operator()(const mimir::Predicate& left_predicate, const mimir::Predicate& right_predicate) const
{
    return *left_predicate < *right_predicate;
}

std::size_t hash<mimir::PredicateImpl>::operator()(const mimir::PredicateImpl& predicate) const { return predicate.hash(); }
}
