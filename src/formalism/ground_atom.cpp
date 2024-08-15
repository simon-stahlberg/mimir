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

#include "mimir/formalism/ground_atom.hpp"

#include "formatter.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"

namespace mimir
{
template<PredicateCategory P>
GroundAtomImpl<P>::GroundAtomImpl(size_t index, Predicate<P> predicate, ObjectList objects) :
    m_index(index),
    m_predicate(std::move(predicate)),
    m_objects(std::move(objects))
{
}

template<PredicateCategory P>
std::string GroundAtomImpl<P>::str() const
{
    auto out = std::stringstream();
    out << *this;
    return out.str();
}

template<PredicateCategory P>
size_t GroundAtomImpl<P>::get_index() const
{
    return m_index;
}

template<PredicateCategory P>
Predicate<P> GroundAtomImpl<P>::get_predicate() const
{
    return m_predicate;
}

template<PredicateCategory P>
const ObjectList& GroundAtomImpl<P>::get_objects() const
{
    return m_objects;
}

template<PredicateCategory P>
size_t GroundAtomImpl<P>::get_arity() const
{
    return m_objects.size();
}

template class GroundAtomImpl<Static>;
template class GroundAtomImpl<Fluent>;
template class GroundAtomImpl<Derived>;

template<PredicateCategory P>
std::ostream& operator<<(std::ostream& out, const GroundAtomImpl<P>& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, const GroundAtomImpl<Static>& element);
template std::ostream& operator<<(std::ostream& out, const GroundAtomImpl<Fluent>& element);
template std::ostream& operator<<(std::ostream& out, const GroundAtomImpl<Derived>& element);
}
