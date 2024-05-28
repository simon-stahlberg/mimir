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

#ifndef MIMIR_FORMALISM_ATOM_HPP_
#define MIMIR_FORMALISM_ATOM_HPP_

#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/term.hpp"

#include <loki/loki.hpp>
#include <string>

namespace mimir
{
template<PredicateCategory P>
class AtomImpl : public loki::Base<AtomImpl<P>>
{
private:
    Predicate<P> m_predicate;
    TermList m_terms;

    // Below: add additional members if needed and initialize them in the constructor

    AtomImpl(int identifier, Predicate<P> predicate, TermList terms);

    // Give access to the constructor.
    friend class loki::PDDLFactory<AtomImpl, loki::Hash<AtomImpl*>, loki::EqualTo<AtomImpl*>>;

    /// @brief Test for semantic equivalence
    bool is_structurally_equivalent_to_impl(const AtomImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<AtomImpl>;

public:
    using Category = P;

    Predicate<P> get_predicate() const;
    const TermList& get_terms() const;
    size_t get_arity() const;
};

/**
 * Type aliases
 */

template<PredicateCategory P>
using Atom = const AtomImpl<P>*;
template<PredicateCategory P>
using AtomList = std::vector<Atom<P>>;

/**
 * Implementation details
 */
template<PredicateCategory P>
AtomImpl<P>::AtomImpl(int identifier, Predicate<P> predicate, TermList terms) :
    loki::Base<AtomImpl<P>>(identifier),
    m_predicate(std::move(predicate)),
    m_terms(std::move(terms))
{
}

template<PredicateCategory P>
bool AtomImpl<P>::is_structurally_equivalent_to_impl(const AtomImpl& other) const
{
    if (this != &other)
    {
        return (m_predicate == other.m_predicate) && (m_terms == other.m_terms);
    }
    return true;
}

template<PredicateCategory P>
size_t AtomImpl<P>::hash_impl() const
{
    return loki::hash_combine(m_predicate, loki::hash_container(m_terms));
}

template<PredicateCategory P>
void AtomImpl<P>::str_impl(std::ostream& out, const loki::FormattingOptions& options) const
{
    out << "(" << m_predicate->get_name();
    for (size_t i = 0; i < m_terms.size(); ++i)
    {
        out << " ";
        std::visit(loki::StringifyVisitor(out, options), *m_terms[i]);
    }
    out << ")";
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
}

#endif
