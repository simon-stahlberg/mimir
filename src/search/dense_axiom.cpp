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

#include "mimir/search/dense_axiom.hpp"

#include "mimir/common/hash.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/repositories.hpp"

size_t cista::storage::DerefStdHasher<mimir::DenseGroundAxiomImpl>::operator()(const mimir::DenseGroundAxiomImpl* ptr) const
{
    const auto axiom = ptr->get_axiom_index();
    const auto& objects = ptr->get_object_indices();
    return mimir::hash_combine(axiom, objects);
}

bool cista::storage::DerefStdEqualTo<mimir::DenseGroundAxiomImpl>::operator()(const mimir::DenseGroundAxiomImpl* lhs,
                                                                              const mimir::DenseGroundAxiomImpl* rhs) const
{
    const auto axiom_left = lhs->get_axiom_index();
    const auto& objects_left = lhs->get_object_indices();
    const auto axiom_right = rhs->get_axiom_index();
    const auto& objects_right = rhs->get_object_indices();
    return (axiom_left == axiom_right) && (objects_left == objects_right);
}

namespace mimir
{

/* DenseGroundAxiom */

Index& DenseGroundAxiomImpl::get_index() { return m_index; }

Index DenseGroundAxiomImpl::get_index() const { return m_index; }

Index& DenseGroundAxiomImpl::get_axiom() { return m_axiom_index; }

Index DenseGroundAxiomImpl::get_axiom_index() const { return m_axiom_index; }

FlatIndexList& DenseGroundAxiomImpl::get_object_indices() { return m_objects; }

const FlatIndexList& DenseGroundAxiomImpl::get_object_indices() const { return m_objects; }

DenseGroundConditionStrips& DenseGroundAxiomImpl::get_strips_precondition() { return m_strips_precondition; }

const DenseGroundConditionStrips& DenseGroundAxiomImpl::get_strips_precondition() const { return m_strips_precondition; }

DenseGroundEffectDerivedLiteral& DenseGroundAxiomImpl::get_derived_effect() { return m_effect; }

const DenseGroundEffectDerivedLiteral& DenseGroundAxiomImpl::get_derived_effect() const { return m_effect; }

bool DenseGroundAxiomImpl::is_dynamically_applicable(State state) const { return get_strips_precondition().is_dynamically_applicable(state); }

bool DenseGroundAxiomImpl::is_statically_applicable(const FlatBitset& static_positive_atoms) const
{
    return get_strips_precondition().is_statically_applicable(static_positive_atoms);
}

bool DenseGroundAxiomImpl::is_applicable(Problem problem, State state) const { return get_strips_precondition().is_applicable(problem, state); }

template<PredicateTag P>
bool DenseGroundAxiomImpl::is_applicable(const FlatBitset& atoms) const
{
    return get_strips_precondition().template is_applicable<P>(atoms);
}

template bool DenseGroundAxiomImpl::is_applicable<Static>(const FlatBitset& atoms) const;
template bool DenseGroundAxiomImpl::is_applicable<Fluent>(const FlatBitset& atoms) const;
template bool DenseGroundAxiomImpl::is_applicable<Derived>(const FlatBitset& atoms) const;

/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<DenseGroundEffectDerivedLiteral, const PDDLRepositories&>& data)
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
std::ostream& operator<<(std::ostream& os, const std::tuple<DenseGroundAxiom, const PDDLRepositories&>& data)
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
