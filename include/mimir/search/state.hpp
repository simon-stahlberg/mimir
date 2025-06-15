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

namespace mimir::buffering
{

namespace v = valla::plain;

// The format of the data as follows:
// - num bits b1 for state index (6 bit)
// - state index (b1 bits)
// - num bits b2 for fluent tree node index
// - fluent tree node index (b2 bits)
// - num bits b3 for fluent tree node size
// - fluent tree node size (b3 bits)
// - num bits b4 for derived tree node index
// - derived tree node index (b4 bits)
// - num bits b5 for derived tree node size
// - derived tree node size (b5 bits)
// - num bits b6 for numeric variables list index
// - numeric variables list index (b6 bits)
struct PackedState
{
};

template<>
struct Data<PackedState>
{
    Index index;  // complete 4 byte range for index for byte aligned hashing and comparison of the remainder.
    VarUint<Index> fluent_atoms_index;
    VarUint<Index> fluent_atoms_size;
    VarUint<Index> derived_atoms_index;
    VarUint<Index> derived_atoms_size;
    VarUint<Index> numeric_variables;

    template<formalism::IsFluentOrDerivedTag P>
    v::RootSlotType get_atoms() const;
};

template<>
class Builder<PackedState>
{
private:
    Data<PackedState> m_data;

    StreamBufferWriter m_buffer;

    void serialize();

public:
    Builder() = default;

    void serialize(Data<PackedState> data);

    const StreamBufferWriter& get_buffer_writer() const;
};

template<>
class View<PackedState>
{
private:
    const uint8_t* m_buffer;

public:
    View(const uint8_t* buffer);

    View(const Builder<PackedState>& builder);

    Data<PackedState> deserialize() const;

    const uint8_t* get_buffer() const;
    size_t get_buffer_size() const;
};

using PackedStateData = Data<PackedState>;
using PackedStateBuilder = Builder<PackedState>;
using PackedStateView = View<PackedState>;
using PackedStateSet = UnorderedSet<PackedState>;
}

namespace mimir::search
{
namespace v = valla::plain;

/**
 * State
 */

class State
{
private:
    buffering::View<buffering::PackedState> m_packed;
    const formalism::ProblemImpl* m_problem;

    buffering::Data<buffering::PackedState> m_unpacked;

public:
    State(buffering::View<buffering::PackedState> packed, const formalism::ProblemImpl& problem);
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

    buffering::View<buffering::PackedState> get_packed() const;
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
    return std::ranges::subrange(v::begin(m_unpacked.get_atoms<P>(), m_problem->get_tree_table()), v::end());
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
struct Hash<mimir::buffering::View<mimir::buffering::PackedState>>
{
    using V = mimir::buffering::View<mimir::buffering::PackedState>;

    size_t operator()(V el) const
    {
        size_t seed = 0;
        size_t hash[2] = { 0, 0 };

        loki::MurmurHash3_x64_128(el.get_buffer() + sizeof(uint8_t) + sizeof(mimir::Index),
                                  el.get_buffer_size() - sizeof(uint8_t) - sizeof(mimir::Index),
                                  seed,
                                  hash);

        loki::hash_combine(seed, hash[0]);
        loki::hash_combine(seed, hash[1]);

        return seed;
    }
};

template<>
struct EqualTo<mimir::buffering::View<mimir::buffering::PackedState>>
{
    using V = mimir::buffering::View<mimir::buffering::PackedState>;

    bool operator()(V lhs, V rhs) const
    {
        auto lhs_begin = lhs.get_buffer() + sizeof(uint8_t) + sizeof(mimir::Index);
        auto rhs_begin = rhs.get_buffer() + sizeof(uint8_t) + sizeof(mimir::Index);
        auto lhs_amount = lhs.get_buffer_size() - sizeof(uint8_t) - sizeof(mimir::Index);
        auto rhs_amount = rhs.get_buffer_size() - sizeof(uint8_t) - sizeof(mimir::Index);

        if (lhs_amount != rhs_amount)
        {
            return false;
        }

        return std::equal(lhs_begin, lhs_begin + lhs_amount, rhs_begin);
    }
};

template<>
struct Hash<mimir::search::State>
{
    size_t operator()(const mimir::search::State& el) const
    {
        return hash_combine(Hash<mimir::buffering::View<mimir::buffering::PackedState>> {}(el.get_packed()), &el.get_problem());
    }
};

template<>
struct EqualTo<mimir::search::State>
{
    bool operator()(const mimir::search::State& lhs, const mimir::search::State& rhs) const
    {
        return EqualTo<mimir::buffering::View<mimir::buffering::PackedState>> {}(lhs.get_packed(), rhs.get_packed())
               && &lhs.get_problem() == &rhs.get_problem();
    }
};
}

namespace mimir::search
{
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
