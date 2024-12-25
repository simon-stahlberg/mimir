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

#include "mimir/buffering/unordered_set.h"
#include "mimir/common/hash_cista.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"

#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>

namespace mimir
{
/// @brief `StateImpl` encapsulates the fluent and derived atoms of a planning state.
/// We refer to the fluent atoms as the non-extended state
/// and the fluent and derived atoms as the extended state.
struct StateImpl
{
    Index m_index = Index(0);
    FlatIndexList m_fluent_atoms = FlatIndexList();
    uintptr_t m_derived_atoms = uintptr_t(0);

    static const FlatIndexList s_empty_derived_atoms;  ///< Returned, if m_derived_atoms is a nullptr.

    /// @brief log(N) operation, ideally, we get rid of it, perhaps useful to expose to python users
    template<DynamicPredicateTag P>
    bool literal_holds(GroundLiteral<P> literal) const;

    /// @brief N*log(N) operation, ideally (currently unused), perhaps useful to expose to python users
    template<DynamicPredicateTag P>
    bool literals_hold(const GroundLiteralList<P>& literals) const;

    template<DynamicPredicateTag P>
    bool literals_hold(const FlatIndexList& positive_atoms, const FlatIndexList& negative_atoms) const;

    /* Immutable Getters */

    Index get_index() const;

    template<DynamicPredicateTag P>
    const FlatIndexList& get_atoms() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    ///
    /// Only return the non-extended portion of a state because it implies the extended portion.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifiable_members() const { return std::forward_as_tuple(std::as_const(get_atoms<Fluent>())); }

private:
    /* Mutable Getters */

    friend class StateRepository;  ///< Given exclusive write access to a state.

    Index& get_index();

    FlatIndexList& get_fluent_atoms();
    uintptr_t& get_derived_atoms();
};

/// @brief STL does not define operator== for std::span.
inline bool operator==(const std::span<const State>& lhs, const std::span<const State>& rhs)
{
    return (lhs.data() == rhs.data()) && (lhs.size() == rhs.size());
}

using StateImplSet = mimir::buffering::UnorderedSet<StateImpl>;
using AxiomEvaluationSet = mimir::buffering::UnorderedSet<FlatIndexList>;

/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<Problem, State, const PDDLRepositories&>& data);
}

#endif
