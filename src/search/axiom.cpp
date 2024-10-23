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

#include "mimir/common/hash.hpp"
#include "mimir/formalism/factories.hpp"

#include <ostream>
#include <tuple>

size_t std::hash<mimir::GroundAxiom>::operator()(mimir::GroundAxiom element) const { return element.get_index(); }

size_t cista::storage::DerefStdHasher<mimir::FlatAxiom>::operator()(const mimir::FlatAxiom* ptr) const
{
    const auto axiom = cista::get<1>(*ptr);
    const auto& objects = cista::get<2>(*ptr);
    return mimir::hash_combine(axiom, objects);
}

bool cista::storage::DerefStdEqualTo<mimir::FlatAxiom>::operator()(const mimir::FlatAxiom* lhs, const mimir::FlatAxiom* rhs) const
{
    const auto axiom_left = cista::get<1>(*lhs);
    const auto& objects_left = cista::get<2>(*lhs);
    const auto axiom_right = cista::get<1>(*rhs);
    const auto& objects_right = cista::get<2>(*rhs);
    return (axiom_left == axiom_right) && (objects_left == objects_right);
}

namespace mimir
{

/* FlatDerivedEffect */

bool FlatDerivedEffect::operator==(const FlatDerivedEffect& other) const
{
    if (this != &other)
    {
        return is_negated == other.is_negated && atom_index == other.atom_index;
    }
    return true;
}

/* GroundAxiomBuilder */

FlatAxiom& GroundAxiomBuilder::get_data() { return m_builder; }

const FlatAxiom& GroundAxiomBuilder::get_data() const { return m_builder; }

Index& GroundAxiomBuilder::get_index() { return cista::get<0>(m_builder); }

Index& GroundAxiomBuilder::get_axiom() { return cista::get<1>(m_builder); }

FlatIndexList& GroundAxiomBuilder::get_objects() { return cista::get<2>(m_builder); }

FlatStripsActionPrecondition& GroundAxiomBuilder::get_strips_precondition() { return cista::get<3>(m_builder); }

FlatStripsActionEffect& GroundAxiomBuilder::get_strips_effect() { return cista::get<4>(m_builder); }

FlatDerivedEffect& GroundAxiomBuilder::get_derived_effect() { return cista::get<5>(m_builder); }

/* GroundAxiom */

GroundAxiom::GroundAxiom(const FlatAxiom& view) : m_view(view) {}

Index GroundAxiom::get_index() const { return cista::get<0>(m_view.get()); }

Index GroundAxiom::get_axiom_index() const { return cista::get<1>(m_view.get()); }

const FlatIndexList& GroundAxiom::get_objects() const { return cista::get<2>(m_view.get()); }

const FlatStripsActionPrecondition& GroundAxiom::get_strips_precondition() const { return cista::get<3>(m_view.get()); }

const FlatStripsActionEffect& GroundAxiom::get_strips_effect() const { return cista::get<4>(m_view.get()); }

const FlatDerivedEffect& GroundAxiom::get_derived_effect() const { return cista::get<5>(m_view.get()); }

bool GroundAxiom::is_applicable(const FlatBitset& state_fluent_atoms, const FlatBitset& state_derived_atoms, const FlatBitset& static_positive_atoms) const
{  //
    return StripsActionPrecondition(get_strips_precondition()).is_applicable(state_fluent_atoms, state_derived_atoms, static_positive_atoms);
}

bool GroundAxiom::is_statically_applicable(const FlatBitset& static_positive_bitset) const
{  //
    return StripsActionPrecondition(get_strips_precondition()).is_statically_applicable(static_positive_bitset);
}

bool operator==(GroundAxiom lhs, GroundAxiom rhs) { return lhs.get_index() == rhs.get_index(); }

/**
 * Pretty printing
 */

std::ostream& operator<<(std::ostream& os, const std::tuple<FlatDerivedEffect, const PDDLFactories&>& data)
{
    const auto [derived_effect, pddl_factories] = data;

    const auto& ground_atom = pddl_factories.get_ground_atom<Derived>(derived_effect.atom_index);

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

std::ostream& operator<<(std::ostream& os, const std::tuple<GroundAxiom, const PDDLFactories&>& data)
{
    const auto [axiom, pddl_factories] = data;

    auto binding = ObjectList {};
    for (const auto object_index : axiom.get_objects())
    {
        binding.push_back(pddl_factories.get_object(object_index));
    }

    auto strips_precondition = StripsActionPrecondition(axiom.get_strips_precondition());

    os << "Axiom("                                                                                                                      //
       << "index=" << axiom.get_index() << ", "                                                                                         //
       << "name=" << pddl_factories.get_axiom(axiom.get_axiom_index())->get_literal()->get_atom()->get_predicate()->get_name() << ", "  //
       << "binding=" << binding << ", "                                                                                                 //
       << std::make_tuple(strips_precondition, std::cref(pddl_factories)) << ", "                                                       //
       << "effect=" << std::make_tuple(axiom.get_derived_effect(), std::cref(pddl_factories)) << ")";

    return os;
}
}
