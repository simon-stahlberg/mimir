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

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/flat_types.hpp"

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

    bool operator==(const FlatDerivedEffect& other) const;
};

using FlatAxiomLayout = flatmemory::Tuple<GroundAxiomIndex,  //
                                          Axiom,
                                          FlatObjectListLayout,
                                          FlatStripsActionPreconditionLayout,
                                          FlatStripsActionEffectLayout,
                                          FlatDerivedEffect>;
using FlatAxiomBuilder = flatmemory::Builder<FlatAxiomLayout>;
using FlatAxiom = flatmemory::ConstView<FlatAxiomLayout>;
using FlatAxiomVector = flatmemory::VariableSizedTypeVector<FlatAxiomLayout>;
}

template<>
struct std::hash<mimir::FlatAxiom>
{
    size_t operator()(mimir::FlatAxiom element) const;
};

template<>
struct std::equal_to<mimir::FlatAxiom>
{
    bool operator()(mimir::FlatAxiom lhs, mimir::FlatAxiom rhs) const;
};

namespace mimir
{
using FlatAxiomSet = flatmemory::UnorderedSet<FlatAxiomLayout>;

/**
 * GroundAxiomBuilder
 */
class GroundAxiomBuilder
{
private:
    FlatAxiomBuilder m_builder;

public:
    FlatAxiomBuilder& get_flatmemory_builder();
    const FlatAxiomBuilder& get_flatmemory_builder() const;

    GroundAxiomIndex& get_index();
    Axiom& get_axiom();
    FlatObjectListBuilder& get_objects();
    /* STRIPS part */
    FlatStripsActionPreconditionBuilder& get_strips_precondition();
    FlatStripsActionEffectBuilder& get_strips_effect();
    /* Simple effect */
    FlatDerivedEffect& get_derived_effect();
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
    explicit GroundAxiom(FlatAxiom view);

    bool operator==(const GroundAxiom& other) const;

    GroundAxiomIndex get_index() const;
    Axiom get_axiom() const;
    FlatObjectList get_objects() const;

    /* STRIPS part */
    FlatStripsActionPrecondition get_strips_precondition() const;
    FlatStripsActionEffect get_strips_effect() const;
    /* Effect*/
    FlatDerivedEffect get_derived_effect() const;

    bool is_applicable(const FlatBitsetBuilder<Fluent>& state_fluent_atoms,
                       const FlatBitsetBuilder<Derived>& state_derived_atoms,
                       const FlatBitsetBuilder<Static>& static_positive_atoms) const;

    bool is_statically_applicable(const FlatBitset<Static> static_positive_bitset) const;
};

}

template<>
struct std::hash<mimir::GroundAxiom>
{
    size_t operator()(mimir::GroundAxiom element) const;
};

template<>
struct std::equal_to<mimir::GroundAxiom>
{
    size_t operator()(mimir::GroundAxiom lhs, mimir::GroundAxiom rhs) const;
};

namespace mimir
{

/**
 * Mimir types
 */

using GroundAxiomList = std::vector<GroundAxiom>;
using GroundAxiomSet = std::unordered_set<GroundAxiom>;

/**
 * Pretty printing
 */

extern std::ostream& operator<<(std::ostream& os, const std::tuple<FlatDerivedEffect, const PDDLFactories&>& data);

extern std::ostream& operator<<(std::ostream& os, const std::tuple<GroundAxiom, const PDDLFactories&>& data);
}

#endif
