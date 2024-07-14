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

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/flat_types.hpp"

#include <flatmemory/flatmemory.hpp>
#include <ostream>
#include <tuple>
#include <unordered_map>

namespace mimir
{

using StateId = uint32_t;
using StateIdList = std::vector<StateId>;
using StateIdSet = std::unordered_set<StateId>;

// StateIndex is a stronger notion of a StateId,
// indicating that there is an underlying indexing scheme 0,1,2,...
using StateIndex = uint32_t;
using StateIndexList = std::vector<StateIndex>;
using StateIndexSet = std::unordered_set<StateIndex>;

/**
 * Flatmemory types
 */
using FlatStateLayout = flatmemory::Tuple<StateId, FlatBitsetLayout<Fluent>, FlatBitsetLayout<Derived>>;
using FlatStateBuilder = flatmemory::Builder<FlatStateLayout>;
using FlatState = flatmemory::ConstView<FlatStateLayout>;

// Only hash/compare the non-extended portion of a state, and the problem.
// The extended portion is computed automatically, when calling ssg.create_state(...)
struct FlatStateHash
{
    size_t operator()(FlatState view) const;
};

struct FlatStateEqual
{
    bool operator()(FlatState view_left, FlatState view_right) const;
};

using FlatStateSet = flatmemory::UnorderedSet<FlatStateLayout, FlatStateHash, FlatStateEqual>;
using FlatStateVector = flatmemory::FixedSizedTypeVector<FlatStateLayout>;

/**
 * StateBuilder
 */
class StateBuilder
{
private:
    FlatStateBuilder m_builder;

public:
    FlatStateBuilder& get_flatmemory_builder();
    const FlatStateBuilder& get_flatmemory_builder() const;

    StateId& get_id();

    template<DynamicPredicateCategory P>
    FlatBitsetBuilder<P>& get_atoms();
};

/**
 * State
 */
class State
{
private:
    FlatState m_view;

public:
    explicit State(FlatState view);

    /// @brief Return true iff two states are equal.
    ///
    /// For states in same SuccessorStateGenerator, we know they are already unique.
    /// Hence, comparison of the buffer pointer suffices.
    /// For states in different SuccessorStateGenerator, buffer pointers are always different.
    /// Hence, comparison always returns false.
    bool operator==(State other) const;

    /// @brief Return a hash value for the state.
    ///
    /// Same argument from operator== applies.
    size_t hash() const;

    StateId get_id() const;

    template<DynamicPredicateCategory P>
    bool contains(GroundAtom<P> atom) const;

    template<DynamicPredicateCategory P>
    bool superset_of(const GroundAtomList<P>& atoms) const;

    template<DynamicPredicateCategory P>
    bool literal_holds(GroundLiteral<P> literal) const;

    template<DynamicPredicateCategory P>
    bool literals_hold(const GroundLiteralList<P>& literals) const;

    template<DynamicPredicateCategory P>
    FlatBitset<P> get_atoms() const;
};

/**
 * Mimir types
 */

using StateList = std::vector<State>;

struct StateHash
{
    size_t operator()(State view) const;
};

template<typename T>
using StateMap = std::unordered_map<State, T, StateHash>;
using StateSet = std::unordered_set<State, StateHash>;

/**
 * Pretty printing
 */

extern std::ostream& operator<<(std::ostream& os, const std::tuple<Problem, State, const PDDLFactories&>& data);
}

#endif
