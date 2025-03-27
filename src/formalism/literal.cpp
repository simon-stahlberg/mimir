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

namespace mimir::formalism
{

template<IsStaticOrFluentOrDerivedTag P>
LiteralImpl<P>::LiteralImpl(Index index, bool polarity, Atom<P> atom) : m_index(index), m_polarity(polarity), m_atom(std::move(atom))
{
}

template<IsStaticOrFluentOrDerivedTag P>
Index LiteralImpl<P>::get_index() const
{
    return m_index;
}

template<IsStaticOrFluentOrDerivedTag P>
bool LiteralImpl<P>::get_polarity() const
{
    return m_polarity;
}

template<IsStaticOrFluentOrDerivedTag P>
Atom<P> LiteralImpl<P>::get_atom() const
{
    return m_atom;
}

template class LiteralImpl<StaticTag>;
template class LiteralImpl<FluentTag>;
template class LiteralImpl<DerivedTag>;

template<IsStaticOrFluentOrDerivedTag P>
std::ostream& operator<<(std::ostream& out, const LiteralImpl<P>& element)
{
    write(element, StringFormatter(), out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, const LiteralImpl<StaticTag>& element);
template std::ostream& operator<<(std::ostream& out, const LiteralImpl<FluentTag>& element);
template std::ostream& operator<<(std::ostream& out, const LiteralImpl<DerivedTag>& element);

template<IsStaticOrFluentOrDerivedTag P>
std::ostream& operator<<(std::ostream& out, Literal<P> element)
{
    write(*element, AddressFormatter(), out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, Literal<StaticTag> element);
template std::ostream& operator<<(std::ostream& out, Literal<FluentTag> element);
template std::ostream& operator<<(std::ostream& out, Literal<DerivedTag> element);

}
