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

#include "mimir/formalism/binding.hpp"

#include "formatter.hpp"

namespace mimir::formalism
{

BindingImpl::BindingImpl(Index index, ObjectList objects) : m_index(index), m_objects(std::move(objects)) {}

Index BindingImpl::get_index() const { return m_index; }

const ObjectList& BindingImpl::get_objects() const { return m_objects; }

size_t BindingImpl::get_arity() const { return m_objects.size(); }

Object BindingImpl::get_object(Index pos) const { return m_objects.at(pos); }

std::ostream& operator<<(std::ostream& out, const BindingImpl& element)
{
    write(element, StringFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, Binding element)
{
    write(*element, StringFormatter(), out);
    return out;
}

}
