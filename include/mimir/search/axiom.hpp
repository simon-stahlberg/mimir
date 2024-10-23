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

#include "mimir/common/printers.hpp"
#include "mimir/common/types.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/action.hpp"

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
    size_t atom_index;

    bool operator==(const FlatDerivedEffect& other) const;
};

using FlatAxiom = cista::tuple<Index,          // GroundAxiomIndex
                               Index,          // AxiomIndex
                               FlatIndexList,  // ObjectIndices
                               FlatStripsActionPrecondition,
                               FlatStripsActionEffect,
                               FlatDerivedEffect>;

using FlatAxiomVector = cista::offset::vector<FlatAxiom>;
}

template<>
struct cista::storage::DerefStdHasher<mimir::FlatAxiom>
{
    size_t operator()(const mimir::FlatAxiom* ptr) const;
};

template<>
struct cista::storage::DerefStdEqualTo<mimir::FlatAxiom>
{
    bool operator()(const mimir::FlatAxiom* lhs, const mimir::FlatAxiom* rhs) const;
};

namespace mimir
{
using FlatAxiomSet = cista::storage::UnorderedSet<FlatAxiom>;

/**
 * GroundAxiomBuilder
 */
class GroundAxiomBuilder
{
private:
    FlatAxiom m_builder;

public:
    FlatAxiom& get_data();
    const FlatAxiom& get_data() const;

    Index& get_index();
    Index& get_axiom();
    FlatIndexList& get_objects();
    /* STRIPS part */
    FlatStripsActionPrecondition& get_strips_precondition();
    FlatStripsActionEffect& get_strips_effect();
    /* Simple effect */
    FlatDerivedEffect& get_derived_effect();
};

/**
 * GroundAxiom
 */
class GroundAxiom
{
private:
    std::reference_wrapper<const FlatAxiom> m_view;

public:
    /// @brief Create a view on a Axiom.
    explicit GroundAxiom(const FlatAxiom& view);

    Index get_index() const;
    Index get_axiom_index() const;
    const FlatIndexList& get_objects() const;

    /* STRIPS part */
    const FlatStripsActionPrecondition& get_strips_precondition() const;
    const FlatStripsActionEffect& get_strips_effect() const;
    /* Effect*/
    const FlatDerivedEffect& get_derived_effect() const;

    // TODO: pass state instead of separated fluent and derived atoms
    bool is_applicable(const FlatBitset& state_fluent_atoms, const FlatBitset& state_derived_atoms, const FlatBitset& static_positive_atoms) const;

    bool is_statically_applicable(const FlatBitset& static_positive_bitset) const;
};

extern bool operator==(GroundAxiom lhs, GroundAxiom rhs);

}

template<>
struct std::hash<mimir::GroundAxiom>
{
    size_t operator()(mimir::GroundAxiom element) const;
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

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<FlatDerivedEffect, const PDDLFactories&>& data);

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundAxiom, const PDDLFactories&>& data);
}

#endif
