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

#ifndef MIMIR_FORMALISM_GROUND_LITERAL_HPP_
#define MIMIR_FORMALISM_GROUND_LITERAL_HPP_

#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/predicate.hpp"

#include <loki/loki.hpp>
#include <string>

namespace mimir
{
/*
    TODO: Flattening GroundLiteralImpl is unnecessary. It is already flat.
*/
template<PredicateCategory P>
class GroundLiteralImpl : public loki::Base<GroundLiteralImpl<P>>
{
private:
    bool m_is_negated;
    GroundAtom<P> m_atom;

    // Below: add additional members if needed and initialize them in the constructor

    GroundLiteralImpl(int identifier, bool is_negated, GroundAtom<P> atom);

    // Give access to the constructor.
    friend class loki::PDDLFactory<GroundLiteralImpl, loki::Hash<GroundLiteralImpl*>, loki::EqualTo<GroundLiteralImpl*>>;

    /// @brief Test for semantic equivalence
    bool is_structurally_equivalent_to_impl(const GroundLiteralImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<GroundLiteralImpl>;

public:
    using Category = P;

    bool is_negated() const;
    const GroundAtom<P>& get_atom() const;
};

/**
 * Type aliases
 */

template<PredicateCategory P>
using GroundLiteral = const GroundLiteralImpl<P>*;
template<PredicateCategory P>
using GroundLiteralList = std::vector<GroundLiteral<P>>;
template<PredicateCategory P>
using GroundLiteralSet = std::unordered_set<GroundLiteral<P>>;

/**
 * Implementation details
 */

template<PredicateCategory P>
GroundLiteralImpl<P>::GroundLiteralImpl(int identifier, bool is_negated, GroundAtom<P> atom) :
    loki::Base<GroundLiteralImpl<P>>(identifier),
    m_is_negated(is_negated),
    m_atom(std::move(atom))
{
}

template<PredicateCategory P>
bool GroundLiteralImpl<P>::is_structurally_equivalent_to_impl(const GroundLiteralImpl<P>& other) const
{
    if (this != &other)
    {
        return (m_is_negated == other.m_is_negated) && (m_atom == other.m_atom);
    }
    return true;
}

template<PredicateCategory P>
size_t GroundLiteralImpl<P>::hash_impl() const
{
    return loki::hash_combine(m_is_negated, m_atom);
}

template<PredicateCategory P>
void GroundLiteralImpl<P>::str_impl(std::ostream& out, const loki::FormattingOptions& options) const
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
bool GroundLiteralImpl<P>::is_negated() const
{
    return m_is_negated;
}

template<PredicateCategory P>
const GroundAtom<P>& GroundLiteralImpl<P>::get_atom() const
{
    return m_atom;
}
}

#endif
