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

#include "formatter.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/variable.hpp"

namespace mimir
{
/* TermObjectImpl */
TermObjectImpl::TermObjectImpl(Index index, Object object) : m_index(index), m_object(std::move(object)) {}

std::string TermObjectImpl::str() const
{
    auto out = std::stringstream();
    out << *this;
    return out.str();
}

Index TermObjectImpl::get_index() const { return m_index; }

const Object& TermObjectImpl::get_object() const { return m_object; }

/* TermVariableImpl */
TermVariableImpl::TermVariableImpl(Index index, Variable variable) : m_index(index), m_variable(std::move(variable)) {}

std::string TermVariableImpl::str() const
{
    auto out = std::stringstream();
    out << *this;
    return out.str();
}

Index TermVariableImpl::get_index() const { return m_index; }

const Variable& TermVariableImpl::get_variable() const { return m_variable; }

std::ostream& operator<<(std::ostream& out, const TermObjectImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, const TermVariableImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, const TermImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, Term element)
{
    std::visit([&](const auto& arg) { out << arg; }, *element);
    return out;
}
}
