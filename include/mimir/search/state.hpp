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

#include "mimir/common/concepts.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/formalism.hpp"
#include "mimir/search/flat_types.hpp"

#include <flatmemory/flatmemory.hpp>
#include <ostream>
#include <tuple>
#include <unordered_map>

namespace mimir
{
/**
 * Flatmemory types
 */
using FlatStateLayout = flatmemory::Tuple<uint32_t, FlatBitsetLayout<Fluent>, FlatBitsetLayout<Derived>>;
using FlatStateBuilder = flatmemory::Builder<FlatStateLayout>;
using FlatState = flatmemory::ConstView<FlatStateLayout>;

// Only hash/compare the non-extended portion of a state, and the problem.
// The extended portion is computed automatically, when calling ssg.create_state(...)
struct FlatStateHash
{
    size_t operator()(const FlatState& view) const
    {
        const auto fluent_atoms = view.get<1>();
        return loki::hash_combine(fluent_atoms.hash());
    }
};

struct FlatStateEqual
{
    bool operator()(const FlatState& view_left, const FlatState& view_right) const
    {
        const auto fluent_atoms_left = view_left.get<1>();
        const auto fluent_atoms_right = view_right.get<1>();
        return (fluent_atoms_left == fluent_atoms_right);
    }
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
    [[nodiscard]] FlatStateBuilder& get_flatmemory_builder() { return m_builder; }
    [[nodiscard]] const FlatStateBuilder& get_flatmemory_builder() const { return m_builder; }

    [[nodiscard]] uint32_t& get_id() { return m_builder.get<0>(); }

    template<DynamicPredicateCategory P>
    [[nodiscard]] FlatBitsetBuilder<P>& get_atoms()
    {
        if constexpr (std::is_same_v<P, Fluent>)
        {
            return m_builder.get<1>();
        }
        else if constexpr (std::is_same_v<P, Derived>)
        {
            return m_builder.get<2>();
        }
        else
        {
            static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
        }
    }
};

/**
 * State
 */
class State
{
private:
    FlatState m_view;

public:
    explicit State(FlatState view) : m_view(view) {}

    /// @brief Return true iff two states are equal.
    ///
    /// For states in same SSG, we know they are already unique.
    /// Hence, comparison of the buffer pointer suffices.
    /// For states in different SSG, buffer pointers are always different.
    /// Hence, comparison always returns false.
    [[nodiscard]] bool operator==(const State& other) const { return m_view.buffer() == other.m_view.buffer(); }

    /// @brief Return a hash value for the state.
    ///
    /// Same argument from operator== applies.
    [[nodiscard]] size_t hash() const { return loki::hash_combine(m_view.buffer()); }

    [[nodiscard]] uint32_t get_id() const { return m_view.get<0>(); }

    [[nodiscard]] auto begin_fluent_ground_atoms() const { return m_view.get<1>().begin(); }
    [[nodiscard]] auto end_fluent_ground_atoms() const { return m_view.get<1>().end(); }
    [[nodiscard]] auto begin_derived_ground_atoms() const { return m_view.get<2>().begin(); }
    [[nodiscard]] auto end_derived_ground_atoms() const { return m_view.get<2>().end(); }

    /**
     * Fluent and Derived
     */
    template<DynamicPredicateCategory P>
    [[nodiscard]] bool contains(const GroundAtom<P>& atom) const
    {
        return get_atoms<P>().get(atom->get_identifier());
    }

    template<DynamicPredicateCategory P>
    [[nodiscard]] bool superset_of(const GroundAtomList<P>& atoms) const
    {
        for (const auto& atom : atoms)
        {
            if (!contains(atom))
            {
                return false;
            }
        }

        return true;
    }

    template<DynamicPredicateCategory P>
    [[nodiscard]] bool literal_holds(const GroundLiteral<P>& literal) const
    {
        return literal->is_negated() != contains(literal->get_atom());
    }

    template<DynamicPredicateCategory P>
    [[nodiscard]] bool literals_hold(const GroundLiteralList<P>& literals) const
    {
        for (const auto& literal : literals)
        {
            if (!literal_holds(literal))
            {
                return false;
            }
        }

        return true;
    }

    template<DynamicPredicateCategory P>
    [[nodiscard]] FlatBitset<P> get_atoms() const
    {
        if constexpr (std::is_same_v<P, Fluent>)
        {
            return m_view.get<1>();
        }
        else if constexpr (std::is_same_v<P, Derived>)
        {
            return m_view.get<2>();
        }
        else
        {
            static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
        }
    }
};

/**
 * Mimir types
 */

using StateList = std::vector<State>;

struct StateHash
{
    size_t operator()(const State& view) const { return view.hash(); }
};

template<typename T>
using StateMap = std::unordered_map<State, T, StateHash>;
using StateSet = std::unordered_set<State, StateHash>;

/**
 * Pretty printing
 */

extern std::ostream& operator<<(std::ostream& os, const std::tuple<const Problem, const State, const PDDLFactories&>& data);
}

#endif
