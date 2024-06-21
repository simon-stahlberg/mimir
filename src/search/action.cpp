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

#include "mimir/search/action.hpp"

#include "mimir/common/printers.hpp"

#include <flatmemory/flatmemory.hpp>
#include <ostream>
#include <tuple>

namespace mimir
{

std::ostream& operator<<(std::ostream& os, const std::tuple<FlatSimpleEffect, const PDDLFactories&>& data)
{
    const auto [simple_effect, pddl_factories] = data;

    const auto& ground_atom = pddl_factories.get_ground_atom<Fluent>(simple_effect.atom_id);

    if (simple_effect.is_negated)
    {
        os << "(not ";
    }

    os << *ground_atom;

    if (simple_effect.is_negated)
    {
        os << ")";
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, const std::tuple<StripsActionPrecondition, const PDDLFactories&>& data)
{
    const auto [strips_precondition_proxy, pddl_factories] = data;

    const auto positive_static_precondition_bitset = strips_precondition_proxy.get_positive_precondition<Static>();
    const auto negative_static_precondition_bitset = strips_precondition_proxy.get_negative_precondition<Static>();
    const auto positive_fluent_precondition_bitset = strips_precondition_proxy.get_positive_precondition<Fluent>();
    const auto negative_fluent_precondition_bitset = strips_precondition_proxy.get_negative_precondition<Fluent>();
    const auto positive_derived_precondition_bitset = strips_precondition_proxy.get_positive_precondition<Derived>();
    const auto negative_derived_precondition_bitset = strips_precondition_proxy.get_negative_precondition<Derived>();

    auto positive_static_precondition = GroundAtomList<Static> {};
    auto negative_static_precondition = GroundAtomList<Static> {};
    auto positive_fluent_precondition = GroundAtomList<Fluent> {};
    auto negative_fluent_precondition = GroundAtomList<Fluent> {};
    auto positive_derived_precondition = GroundAtomList<Derived> {};
    auto negative_derived_precondition = GroundAtomList<Derived> {};

    pddl_factories.get_ground_atoms_from_ids<Static>(positive_static_precondition_bitset, positive_static_precondition);
    pddl_factories.get_ground_atoms_from_ids<Static>(negative_static_precondition_bitset, negative_static_precondition);
    pddl_factories.get_ground_atoms_from_ids<Fluent>(positive_fluent_precondition_bitset, positive_fluent_precondition);
    pddl_factories.get_ground_atoms_from_ids<Fluent>(negative_fluent_precondition_bitset, negative_fluent_precondition);
    pddl_factories.get_ground_atoms_from_ids<Derived>(positive_derived_precondition_bitset, positive_derived_precondition);
    pddl_factories.get_ground_atoms_from_ids<Derived>(negative_derived_precondition_bitset, negative_derived_precondition);

    os << "positive static precondition=" << positive_static_precondition << ", "
       << "negative static precondition=" << negative_static_precondition << ", "
       << "positive fluent precondition=" << positive_fluent_precondition << ", "
       << "negative fluent precondition=" << negative_fluent_precondition << ", "
       << "positive derived precondition=" << positive_derived_precondition << ", "
       << "negative derived precondition=" << negative_derived_precondition;

    return os;
}

std::ostream& operator<<(std::ostream& os, const std::tuple<StripsActionEffect, const PDDLFactories&>& data)
{
    const auto [strips_effect_proxy, pddl_factories] = data;

    const auto positive_effect_bitset = strips_effect_proxy.get_positive_effects();
    const auto negative_effect_bitset = strips_effect_proxy.get_negative_effects();

    auto positive_simple_effects = GroundAtomList<Fluent> {};
    auto negative_simple_effects = GroundAtomList<Fluent> {};

    pddl_factories.get_ground_atoms_from_ids<Fluent>(positive_effect_bitset, positive_simple_effects);
    pddl_factories.get_ground_atoms_from_ids<Fluent>(negative_effect_bitset, negative_simple_effects);

    os << "delete effects=" << negative_simple_effects << ", "
       << "add effects=" << positive_simple_effects;

    return os;
}

std::ostream& operator<<(std::ostream& os, const std::tuple<ConditionalEffect, const PDDLFactories&>& data)
{
    const auto [cond_effect_proxy, pddl_factories] = data;

    const auto positive_static_precondition_bitset = cond_effect_proxy.get_positive_precondition<Static>();
    const auto negative_static_precondition_bitset = cond_effect_proxy.get_negative_precondition<Static>();
    const auto positive_fluent_precondition_bitset = cond_effect_proxy.get_positive_precondition<Fluent>();
    const auto negative_fluent_precondition_bitset = cond_effect_proxy.get_negative_precondition<Fluent>();
    const auto positive_derived_precondition_bitset = cond_effect_proxy.get_positive_precondition<Derived>();
    const auto negative_derived_precondition_bitset = cond_effect_proxy.get_negative_precondition<Derived>();
    const auto& simple_effect = cond_effect_proxy.get_simple_effect();

    auto positive_static_precondition = GroundAtomList<Static> {};
    auto negative_static_precondition = GroundAtomList<Static> {};
    auto positive_fluent_precondition = GroundAtomList<Fluent> {};
    auto negative_fluent_precondition = GroundAtomList<Fluent> {};
    auto positive_derived_precondition = GroundAtomList<Derived> {};
    auto negative_derived_precondition = GroundAtomList<Derived> {};

    pddl_factories.get_ground_atoms_from_ids<Static>(positive_static_precondition_bitset, positive_static_precondition);
    pddl_factories.get_ground_atoms_from_ids<Static>(negative_static_precondition_bitset, negative_static_precondition);
    pddl_factories.get_ground_atoms_from_ids<Fluent>(positive_fluent_precondition_bitset, positive_fluent_precondition);
    pddl_factories.get_ground_atoms_from_ids<Fluent>(negative_fluent_precondition_bitset, negative_fluent_precondition);
    pddl_factories.get_ground_atoms_from_ids<Derived>(positive_derived_precondition_bitset, positive_derived_precondition);
    pddl_factories.get_ground_atoms_from_ids<Derived>(negative_derived_precondition_bitset, negative_derived_precondition);

    os << "positive static precondition=" << positive_static_precondition << ", "
       << "negative static precondition=" << negative_static_precondition << ", "
       << "positive fluent precondition=" << positive_fluent_precondition << ", "
       << "negative fluent precondition=" << negative_fluent_precondition << ", "
       << "positive derived precondition=" << positive_derived_precondition << ", "
       << "negative derived precondition=" << negative_derived_precondition << ", "
       << "effect=" << std::make_tuple(simple_effect, std::cref(pddl_factories));

    return os;
}

std::ostream& operator<<(std::ostream& os, const std::tuple<GroundAction, const PDDLFactories&>& data)
{
    const auto [action, pddl_factories] = data;

    auto binding = ObjectList {};
    for (const auto object : action.get_objects())
    {
        binding.push_back(object);
    }

    auto strips_precondition = StripsActionPrecondition(action.get_strips_precondition());
    auto strips_effect = StripsActionEffect(action.get_strips_effect());
    auto cond_effects = action.get_conditional_effects();

    os << "Action("                                                                //
       << "id=" << action.get_id() << ", "                                         //
       << "name=" << action.get_action()->get_name() << ", "                       //
       << "binding=" << binding << ", "                                            //
       << std::make_tuple(strips_precondition, std::cref(pddl_factories)) << ", "  //
       << std::make_tuple(strips_effect, std::cref(pddl_factories))                //
       << ", "
       << "conditional_effects=[";
    for (const auto& cond_effect : cond_effects)
    {
        os << "[" << std::make_tuple(ConditionalEffect(cond_effect), std::cref(pddl_factories)) << "], ";
    }
    os << "])";

    return os;
}

std::ostream& operator<<(std::ostream& os, GroundAction action)
{
    os << "(" << action.get_action()->get_name();
    // Only take objects w.r.t. to the original action parameters
    for (size_t i = 0; i < action.get_action()->get_original_arity(); ++i)
    {
        os << " " << *action.get_objects()[i];
    }
    os << ")";
    return os;
}

}
