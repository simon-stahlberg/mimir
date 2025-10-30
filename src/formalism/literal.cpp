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
}
