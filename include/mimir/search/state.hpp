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

/// @brief `InternalStateImpl` encapsulates the fluent and derived atoms, and numeric variables of a planning state.
class InternalStateImpl
{
private:
    // Idea for more compact design based on cista::basic_vector:
    bool m_self_allocated;  ///< flag to indicate whether the m_data pointer is self allocated or owned elsewhere
    // The format of the data as follows:
    // Header, 6 bit for each:
    // - num bits for state index
    // - num bits for fluent tree node index
    // - num bits for fluent tree node size
    // - num bits for derived tree node index
    // - num bits for derived tree node size
    // - num bits for numeric variables list index
    // Footer, as many bits as needed for each of the 6 entries
    uint8_t* m_data;

    static constexpr size_t s_capacity = 29;  // 8 uint32_t: 6 * 6 bits (5 bytes) for header + 6 * 4 bytes (24 bytes) for footer

public:
    InternalStateImpl();

    ~InternalStateImpl();

    struct UnpackedData
    {
        v::RootSlotType fluent_atoms;
        v::RootSlotType derived_atoms;
        Index numeric_variables;
        Index index;

        template<formalism::IsFluentOrDerivedTag P>
        v::RootSlotType get_atoms() const
        {
            if constexpr (std::is_same_v<P, formalism::FluentTag>)
            {
                return fluent_atoms;
            }
            else if constexpr (std::is_same_v<P, formalism::DerivedTag>)
            {
                return derived_atoms;
            }
            else
            {
                static_assert(dependent_false<P>::value, "Missing implementation for IsStaticOrFluentOrDerivedTag.");
            }
        }
    };

    void set_data(const UnpackedData& data);

    UnpackedData get_data() const;

    size_t buffer_size() const;

    const uint8_t* buffer() const;
};

template<typename Ctx>
inline void serialize(Ctx& context, InternalStateImpl const* el, cista::offset_t const offset)
{
    using cista::serialize;

    const auto buffer_size = el->buffer_size();

    // serialize buffer_size many bytes from el.m_data
    for (std::size_t i = 0; i < buffer_size; ++i)
    {
        cista::serialize(context, &el->m_data[i], offset + i);
    }
}

template<typename Ctx>
inline void deserialize(Ctx const& context, InternalStateImpl* el)
{
    using cista::deserialize;
    // simply set the m_data to the pointer

    assert(!el->m_self_allocated);

    auto* buffer_start = deserialize<uint8_t const*>(context);

    el->m_data = const_cast<uint8_t*>(buffer_start);  // safe only if you know it's mutable
    el->m_self_allocated = false;
}

using InternalStateImplSet = buffering::UnorderedSet<InternalStateImpl>;

/**
 * State
 */

class State
{
private:
    InternalState m_internal;
    const formalism::ProblemImpl* m_problem;

    InternalStateImpl::UnpackedData m_unpacked;

    State(const InternalStateImpl& internal, const formalism::ProblemImpl& problem);

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

    const InternalStateImpl& get_internal_state() const;
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
struct Hash<mimir::search::InternalStateImpl>
{
    size_t operator()(const mimir::search::InternalStateImpl& el) const
    {
        auto writer = mimir::buffering::UintReader<mimir::Index, mimir::Index, mimir::Index, mimir::Index, mimir::Index, mimir::Index>(el.buffer());
        const auto [index, fluent_index, fluent_size, derived_index, derived_size, numeric_variables] = writer.read();

        return hash_combine(valla::SlotHash()(valla::make_slot(fluent_index, fluent_size)), numeric_variables);
    }
};

template<>
struct EqualTo<mimir::search::InternalStateImpl>
{
    bool operator()(const mimir::search::InternalStateImpl& lhs, const mimir::search::InternalStateImpl& rhs) const
    {
        auto lhs_writer = mimir::buffering::UintReader<mimir::Index, mimir::Index, mimir::Index, mimir::Index, mimir::Index, mimir::Index>(lhs.buffer());
        const auto [lhs_index, lhs_fluent_index, lhs_fluent_size, lhs_derived_index, lhs_derived_size, lhs_numeric_variables] = lhs_writer.read();

        auto rhs_writer = mimir::buffering::UintReader<mimir::Index, mimir::Index, mimir::Index, mimir::Index, mimir::Index, mimir::Index>(rhs.buffer());
        const auto [rhs_index, rhs_fluent_index, rhs_fluent_size, rhs_derived_index, rhs_derived_size, rhs_numeric_variables] = rhs_writer.read();

        return lhs_fluent_index == rhs_fluent_index && lhs_fluent_size == rhs_fluent_size && lhs_numeric_variables == rhs_numeric_variables;
    }
};

template<>
struct Hash<mimir::search::State>
{
    size_t operator()(const mimir::search::State& el) const { return hash_combine(&el.get_internal_state(), &el.get_problem()); }
};

template<>
struct EqualTo<mimir::search::State>
{
    bool operator()(const mimir::search::State& lhs, const mimir::search::State& rhs) const
    {
        return &lhs.get_internal_state() == &rhs.get_internal_state() && &lhs.get_problem() == &rhs.get_problem();
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
