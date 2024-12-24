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

#include "mimir/search/axiom.hpp"

#include "mimir/common/hash_cista.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/repositories.hpp"

namespace mimir
{

/* GroundAxiom */

Index& GroundAxiomImpl::get_index() { return m_index; }

Index GroundAxiomImpl::get_index() const { return m_index; }

Index& GroundAxiomImpl::get_axiom() { return m_axiom_index; }

Index GroundAxiomImpl::get_axiom_index() const { return m_axiom_index; }

FlatIndexList& GroundAxiomImpl::get_object_indices() { return m_objects; }

const FlatIndexList& GroundAxiomImpl::get_object_indices() const { return m_objects; }

GroundConditionStrips& GroundAxiomImpl::get_strips_precondition() { return m_strips_precondition; }

const GroundConditionStrips& GroundAxiomImpl::get_strips_precondition() const { return m_strips_precondition; }

GroundEffectDerivedLiteral& GroundAxiomImpl::get_derived_effect() { return m_effect; }

const GroundEffectDerivedLiteral& GroundAxiomImpl::get_derived_effect() const { return m_effect; }

bool GroundAxiomImpl::is_dynamically_applicable(const DenseState& dense_state) const
{
    return get_strips_precondition().is_dynamically_applicable(dense_state);
}

bool GroundAxiomImpl::is_statically_applicable(const FlatBitset& static_positive_atoms) const
{
    return get_strips_precondition().is_statically_applicable(static_positive_atoms);
}

bool GroundAxiomImpl::is_applicable(Problem problem, const DenseState& dense_state) const
{
    return get_strips_precondition().is_applicable(problem, dense_state);
}

/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundEffectDerivedLiteral, const PDDLRepositories&>& data)
{
    const auto [derived_effect, pddl_repositories] = data;

    const auto& ground_atom = pddl_repositories.get_ground_atom<Derived>(derived_effect.atom_index);

    if (derived_effect.is_negated)
    {
        os << "(not ";
    }

    os << *ground_atom;

    if (derived_effect.is_negated)
    {
        os << ")";
    }

    return os;
}

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundAxiom, const PDDLRepositories&>& data)
{
    const auto [axiom, pddl_repositories] = data;

    auto binding = ObjectList {};
    for (const auto object_index : axiom->get_object_indices())
    {
        binding.push_back(pddl_repositories.get_object(object_index));
    }

    const auto& strips_precondition = axiom->get_strips_precondition();

    os << "Axiom("                                                                                                                          //
       << "index=" << axiom->get_index() << ", "                                                                                            //
       << "name=" << pddl_repositories.get_axiom(axiom->get_axiom_index())->get_literal()->get_atom()->get_predicate()->get_name() << ", "  //
       << "binding=" << binding << ", "                                                                                                     //
       << std::make_tuple(strips_precondition, std::cref(pddl_repositories)) << ", "                                                        //
       << "effect=" << std::make_tuple(axiom->get_derived_effect(), std::cref(pddl_repositories)) << ")";

    return os;
}
}
