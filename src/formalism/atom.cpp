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

#include "mimir/formalism/atom.hpp"

#include "formatter.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/term.hpp"

namespace mimir
{
template<PredicateCategory P>
AtomImpl<P>::AtomImpl(Index index, Predicate<P> predicate, TermList terms) : m_index(index), m_predicate(std::move(predicate)), m_terms(std::move(terms))
{
}

template<PredicateCategory P>
std::string AtomImpl<P>::str() const
{
    auto out = std::stringstream();
    out << *this;
    return out.str();
}

template<PredicateCategory P>
Index AtomImpl<P>::get_index() const
{
    return m_index;
}

template<PredicateCategory P>
Predicate<P> AtomImpl<P>::get_predicate() const
{
    return m_predicate;
}

template<PredicateCategory P>
const TermList& AtomImpl<P>::get_terms() const
{
    return m_terms;
}

template<PredicateCategory P>
size_t AtomImpl<P>::get_arity() const
{
    return m_terms.size();
}

template class AtomImpl<Static>;
template class AtomImpl<Fluent>;
template class AtomImpl<Derived>;

template<PredicateCategory P>
std::ostream& operator<<(std::ostream& out, const AtomImpl<P>& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, const AtomImpl<Static>& element);
template std::ostream& operator<<(std::ostream& out, const AtomImpl<Fluent>& element);
template std::ostream& operator<<(std::ostream& out, const AtomImpl<Derived>& element);

}
