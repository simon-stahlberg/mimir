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
#include "mimir/formalism/literal.hpp"

#include "formatter.hpp"
#include "mimir/formalism/atom.hpp"
#include "mimir/formalism/predicate.hpp"

namespace mimir
{

template<PredicateTag P>
LiteralImpl<P>::LiteralImpl(Index index, bool is_negated, Atom<P> atom) : m_index(index), m_is_negated(is_negated), m_atom(std::move(atom))
{
}

template<PredicateTag P>
Index LiteralImpl<P>::get_index() const
{
    return m_index;
}

template<PredicateTag P>
bool LiteralImpl<P>::is_negated() const
{
    return m_is_negated;
}

template<PredicateTag P>
const Atom<P>& LiteralImpl<P>::get_atom() const
{
    return m_atom;
}

template class LiteralImpl<Static>;
template class LiteralImpl<Fluent>;
template class LiteralImpl<Derived>;

template<PredicateTag P>
std::ostream& operator<<(std::ostream& out, const LiteralImpl<P>& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, const LiteralImpl<Static>& element);
template std::ostream& operator<<(std::ostream& out, const LiteralImpl<Fluent>& element);
template std::ostream& operator<<(std::ostream& out, const LiteralImpl<Derived>& element);

template<PredicateTag P>
std::ostream& operator<<(std::ostream& out, Literal<P> element)
{
    out << *element;
    return out;
}

template std::ostream& operator<<(std::ostream& out, Literal<Static> element);
template std::ostream& operator<<(std::ostream& out, Literal<Fluent> element);
template std::ostream& operator<<(std::ostream& out, Literal<Derived> element);

}
