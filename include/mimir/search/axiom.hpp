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

struct SimpleDerivedEffect
{
    bool is_negated = false;
    Index atom_index = Index(0);
};

/**
 * GroundAxiom
 */
struct GroundAxiomImpl
{
    Index m_index = Index(0);
    Index m_axiom_index = Index(0);
    FlatIndexList m_objects = FlatIndexList();
    StripsActionPrecondition m_strips_precondition = StripsActionPrecondition();
    SimpleDerivedEffect m_effect = SimpleDerivedEffect();

    Index& get_index();
    Index get_index() const;

    Index& get_axiom();
    Index get_axiom_index() const;

    FlatIndexList& get_objects();
    const FlatIndexList& get_objects() const;

    /* STRIPS part */
    StripsActionPrecondition& get_strips_precondition();
    const StripsActionPrecondition& get_strips_precondition() const;

    /* Effect*/
    SimpleDerivedEffect& get_derived_effect();
    const SimpleDerivedEffect& get_derived_effect() const;

    // TODO: pass state instead of separated fluent and derived atoms?
    bool is_applicable(const FlatBitset& state_fluent_atoms, const FlatBitset& state_derived_atoms, const FlatBitset& static_positive_atoms) const;

    bool is_statically_applicable(const FlatBitset& static_positive_bitset) const;
};

}

template<>
struct cista::storage::DerefStdHasher<mimir::GroundAxiomImpl>
{
    size_t operator()(const mimir::GroundAxiomImpl* ptr) const;
};

template<>
struct cista::storage::DerefStdEqualTo<mimir::GroundAxiomImpl>
{
    bool operator()(const mimir::GroundAxiomImpl* lhs, const mimir::GroundAxiomImpl* rhs) const;
};

namespace mimir
{

/**
 * Mimir types
 */

using GroundAxiomImplSet = cista::storage::UnorderedSet<GroundAxiomImpl>;

/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<SimpleDerivedEffect, const PDDLRepositories&>& data);

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundAxiom, const PDDLRepositories&>& data);
}

#endif
