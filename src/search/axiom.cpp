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

#include "mimir/common/printers.hpp"
#include "mimir/formalism/factories.hpp"

#include <ostream>
#include <tuple>

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

/* FlatAxiomHash */

size_t FlatAxiomHash::operator()(const FlatAxiom& view) const
{
    const auto axiom = view.get<1>();
    const auto objects = view.get<2>();
    return loki::hash_combine(axiom, objects.hash());
}

/* FlatAxiomEqual */

bool FlatAxiomEqual::operator()(const FlatAxiom& view_left, const FlatAxiom& view_right) const
{
    const auto axiom_left = view_left.get<1>();
    const auto objects_left = view_left.get<2>();
    const auto axiom_right = view_right.get<1>();
    const auto objects_right = view_right.get<2>();
    return (axiom_left == axiom_right) && (objects_left == objects_right);
}

/* GroundAxiomBuilder */

FlatAxiomBuilder& GroundAxiomBuilder::get_flatmemory_builder() { return m_builder; }

const FlatAxiomBuilder& GroundAxiomBuilder::get_flatmemory_builder() const { return m_builder; }

uint32_t& GroundAxiomBuilder::get_id() { return m_builder.get<0>(); }

Axiom& GroundAxiomBuilder::get_axiom() { return m_builder.get<1>(); }

FlatObjectListBuilder& GroundAxiomBuilder::get_objects() { return m_builder.get<2>(); }

FlatStripsActionPreconditionBuilder& GroundAxiomBuilder::get_strips_precondition() { return m_builder.get<3>(); }

FlatStripsActionEffectBuilder& GroundAxiomBuilder::get_strips_effect() { return m_builder.get<4>(); }

FlatDerivedEffect& GroundAxiomBuilder::get_derived_effect() { return m_builder.get<5>(); }

/* GroundAxiom */

GroundAxiom::GroundAxiom(FlatAxiom view) : m_view(view) {}

size_t GroundAxiom::hash() const { return loki::hash_combine(m_view.buffer()); }

uint32_t GroundAxiom::get_id() const { return m_view.get<0>(); }

Axiom GroundAxiom::get_axiom() const { return m_view.get<1>(); }

FlatObjectList GroundAxiom::get_objects() const { return m_view.get<2>(); }

bool GroundAxiom::operator==(const GroundAxiom& other) const { return m_view.buffer() == other.m_view.buffer(); }

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
       << "id=" << axiom.get_id() << ", "                                                               //
       << "name=" << axiom.get_axiom()->get_literal()->get_atom()->get_predicate()->get_name() << ", "  //
       << "binding=" << binding << ", "                                                                 //
       << std::make_tuple(strips_precondition, std::cref(pddl_factories)) << ", "                       //
       << "effect=" << std::make_tuple(axiom.get_derived_effect(), std::cref(pddl_factories)) << ")";

    return os;
}
}
