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

struct PackedState
{
};

template<>
struct Data<PackedState>
{
    Index fluent_atoms_index;
    Index fluent_atoms_size;
    Index derived_atoms_index;
    Index derived_atoms_size;
    Index numeric_variables;
    Index index;

    template<formalism::IsFluentOrDerivedTag P>
    Index get_atoms_index() const
    {
        if constexpr (std::is_same_v<P, formalism::FluentTag>)
        {
            return fluent_atoms_index;
        }
        else if constexpr (std::is_same_v<P, formalism::DerivedTag>)
        {
            return Index derived_atoms_index;
            ;
        }
        else
        {
            static_assert(dependent_false<P>::value, "Missing implementation for IsStaticOrFluentOrDerivedTag.");
        }
    }

    template<formalism::IsFluentOrDerivedTag P>
    Index get_atoms_size() const
    {
        if constexpr (std::is_same_v<P, formalism::FluentTag>)
        {
            return fluent_atoms_size;
        }
        else if constexpr (std::is_same_v<P, formalism::DerivedTag>)
        {
            return derived_atoms_size;
        }
        else
        {
            static_assert(dependent_false<P>::value, "Missing implementation for IsStaticOrFluentOrDerivedTag.");
        }
    }
};

// The format of the data as follows:
// - self allocated (1 bit)
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
// Footer, as many bits as needed for each of the 6 entries
template<>
class Builder<PackedState>
{
private:
    Data<PackedState> m_data;

    BufferWriter m_buffer;

public:
    Builder() = default;

    void set_data(Data<PackedState> data);

    void serialize();

    const BufferWriter& get_buffer_writer() const;
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
};

using PackedStateSet = buffering::UnorderedSet<PackedState>;
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
    buffering::View<buffering::PackedState> m_internal;
    const formalism::ProblemImpl* m_problem;

    buffering::Data<buffering::PackedState> m_unpacked;

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

    buffering::View<buffering::PackedState> get_internal_state() const;
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
        // TODO
        return 0;
    }
};

template<>
struct EqualTo<mimir::buffering::View<mimir::buffering::PackedState>>
{
    using V = mimir::buffering::View<mimir::buffering::PackedState>;

    bool operator()(V lhs, V rhs) const
    {
        // TODO
        return false;
    }
};

template<>
struct Hash<mimir::search::State>
{
    size_t operator()(const mimir::search::State& el) const
    {
        // TODO
        return hash_combine(el.get_internal_state(), &el.get_problem());
    }
};

template<>
struct EqualTo<mimir::search::State>
{
    bool operator()(const mimir::search::State& lhs, const mimir::search::State& rhs) const
    {
        // TODO
        return lhs.get_internal_state() == rhs.get_internal_state() && &lhs.get_problem() == &rhs.get_problem();
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
