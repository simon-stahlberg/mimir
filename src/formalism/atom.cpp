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
template<PredicateTag P>
AtomImpl<P>::AtomImpl(Index index, Predicate<P> predicate, TermList terms) : m_index(index), m_predicate(std::move(predicate)), m_terms(std::move(terms))
{
}

template<PredicateTag P>
Index AtomImpl<P>::get_index() const
{
    return m_index;
}

template<PredicateTag P>
Predicate<P> AtomImpl<P>::get_predicate() const
{
    return m_predicate;
}

template<PredicateTag P>
const TermList& AtomImpl<P>::get_terms() const
{
    return m_terms;
}

template<PredicateTag P>
VariableList AtomImpl<P>::get_variables() const
{
    VariableList variables;
    for (const auto& term : get_terms())
    {
        if (const auto variable = std::get_if<Variable>(&term->get_variant()))
        {
            if (!std::count(variables.begin(), variables.end(), *variable))
            {
                variables.emplace_back(*variable);
            }
        }
    }
    return variables;
}

template<PredicateTag P>
size_t AtomImpl<P>::get_arity() const
{
    return m_terms.size();
}

template class AtomImpl<Static>;
template class AtomImpl<Fluent>;
template class AtomImpl<Derived>;

template<PredicateTag P>
std::ostream& operator<<(std::ostream& out, const AtomImpl<P>& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, const AtomImpl<Static>& element);
template std::ostream& operator<<(std::ostream& out, const AtomImpl<Fluent>& element);
template std::ostream& operator<<(std::ostream& out, const AtomImpl<Derived>& element);

template<PredicateTag P>
std::ostream& operator<<(std::ostream& out, Atom<P> element)
{
    out << *element;
    return out;
}

template std::ostream& operator<<(std::ostream& out, Atom<Static> element);
template std::ostream& operator<<(std::ostream& out, Atom<Fluent> element);
template std::ostream& operator<<(std::ostream& out, Atom<Derived> element);

}
