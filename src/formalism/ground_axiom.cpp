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

#include "mimir/common/hash.hpp"
#include "mimir/formalism/problem.hpp"

namespace mimir::formalism
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

}

namespace mimir
{

/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<formalism::GroundEffectDerivedLiteral, const formalism::ProblemImpl&>& data)
{
    const auto [derived_effect, problem] = data;

    const auto& ground_atom = problem.get_repositories().get_ground_atom<formalism::DerivedTag>(derived_effect.atom_index);

    if (derived_effect.polarity)
    {
        os << "(not ";
    }

    os << *ground_atom;

    if (derived_effect.polarity)
    {
        os << ")";
    }

    return os;
}

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<formalism::GroundAxiom, const formalism::ProblemImpl&>& data)
{
    const auto [axiom, problem] = data;

    auto binding = formalism::ObjectList {};
    for (const auto object_index : axiom->get_object_indices())
    {
        binding.push_back(problem.get_repositories().get_object(object_index));
    }

    const auto& conjunctive_condition = axiom->get_conjunctive_condition();

    os << "Axiom("                                                                                                                                   //
       << "index=" << axiom->get_index() << ", "                                                                                                     //
       << "name=" << problem.get_repositories().get_axiom(axiom->get_axiom_index())->get_literal()->get_atom()->get_predicate()->get_name() << ", "  //
       << "binding=" << binding << ", "                                                                                                              //
       << std::make_tuple(conjunctive_condition, std::cref(problem)) << ", "                                                                         //
       << "effect=" << std::make_tuple(axiom->get_derived_effect(), std::cref(problem)) << ")";

    return os;
}
}
