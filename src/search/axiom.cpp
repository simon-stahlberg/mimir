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
#include "mimir/common/printers.hpp"
#include "mimir/formalism/factories.hpp"

#include <ostream>
#include <tuple>

size_t std::hash<mimir::GroundAxiom>::operator()(mimir::GroundAxiom element) const { return element.get_index(); }

size_t std::equal_to<mimir::GroundAxiom>::operator()(mimir::GroundAxiom lhs, mimir::GroundAxiom rhs) const { return lhs.get_index() == rhs.get_index(); }

size_t std::hash<mimir::FlatAxiom>::operator()(mimir::FlatAxiom element) const
{
    const auto axiom = element.get<1>();
    const auto objects = element.get<2>();
    return mimir::hash_combine(axiom, objects);
}

bool std::equal_to<mimir::FlatAxiom>::operator()(mimir::FlatAxiom lhs, mimir::FlatAxiom rhs) const
{
    const auto axiom_left = lhs.get<1>();
    const auto objects_left = lhs.get<2>();
    const auto axiom_right = rhs.get<1>();
    const auto objects_right = rhs.get<2>();
    return (axiom_left == axiom_right) && (objects_left == objects_right);
}

namespace mimir
{

/* FlatDerivedEffect */

bool FlatDerivedEffect::operator==(const FlatDerivedEffect& other) const
{
    if (this != &other)
    {
        return is_negated == other.is_negated && atom_id == other.atom_id;
    }
    return true;
}

/* GroundAxiomBuilder */

FlatAxiomBuilder& GroundAxiomBuilder::get_flatmemory_builder() { return m_builder; }

const FlatAxiomBuilder& GroundAxiomBuilder::get_flatmemory_builder() const { return m_builder; }

GroundAxiomIndex& GroundAxiomBuilder::get_index() { return m_builder.get<0>(); }

Axiom& GroundAxiomBuilder::get_axiom() { return m_builder.get<1>(); }

FlatObjectListBuilder& GroundAxiomBuilder::get_objects() { return m_builder.get<2>(); }

FlatStripsActionPreconditionBuilder& GroundAxiomBuilder::get_strips_precondition() { return m_builder.get<3>(); }

FlatStripsActionEffectBuilder& GroundAxiomBuilder::get_strips_effect() { return m_builder.get<4>(); }

FlatDerivedEffect& GroundAxiomBuilder::get_derived_effect() { return m_builder.get<5>(); }

/* GroundAxiom */

GroundAxiom::GroundAxiom(FlatAxiom view) : m_view(view) {}

bool GroundAxiom::operator==(const GroundAxiom& other) const { return get_index() == other.get_index(); }

GroundAxiomIndex GroundAxiom::get_index() const { return m_view.get<0>(); }

Axiom GroundAxiom::get_axiom() const { return m_view.get<1>(); }

FlatObjectList GroundAxiom::get_objects() const { return m_view.get<2>(); }

FlatStripsActionPrecondition GroundAxiom::get_strips_precondition() const { return m_view.get<3>(); }

FlatStripsActionEffect GroundAxiom::get_strips_effect() const { return m_view.get<4>(); }

FlatDerivedEffect GroundAxiom::get_derived_effect() const { return m_view.get<5>(); }

bool GroundAxiom::is_applicable(const FlatBitsetBuilder<Fluent>& state_fluent_atoms,
                                const FlatBitsetBuilder<Derived>& state_derived_atoms,
                                const FlatBitsetBuilder<Static>& static_positive_atoms) const
{  //
    return StripsActionPrecondition(get_strips_precondition()).is_applicable(state_fluent_atoms, state_derived_atoms, static_positive_atoms);
}

bool GroundAxiom::is_statically_applicable(const FlatBitset<Static> static_positive_bitset) const
{  //
    return StripsActionPrecondition(get_strips_precondition()).is_statically_applicable(static_positive_bitset);
}

/**
 * Pretty printing
 */

std::ostream& operator<<(std::ostream& os, const std::tuple<FlatDerivedEffect, const PDDLFactories&>& data)
{
    const auto [derived_effect, pddl_factories] = data;

    const auto& ground_atom = pddl_factories.get_ground_atom<Derived>(derived_effect.atom_id);

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
    for (const auto object : axiom.get_objects())
    {
        binding.push_back(object);
    }

    auto strips_precondition = StripsActionPrecondition(axiom.get_strips_precondition());

    os << "Axiom("                                                                                      //
       << "index=" << axiom.get_index() << ", "                                                         //
       << "name=" << axiom.get_axiom()->get_literal()->get_atom()->get_predicate()->get_name() << ", "  //
       << "binding=" << binding << ", "                                                                 //
       << std::make_tuple(strips_precondition, std::cref(pddl_factories)) << ", "                       //
       << "effect=" << std::make_tuple(axiom.get_derived_effect(), std::cref(pddl_factories)) << ")";

    return os;
}
}
