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

#include "mimir/common/hash.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/state_packed.hpp"
#include "mimir/search/state_unpacked.hpp"

#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>
#include <memory>

namespace mimir::search
{

class State
{
private:
    StateRepository m_parent;
    PackedState m_packed;
    UnpackedState m_unpacked;  ///< we could remove it fall back to it returning iterators
    Index m_index;

    State(Index index, PackedState packed, UnpackedState unpacked, StateRepository parent);

    friend class StateRepositoryImpl;  ///< Delegates construction from PackedState.

public:
    State(const State&) = default;
    State(State&&) noexcept = default;
    State& operator=(const State&) = default;
    State& operator=(State&&) noexcept = default;
    ~State() = default;

    bool operator==(const State& other) const noexcept;
    bool operator!=(const State& other) const noexcept;

    /**
     * Getters
     */

    Index get_index() const;
    const StateRepository& get_state_repository() const;
    PackedState get_packed_state() const;
    const UnpackedStateImpl& get_unpacked_state() const;
    const formalism::ProblemImpl& get_problem() const;

    template<formalism::IsFluentOrDerivedTag P>
    const FlatBitset& get_atoms() const;
    const FlatDoubleList& get_numeric_variables() const;

    /**
     * Utils
     */

    /// @brief Check whether the literal holds in the state using binary search over the atoms in the state.
    /// @tparam P is the literal type.
    /// @param literal is the literal.
    /// @return true if the literal holds in the state, and false otherwise.
    template<formalism::IsFluentOrDerivedTag P>
    bool literal_holds(formalism::GroundLiteral<P> literal) const;

    /// @brief Check whether all literals hold in the state using binary searches over the atoms in the state.
    /// @tparam P is the literal type.
    /// @param literals are the literals.
    /// @return true if all literals hold in the state, and false otherwise.
    template<formalism::IsFluentOrDerivedTag P>
    bool literals_hold(const formalism::GroundLiteralList<P>& literals) const;

    bool numeric_constraint_holds(formalism::GroundNumericConstraint numeric_constraint, const FlatDoubleList& static_numeric_variables) const;

    bool numeric_constraints_hold(const formalism::GroundNumericConstraintList& numeric_constraints, const FlatDoubleList& static_numeric_variables) const;

    template<formalism::IsFluentOrDerivedTag P, std::ranges::input_range Range1, std::ranges::input_range Range2>
        requires IsRangeOver<Range1, Index> && IsRangeOver<Range2, Index>
    bool literals_hold(const Range1& positive_atoms, const Range2& negative_atoms) const;
};

/**
 * Implementations
 */

template<formalism::IsFluentOrDerivedTag P, std::ranges::input_range Range1, std::ranges::input_range Range2>
    requires IsRangeOver<Range1, Index> && IsRangeOver<Range2, Index>
bool State::literals_hold(const Range1& positive_atoms, const Range2& negative_atoms) const
{
    auto atoms = get_atoms<P>();
    return is_supseteq(atoms, positive_atoms) && are_disjoint(atoms, negative_atoms);
}

/**
 * Pretty printing
 */

std::ostream& operator<<(std::ostream& os, const search::State& state);

}

namespace loki
{
/// @private
template<>
struct Hash<mimir::search::State>
{
    size_t operator()(const mimir::search::State& el) const;
};

/// @private
template<>
struct EqualTo<mimir::search::State>
{
    bool operator()(const mimir::search::State& lhs, const mimir::search::State& rhs) const;
};
}

namespace mimir::search
{
using PackedStateImplMap = absl::node_hash_map<PackedStateImpl, Index, loki::Hash<PackedStateImpl>, loki::EqualTo<PackedStateImpl>>;

using StateList = std::vector<State>;
using StateSet = UnorderedSet<State>;
using StatePair = std::pair<State, State>;
using StatePairList = std::vector<StatePair>;
}

#endif
