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

struct GroundEffectDerivedLiteral
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
    GroundConditionStrips m_strips_precondition = GroundConditionStrips();
    GroundEffectDerivedLiteral m_effect = GroundEffectDerivedLiteral();

    Index& get_index();
    Index get_index() const;

    Index& get_axiom();
    Index get_axiom_index() const;

    FlatIndexList& get_object_indices();
    const FlatIndexList& get_object_indices() const;

    /* STRIPS part */
    GroundConditionStrips& get_strips_precondition();
    const GroundConditionStrips& get_strips_precondition() const;

    /* Effect*/
    GroundEffectDerivedLiteral& get_derived_effect();
    const GroundEffectDerivedLiteral& get_derived_effect() const;

    bool is_dynamically_applicable(State state) const;

    bool is_statically_applicable(const FlatBitset& static_positive_atoms) const;

    bool is_applicable(Problem problem, State state) const;

    template<PredicateTag P>
    bool is_applicable(const FlatBitset& atoms) const;
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
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundEffectDerivedLiteral, const PDDLRepositories&>& data);

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundAxiom, const PDDLRepositories&>& data);
}

#endif
