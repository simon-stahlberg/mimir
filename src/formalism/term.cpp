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

#include "mimir/formalism/term.hpp"

#include "loki/pddl/visitors.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/variable.hpp"

#include <loki/utils/collections.hpp>
#include <loki/utils/hash.hpp>

namespace mimir
{
/* TermObjectImpl */
TermObjectImpl::TermObjectImpl(int identifier, Object object) : Base(identifier), m_object(std::move(object)) {}

bool TermObjectImpl::is_structurally_equivalent_to_impl(const TermObjectImpl& other) const
{
    if (this != &other)
    {
        return m_object == other.m_object;
    }
    return true;
}

size_t TermObjectImpl::hash_impl() const { return hash_combine(m_object); }

void TermObjectImpl::str_impl(std::ostringstream& out, const loki::FormattingOptions& /*options*/) const { out << *m_object; }

const Object& TermObjectImpl::get_object() const { return m_object; }

/* TermVariableImpl */
TermVariableImpl::TermVariableImpl(int identifier, Variable variable) : Base(identifier), m_variable(std::move(variable)) {}

bool TermVariableImpl::is_structurally_equivalent_to_impl(const TermVariableImpl& other) const
{
    if (this != &other)
    {
        return m_variable == other.m_variable;
    }
    return true;
}

size_t TermVariableImpl::hash_impl() const { return hash_combine(m_variable); }

void TermVariableImpl::str_impl(std::ostringstream& out, const loki::FormattingOptions& /*options*/) const { out << *m_variable; }

const Variable& TermVariableImpl::get_variable() const { return m_variable; }
}

namespace std
{
bool less<mimir::Term>::operator()(const mimir::Term& left_term, const mimir::Term& right_term) const
{
    return std::visit(loki::pddl::LessComparatorVisitor(), *left_term, *right_term);
}

std::size_t hash<mimir::TermObjectImpl>::operator()(const mimir::TermObjectImpl& term) const { return term.hash(); }

std::size_t hash<mimir::TermVariableImpl>::operator()(const mimir::TermVariableImpl& term) const { return term.hash(); }
}
