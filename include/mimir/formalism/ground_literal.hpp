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

#include "mimir/formalism/declarations.hpp"

namespace mimir::formalism
{

template<IsStaticOrFluentOrDerivedTag P>
class GroundLiteralImpl
{
private:
    Index m_index;
    bool m_polarity;
    GroundAtom<P> m_atom;

    // Below: add additional members if needed and initialize them in the constructor

    GroundLiteralImpl(Index index, bool polarity, GroundAtom<P> atom);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    using FormalismEntity = void;
    using Type = P;

    // moveable but not copyable
    GroundLiteralImpl(const GroundLiteralImpl& other) = delete;
    GroundLiteralImpl& operator=(const GroundLiteralImpl& other) = delete;
    GroundLiteralImpl(GroundLiteralImpl&& other) = default;
    GroundLiteralImpl& operator=(GroundLiteralImpl&& other) = default;

    Index get_index() const;
    bool get_polarity() const;
    GroundAtom<P> get_atom() const;
    Literal<P> lift(const TermList& terms, Repositories& pddl_repositories) const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_polarity(), get_atom()); }
};

template<IsStaticOrFluentOrDerivedTag P>
extern std::pair<VariableList, LiteralList<P>> lift(const GroundLiteralList<P>& ground_literals, Repositories& pddl_repositories);

template<IsStaticOrFluentOrDerivedTag P>
extern std::ostream& operator<<(std::ostream& out, const GroundLiteralImpl<P>& element);

template<IsStaticOrFluentOrDerivedTag P>
extern std::ostream& operator<<(std::ostream& out, GroundLiteral<P> element);
}

#endif
