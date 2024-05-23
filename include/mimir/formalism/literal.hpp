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

#ifndef MIMIR_FORMALISM_LITERAL_HPP_
#define MIMIR_FORMALISM_LITERAL_HPP_

#include "mimir/formalism/declarations.hpp"

#include <loki/loki.hpp>
#include <string>

namespace mimir
{
/*
    TODO: Flattening LiteralImpl is unnecessary. It is already flat.
*/
template<IsPredicate P>
class LiteralImpl : public loki::Base<LiteralImpl>
{
private:
    bool m_is_negated;
    Atom<P> m_atom;

    // Below: add additional members if needed and initialize them in the constructor

    LiteralImpl(int identifier, bool is_negated, Atom<P> atom);

    // Give access to the constructor.
    friend class loki::PDDLFactory<LiteralImpl, loki::Hash<LiteralImpl*>, loki::EqualTo<LiteralImpl*>>;

    /// @brief Test for semantic equivalence
    bool is_structurally_equivalent_to_impl(const LiteralImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<LiteralImpl>;

public:
    bool is_negated() const;
    const Atom<P>& get_atom() const;
};

template<IsPredicate P>
LiteralImpl<P>::LiteralImpl(int identifier, bool is_negated, Atom<P> atom) : Base(identifier), m_is_negated(is_negated), m_atom(std::move(atom))
{
}

template<IsPredicate P>
bool LiteralImpl<P>::is_structurally_equivalent_to_impl(const LiteralImpl<P>& other) const
{
    if (this != &other)
    {
        return (m_is_negated == other.m_is_negated) && (m_atom == other.m_atom);
    }
    return true;
}

template<IsPredicate P>
size_t LiteralImpl<P>::hash_impl() const
{
    return loki::hash_combine(m_is_negated, m_atom);
}

template<IsPredicate P>
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

template<IsPredicate P>
bool LiteralImpl<P>::is_negated() const
{
    return m_is_negated;
}

template<IsPredicate P>
const Atom<P>& LiteralImpl<P>::get_atom() const
{
    return m_atom;
}

}

#endif
