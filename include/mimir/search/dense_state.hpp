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

namespace mimir::search
{

/// @brief `DenseState` is an intermediate representation of a `StateImpl`
struct DenseState
{
    Index m_index = Index(0);
    FlatBitset m_fluent_atoms = FlatBitset();
    FlatBitset m_derived_atoms = FlatBitset();
    FlatDoubleList m_numeric_variables = FlatDoubleList();

    DenseState() = default;
    DenseState(State state);

    static void translate(State state, DenseState& out_state);

    template<formalism::IsFluentOrDerivedTag P>
    bool contains(formalism::GroundAtom<P> atom) const;

    template<formalism::IsFluentOrDerivedTag P>
    bool literal_holds(formalism::GroundLiteral<P> literal) const;

    template<formalism::IsFluentOrDerivedTag P>
    bool literals_hold(const formalism::GroundLiteralList<P>& literals) const;

    template<formalism::IsFluentOrDerivedTag P>
    bool literals_hold(const FlatIndexList& positive_atoms, const FlatIndexList& negative_atoms) const;

    /* Immutable Getters */

    Index get_index() const;

    template<formalism::IsFluentOrDerivedTag P>
    const FlatBitset& get_atoms() const;

    const FlatDoubleList& get_numeric_variables() const;

    Index& get_index();

    template<formalism::IsFluentOrDerivedTag P>
    FlatBitset& get_atoms();

    FlatDoubleList& get_numeric_variables();
};

}

#endif
