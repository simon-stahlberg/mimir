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

#ifndef MIMIR_SEARCH_AXIOM_HPP_
#define MIMIR_SEARCH_AXIOM_HPP_

#include "mimir/formalism/factories.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/flat_types.hpp"
#include "mimir/search/state.hpp"

#include <cstdint>
#include <ostream>
#include <tuple>

namespace mimir
{

/**
 * Flatmemory types
 */

struct FlatDerivedEffect
{
    bool is_negated;
    size_t atom_id;

    bool operator==(const FlatDerivedEffect& other) const
    {
        if (this != &other)
        {
            return is_negated == other.is_negated && atom_id == other.atom_id;
        }
        return true;
    }
};

using FlatAxiomLayout = flatmemory::Tuple<uint32_t,  //
                                          Axiom,
                                          FlatObjectListLayout,
                                          FlatStripsActionPreconditionLayout,
                                          FlatStripsActionEffectLayout,
                                          FlatDerivedEffect>;
using FlatAxiomBuilder = flatmemory::Builder<FlatAxiomLayout>;
using FlatAxiom = flatmemory::ConstView<FlatAxiomLayout>;
using FlatAxiomVector = flatmemory::VariableSizedTypeVector<FlatAxiomLayout>;

struct FlatAxiomHash
{
    size_t operator()(const FlatAxiom& view) const
    {
        const auto axiom = view.get<1>();
        const auto objects = view.get<2>();
        return loki::hash_combine(axiom, objects.hash());
    }
};

struct FlatAxiomEqual
{
    bool operator()(const FlatAxiom& view_left, const FlatAxiom& view_right) const
    {
        const auto axiom_left = view_left.get<1>();
        const auto objects_left = view_left.get<2>();
        const auto axiom_right = view_right.get<1>();
        const auto objects_right = view_right.get<2>();
        return (axiom_left == axiom_right) && (objects_left == objects_right);
    }
};

using FlatAxiomSet = flatmemory::UnorderedSet<FlatAxiomLayout, FlatAxiomHash, FlatAxiomEqual>;

/**
 * GroundAxiomBuilder
 */
class GroundAxiomBuilder
{
private:
    FlatAxiomBuilder m_builder;

public:
    [[nodiscard]] FlatAxiomBuilder& get_flatmemory_builder() { return m_builder; }
    [[nodiscard]] const FlatAxiomBuilder& get_flatmemory_builder() const { return m_builder; }

    [[nodiscard]] uint32_t& get_id() { return m_builder.get<0>(); }
    [[nodiscard]] Axiom& get_axiom() { return m_builder.get<1>(); }
    [[nodiscard]] FlatObjectListBuilder& get_objects() { return m_builder.get<2>(); }
    /* STRIPS part */
    [[nodiscard]] FlatStripsActionPreconditionBuilder& get_strips_precondition() { return m_builder.get<3>(); }
    [[nodiscard]] FlatStripsActionEffectBuilder& get_strips_effect() { return m_builder.get<4>(); }
    /* Simple effect */
    [[nodiscard]] FlatDerivedEffect& get_derived_effect() { return m_builder.get<5>(); }
};

/**
 * GroundAxiom
 */
class GroundAxiom
{
private:
    FlatAxiom m_view;

public:
    /// @brief Create a view on a Axiom.
    explicit GroundAxiom(FlatAxiom view) : m_view(view) {}

    /// @brief Return a hash value for the grounded action.
    ///
    /// Same argument from operator== applies.
    [[nodiscard]] size_t _Cxx_hashtable_define_trivial_hash() const { return loki::hash_combine(m_view.buffer()); }

    [[nodiscard]] uint32_t get_id() const { return m_view.get<0>(); }
    [[nodiscard]] Axiom get_axiom() const { return m_view.get<1>(); }
    [[nodiscard]] FlatObjectList get_objects() const { return m_view.get<2>(); }

    /// Return true iff two grounded axioms are equal.
    ///
    /// For grounded axioms in same AE, we know they are already unique.
    /// Hence, comparison of the buffer pointer suffices.
    /// For grounded axioms in different AE, buffer pointers are always different.
    /// Hence, comparison always returns false.
    [[nodiscard]] bool operator==(const GroundAxiom& other) const { return m_view.buffer() == other.m_view.buffer(); }

    /* STRIPS part */
    [[nodiscard]] FlatStripsActionPrecondition get_strips_precondition() const { return m_view.get<3>(); }
    [[nodiscard]] FlatStripsActionEffect get_strips_effect() const { return m_view.get<4>(); }
    /* Effect*/
    [[nodiscard]] FlatDerivedEffect get_derived_effect() const { return m_view.get<5>(); }

    [[nodiscard]] bool is_applicable(const FlatBitsetBuilder<Fluent> state_fluent_atoms,
                                     const FlatBitsetBuilder<Derived> state_derived_atoms,
                                     const FlatBitset<Static> static_positive_bitset) const
    {  //
        return StripsActionPrecondition(get_strips_precondition()).is_applicable(state_fluent_atoms, state_derived_atoms, static_positive_bitset);
    }

    [[nodiscard]] bool is_statically_applicable(const FlatBitset<Static> static_positive_bitset) const
    {  //
        return StripsActionPrecondition(get_strips_precondition()).is_statically_applicable(static_positive_bitset);
    }
};

/**
 * Mimir types
 */

using GroundAxiomList = std::vector<GroundAxiom>;
using GroundAxiomSet = std::unordered_set<GroundAxiom, loki::Hash<GroundAxiom>, loki::EqualTo<GroundAxiom>>;

/**
 * Pretty printing
 */

extern std::ostream& operator<<(std::ostream& os, const std::tuple<FlatDerivedEffect, const PDDLFactories&>& data);

extern std::ostream& operator<<(std::ostream& os, const std::tuple<GroundAxiom, const PDDLFactories&>& data);
}

#endif
