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

namespace mimir
{

template<PredicateCategory P>
LiteralImpl<P>::LiteralImpl(int identifier, bool is_negated, Atom<P> atom) :
    loki::Base<LiteralImpl<P>>(identifier),
    m_is_negated(is_negated),
    m_atom(std::move(atom))
{
}

template<PredicateCategory P>
bool LiteralImpl<P>::is_structurally_equivalent_to_impl(const LiteralImpl<P>& other) const
{
    if (this != &other)
    {
        return (m_is_negated == other.m_is_negated) && (m_atom == other.m_atom);
    }
    return true;
}

template<PredicateCategory P>
size_t LiteralImpl<P>::hash_impl() const
{
    return loki::hash_combine(m_is_negated, m_atom);
}

template<PredicateCategory P>
void LiteralImpl<P>::str_impl(std::ostream& out, const loki::FormattingOptions& options) const
{
    if (m_is_negated)
    {
        out << "(not ";
        m_atom->str(out, options);
        out << ")";
    }
    else
    {
        m_atom->str(out, options);
    }
}

template<PredicateCategory P>
bool LiteralImpl<P>::is_negated() const
{
    return m_is_negated;
}

template<PredicateCategory P>
const Atom<P>& LiteralImpl<P>::get_atom() const
{
    return m_atom;
}

template class LiteralImpl<Static>;
template class LiteralImpl<Fluent>;
template class LiteralImpl<Derived>;

}
