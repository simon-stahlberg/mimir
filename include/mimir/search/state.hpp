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

#ifndef MIMIR_SEARCH_STATE_HPP_
#define MIMIR_SEARCH_STATE_HPP_

#include "cista/containers/dynamic_bitset.h"
#include "cista/containers/tuple.h"
#include "cista/serialization.h"
#include "cista/storage/unordered_set.h"
#include "mimir/common/printers.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"

#include <ostream>
#include <tuple>
#include <unordered_map>

namespace mimir
{

/// @brief `StateImpl` encapsulates the fluent and derived atoms of a planning state.
/// We refer to the fluent atoms as the non-extended state
/// and the fluent and derived atoms as the extended state.
struct StateImpl
{
    Index m_index = Index(0);
    FlatBitset m_fluent_atoms = FlatBitset();
    FlatBitset m_derived_atoms = FlatBitset();

    template<DynamicPredicateTag P>
    bool contains(GroundAtom<P> atom) const;

    template<DynamicPredicateTag P>
    bool superset_of(const GroundAtomList<P>& atoms) const;

    template<DynamicPredicateTag P>
    bool literal_holds(GroundLiteral<P> literal) const;

    template<DynamicPredicateTag P>
    bool literals_hold(const GroundLiteralList<P>& literals) const;

    /* Getters */

    Index& get_index();

    Index get_index() const;

    template<DynamicPredicateTag P>
    FlatBitset& get_atoms();

    template<DynamicPredicateTag P>
    const FlatBitset& get_atoms() const;
};

}

// Only hash/compare the non-extended portion of a state, and the problem.
// The extended portion is always equal for the same non-extended portion.
// We use it for the unique state construction in the `StateRepository`.
template<>
struct cista::storage::DerefStdHasher<mimir::StateImpl>
{
    size_t operator()(const mimir::StateImpl* ptr) const;
};
template<>
struct cista::storage::DerefStdEqualTo<mimir::StateImpl>
{
    bool operator()(const mimir::StateImpl* lhs, const mimir::StateImpl* rhs) const;
};

namespace mimir
{

using StateImplSet = cista::storage::UnorderedSet<StateImpl>;

/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<Problem, State, const PDDLRepositories&>& data);
}

#endif
