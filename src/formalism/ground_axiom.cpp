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

#include "mimir/formalism/ground_axiom.hpp"

#include "mimir/common/hash_cista.hpp"
#include "mimir/formalism/repositories.hpp"

namespace mimir
{

/* GroundAxiom */

Index& GroundAxiomImpl::get_index() { return m_index; }

Index GroundAxiomImpl::get_index() const { return m_index; }

Index& GroundAxiomImpl::get_axiom() { return m_axiom_index; }

Index GroundAxiomImpl::get_axiom_index() const { return m_axiom_index; }

FlatIndexList& GroundAxiomImpl::get_object_indices() { return m_object_indices; }

const FlatIndexList& GroundAxiomImpl::get_object_indices() const { return m_object_indices; }

GroundConjunctiveCondition& GroundAxiomImpl::get_conjunctive_condition() { return m_conjunctive_condition; }

const GroundConjunctiveCondition& GroundAxiomImpl::get_conjunctive_condition() const { return m_conjunctive_condition; }

GroundEffectDerivedLiteral& GroundAxiomImpl::get_derived_effect() { return m_literal; }

const GroundEffectDerivedLiteral& GroundAxiomImpl::get_derived_effect() const { return m_literal; }

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

    const auto& conjunctive_condition = axiom->get_conjunctive_condition();

    os << "Axiom("                                                                                                                          //
       << "index=" << axiom->get_index() << ", "                                                                                            //
       << "name=" << pddl_repositories.get_axiom(axiom->get_axiom_index())->get_literal()->get_atom()->get_predicate()->get_name() << ", "  //
       << "binding=" << binding << ", "                                                                                                     //
       << std::make_tuple(conjunctive_condition, std::cref(pddl_repositories)) << ", "                                                      //
       << "effect=" << std::make_tuple(axiom->get_derived_effect(), std::cref(pddl_repositories)) << ")";

    return os;
}
}
