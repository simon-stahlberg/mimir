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

namespace mimir
{

template<PredicateCategory P>
class GroundLiteralImpl
{
private:
    size_t m_index;
    bool m_is_negated;
    GroundAtom<P> m_atom;

    // Below: add additional members if needed and initialize them in the constructor

    GroundLiteralImpl(size_t index, bool is_negated, GroundAtom<P> atom);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    using Category = P;

    // moveable but not copyable
    GroundLiteralImpl(const GroundLiteralImpl& other) = delete;
    GroundLiteralImpl& operator=(const GroundLiteralImpl& other) = delete;
    GroundLiteralImpl(GroundLiteralImpl&& other) = default;
    GroundLiteralImpl& operator=(GroundLiteralImpl&& other) = default;

    std::string str() const;

    size_t get_index() const;
    bool is_negated() const;
    const GroundAtom<P>& get_atom() const;
};

template<PredicateCategory P>
extern std::ostream& operator<<(std::ostream& out, const GroundLiteralImpl<P>& element);
}

#endif
