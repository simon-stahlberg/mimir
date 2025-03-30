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

namespace mimir::formalism
{

/* TermImpl */
TermImpl::TermImpl(size_t index, std::variant<Object, Variable> object_or_variable) : m_index(index), m_object_or_variable(std::move(object_or_variable)) {}

size_t TermImpl::get_index() const { return m_index; }

const std::variant<Object, Variable>& TermImpl::get_variant() const { return m_object_or_variable; }

std::ostream& operator<<(std::ostream& out, const TermImpl& element)
{
    write(element, StringFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, Term element)
{
    write(*element, AddressFormatter(), out);
    return out;
}
}
