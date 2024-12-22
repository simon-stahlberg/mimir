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

#ifndef MIMIR_SEARCH_DENSE_STATE_HPP_
#define MIMIR_SEARCH_DENSE_STATE_HPP_

#include "cista/serialization.h"
#include "mimir/common/printers.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir
{

/// @brief `DenseState` is an intermediate representation of a `StateImpl`
struct DenseState
{
    Index m_index = Index(0);
    FlatBitset m_fluent_atoms = FlatBitset();
    FlatBitset m_derived_atoms = FlatBitset();

    DenseState() = default;
    DenseState(State state);

    static void translate(State state, DenseState& out_state);

    template<DynamicPredicateTag P>
    bool contains(GroundAtom<P> atom) const;

    template<DynamicPredicateTag P>
    bool literal_holds(GroundLiteral<P> literal) const;

    template<DynamicPredicateTag P>
    bool literals_hold(const GroundLiteralList<P>& literals) const;

    template<DynamicPredicateTag P>
    bool literals_hold(const FlatIndexList& positive_atoms, const FlatIndexList& negative_atoms) const;

    /* Immutable Getters */

    Index get_index() const;

    template<DynamicPredicateTag P>
    const FlatBitset& get_atoms() const;

    Index& get_index();

    template<DynamicPredicateTag P>
    FlatBitset& get_atoms();
};

}

#endif
