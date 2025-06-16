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

#include "mimir/buffering/builder.hpp"
#include "mimir/buffering/unordered_set.hpp"
#include "mimir/buffering/utils.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/declarations.hpp"

#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>
#include <memory>
#include <valla/canonical_delta_tree_compression.hpp>
#include <valla/canonical_tree_compression.hpp>
#include <valla/delta_tree_compression.hpp>
#include <valla/indexed_hash_set.hpp>
#include <valla/root_slot.hpp>
#include <valla/tree_compression.hpp>

namespace mimir::search
{
namespace v = valla::plain;

/**
 * InternalState
 */

/// @brief `InternalStateImpl` encapsulates the fluent and derived atoms, and numeric variables of a planning state.
class InternalStateImpl
{
private:
    Index m_index;
    Index m_fluent_atoms_index;
    Index m_fluent_atoms_size;
    Index m_derived_atoms_index;
    Index m_derived_atoms_size;
    Index m_numeric_variables;

    InternalStateImpl(Index index, v::RootSlotType fluent_atoms, v::RootSlotType derived_atoms, Index numeric_variables);

    friend class StateRepositoryImpl;

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    /**
     * Getters
     */

    Index get_index() const;
    template<formalism::IsFluentOrDerivedTag P>
    v::RootSlotType get_atoms() const;
    Index get_numeric_variables() const;
};

static_assert(sizeof(InternalStateImpl) == 24);

/**
 * State
 */

class State
{
private:
    InternalState m_internal;
    const formalism::ProblemImpl* m_problem;

public:
    State(InternalState internal, const formalism::ProblemImpl& problem);
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

    InternalState get_internal() const;
    const formalism::ProblemImpl& get_problem() const;

    Index get_index() const;
    template<formalism::IsFluentOrDerivedTag P>
    auto get_atoms() const;
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

template<formalism::IsFluentOrDerivedTag P>
auto State::get_atoms() const
{
    return std::ranges::subrange(v::begin(m_internal->get_atoms<P>(), m_problem->get_tree_table()), v::end());
}

template<formalism::IsFluentOrDerivedTag P, std::ranges::input_range Range1, std::ranges::input_range Range2>
    requires IsRangeOver<Range1, Index> && IsRangeOver<Range2, Index>
bool State::literals_hold(const Range1& positive_atoms, const Range2& negative_atoms) const
{
    return is_supseteq(get_atoms<P>(), positive_atoms) && are_disjoint(get_atoms<P>(), negative_atoms);
}

/**
 * Pretty printing
 */

std::ostream& operator<<(std::ostream& os, const search::State& state);

}

namespace loki
{
template<>
struct Hash<mimir::search::InternalStateImpl>
{
    size_t operator()(const mimir::search::InternalStateImpl& el) const
    {
        static_assert(std::is_standard_layout_v<mimir::search::InternalStateImpl>, "InternalStateImpl must be standard layout");
        static_assert(sizeof(mimir::search::InternalStateImpl) == sizeof(mimir::Index) * 6, "Unexpected padding in InternalStateImpl");

        size_t seed = 0;
        size_t hash[2] = { 0, 0 };

        loki::MurmurHash3_x64_128(reinterpret_cast<const uint8_t*>(&el) + sizeof(mimir::Index), sizeof(mimir::Index) * 5, seed, hash);

        loki::hash_combine(seed, hash[0]);
        loki::hash_combine(seed, hash[1]);

        return seed;
    }
};

template<>
struct EqualTo<mimir::search::InternalStateImpl>
{
    bool operator()(const mimir::search::InternalStateImpl& lhs, const mimir::search::InternalStateImpl& rhs) const
    {
        return lhs.get_atoms<mimir::formalism::FluentTag>() == rhs.get_atoms<mimir::formalism::FluentTag>()
               && lhs.get_numeric_variables() == rhs.get_numeric_variables();
    }
};

template<>
struct Hash<mimir::search::State>
{
    size_t operator()(const mimir::search::State& el) const
    {
        return hash_combine(Hash<mimir::search::InternalStateImpl> {}(*el.get_internal()), &el.get_problem());
    }
};

template<>
struct EqualTo<mimir::search::State>
{
    bool operator()(const mimir::search::State& lhs, const mimir::search::State& rhs) const
    {
        return EqualTo<mimir::search::InternalStateImpl> {}(*lhs.get_internal(), *rhs.get_internal()) && &lhs.get_problem() == &rhs.get_problem();
    }
};
}

namespace mimir::search
{
using InternalStateImplSet = absl::node_hash_set<InternalStateImpl, loki::Hash<InternalStateImpl>, loki::EqualTo<InternalStateImpl>>;

using StateList = std::vector<State>;
using StateSet = UnorderedSet<State>;
template<typename T>
using StateMap = UnorderedMap<State, T>;
using StateProblem = std::pair<State, formalism::Problem>;
using StateProblemPair = std::pair<StateProblem, StateProblem>;
using StateProblemList = std::vector<StateProblem>;
using StateProblemPairList = std::vector<StateProblemPair>;
}

#endif
