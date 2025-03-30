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

namespace mimir::formalism
{

template<IsStaticOrFluentOrDerivedTag P>
class LiteralImpl
{
private:
    Index m_index;
    bool m_polarity;
    Atom<P> m_atom;

    // Below: add additional members if needed and initialize them in the constructor

    LiteralImpl(Index index, bool polarity, Atom<P> atom);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    using FormalismEntity = void;
    using Type = P;

    // moveable but not copyable
    LiteralImpl(const LiteralImpl& other) = delete;
    LiteralImpl& operator=(const LiteralImpl& other) = delete;
    LiteralImpl(LiteralImpl&& other) = default;
    LiteralImpl& operator=(LiteralImpl&& other) = default;

    Index get_index() const;
    bool get_polarity() const;
    Atom<P> get_atom() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_polarity(), get_atom()); }
};

template<IsStaticOrFluentOrDerivedTag P>
extern std::ostream& operator<<(std::ostream& out, const LiteralImpl<P>& element);

template<IsStaticOrFluentOrDerivedTag P>
extern std::ostream& operator<<(std::ostream& out, Literal<P> element);

}

#endif
