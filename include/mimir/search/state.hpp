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
#include "mimir/search/dense_state.hpp"

#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>
#include <memory>
#include <valla/double_tree_compression.hpp>
#include <valla/indexed_hash_set.hpp>
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
    Index m_fluent_atoms_index;
    Index m_fluent_atoms_size;
    Index m_derived_atoms_index;
    Index m_derived_atoms_size;
    Index m_numeric_variables_index;
    Index m_numeric_variables_size;

    InternalStateImpl(v::RootSlotType fluent_atoms, v::RootSlotType derived_atoms, v::RootSlotType numeric_variables);

    friend class StateRepositoryImpl;

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    /**
     * Getters
     */

    template<formalism::IsFluentOrDerivedTag P>
    v::RootSlotType get_atoms() const;
    v::RootSlotType get_numeric_variables() const;
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
    Index m_index;

    SharedMemoryPoolPtr<DenseState> m_dense_state;

    State(Index index, const InternalStateImpl& internal, SharedMemoryPoolPtr<DenseState> dense_state, const formalism::ProblemImpl& problem);

    friend class StateRepositoryImpl;

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

    InternalState get_internal() const;
    const formalism::ProblemImpl& get_problem() const;

    Index get_index() const;
    template<formalism::IsFluentOrDerivedTag P>
    const FlatBitset& get_atoms() const;
    const FlatDoubleList& get_numeric_variables() const;

    template<formalism::IsFluentOrDerivedTag P>
    auto get_atoms_range() const;
    auto get_numeric_variables_range() const;

    const DenseState& get_dense_state() const;

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
auto State::get_atoms_range() const
{
    return std::ranges::subrange(v::begin(m_internal->get_atoms<P>(), m_problem->get_tree_table()), v::end());
}

inline auto State::get_numeric_variables_range() const
{
    return std::ranges::subrange(valla::doubles::plain::begin(m_internal->get_numeric_variables(), m_problem->get_tree_table(), m_problem->get_double_table()),
                                 valla::doubles::plain::end());
}

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
template<>
struct Hash<mimir::search::InternalStateImpl>
{
    size_t operator()(const mimir::search::InternalStateImpl& el) const
    {
        static_assert(std::is_standard_layout_v<mimir::search::InternalStateImpl>, "InternalStateImpl must be standard layout");

        size_t seed = 0;
        size_t hash[2] = { 0, 0 };

        loki::MurmurHash3_x64_128(reinterpret_cast<const uint8_t*>(&el), sizeof(mimir::search::InternalStateImpl), seed, hash);

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
        static_assert(std::is_standard_layout_v<mimir::search::InternalStateImpl>, "InternalStateImpl must be standard layout");

        const auto lhs_begin = reinterpret_cast<const uint8_t*>(&lhs);
        const auto rhs_begin = reinterpret_cast<const uint8_t*>(&rhs);

        return std::equal(lhs_begin, lhs_begin + sizeof(mimir::search::InternalStateImpl), rhs_begin);
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
using InternalStateImplMap = absl::node_hash_map<InternalStateImpl, Index, loki::Hash<InternalStateImpl>, loki::EqualTo<InternalStateImpl>>;

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
