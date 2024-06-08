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

#include "mimir/common/printers.hpp"
#include "mimir/search/axiom.hpp"

#include <flatmemory/flatmemory.hpp>
#include <ostream>
#include <tuple>

namespace mimir
{

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
