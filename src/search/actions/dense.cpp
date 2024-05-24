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

#include "mimir/search/actions/dense.hpp"

#include "mimir/common/printers.hpp"

#include <flatmemory/flatmemory.hpp>
#include <ostream>
#include <tuple>

namespace mimir
{

DenseGroundActionList to_ground_actions(const FlatDenseActionSet& flat_actions)
{
    auto result = DenseGroundActionList {};
    result.reserve(flat_actions.size());
    for (const auto& flat_action : flat_actions)
    {
        result.push_back(DenseGroundAction(flat_action));
    }
    return result;
}

std::ostream& operator<<(std::ostream& os, const std::tuple<FlatSimpleEffect, const PDDLFactories&>& data)
{
    const auto [simple_effect, pddl_factories] = data;

    const auto& ground_atom = pddl_factories.get_fluent_ground_atom(simple_effect.atom_id);

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

std::ostream& operator<<(std::ostream& os, const std::tuple<DenseGroundAction, const PDDLFactories&>& data)
{
    const auto [action, pddl_factories] = data;

    auto binding = ObjectList {};
    for (const auto object : action.get_objects())
    {
        binding.push_back(object);
    }

    auto positive_static_precondition_bitset = action.get_applicability_positive_static_precondition_bitset();
    auto negative_static_precondition_bitset = action.get_applicability_negative_static_precondition_bitset();
    auto positive_fluent_precondition_bitset = action.get_applicability_positive_precondition_bitset();
    auto negative_fluent_precondition_bitset = action.get_applicability_negative_precondition_bitset();
    auto positive_effect_bitset = action.get_unconditional_positive_effect_bitset();
    auto negative_effect_bitset = action.get_unconditional_negative_effect_bitset();
    auto positive_conditional_fluent_condition_bitsets = action.get_conditional_positive_precondition_bitsets();
    auto negative_conditional_fluent_condition_bitsets = action.get_conditional_negative_precondition_bitsets();
    auto positive_conditional_static_condition_bitsets = action.get_conditional_positive_static_precondition_bitsets();
    auto negative_conditional_static_condition_bitsets = action.get_conditional_negative_static_precondition_bitsets();
    auto conditional_effects = action.get_conditional_effects();

    auto positive_static_precondition = GroundAtomList<Static> {};
    auto negative_static_precondition = GroundAtomList<Static> {};
    auto positive_fluent_precondition = GroundAtomList<Fluent> {};
    auto negative_fluent_precondition = GroundAtomList<Fluent> {};
    auto positive_simple_effects = GroundAtomList<Fluent> {};
    auto negative_simple_effects = GroundAtomList<Fluent> {};
    auto positive_conditional_static_preconditions = std::vector<GroundAtomList<Static>> {};
    auto negative_conditional_static_preconditions = std::vector<GroundAtomList<Static>> {};
    auto positive_conditional_fluent_preconditions = std::vector<GroundAtomList<Fluent>> {};
    auto negative_conditional_fluent_preconditions = std::vector<GroundAtomList<Fluent>> {};

    pddl_factories.get_fluent_ground_atoms_from_ids(positive_fluent_precondition_bitset, positive_fluent_precondition);
    pddl_factories.get_fluent_ground_atoms_from_ids(negative_fluent_precondition_bitset, negative_fluent_precondition);
    pddl_factories.get_static_ground_atoms_from_ids(positive_static_precondition_bitset, positive_static_precondition);
    pddl_factories.get_static_ground_atoms_from_ids(negative_static_precondition_bitset, negative_static_precondition);
    pddl_factories.get_fluent_ground_atoms_from_ids(positive_effect_bitset, positive_simple_effects);
    pddl_factories.get_fluent_ground_atoms_from_ids(negative_effect_bitset, negative_simple_effects);

    const auto num_conditional_effects = action.get_conditional_effects().size();
    positive_conditional_static_preconditions.resize(num_conditional_effects);
    negative_conditional_static_preconditions.resize(num_conditional_effects);
    positive_conditional_fluent_preconditions.resize(num_conditional_effects);
    negative_conditional_fluent_preconditions.resize(num_conditional_effects);
    for (size_t i = 0; i < num_conditional_effects; ++i)
    {
        pddl_factories.get_static_ground_atoms_from_ids(positive_conditional_static_condition_bitsets[i], positive_conditional_static_preconditions[i]);
        pddl_factories.get_static_ground_atoms_from_ids(negative_conditional_static_condition_bitsets[i], negative_conditional_static_preconditions[i]);
        pddl_factories.get_fluent_ground_atoms_from_ids(positive_conditional_fluent_condition_bitsets[i], positive_conditional_fluent_preconditions[i]);
        pddl_factories.get_fluent_ground_atoms_from_ids(negative_conditional_fluent_condition_bitsets[i], negative_conditional_fluent_preconditions[i]);
    }

    os << "Action("
       << "id=" << action.get_id() << ", "
       << "name=" << action.get_action()->get_name() << ", "
       << "binding=" << binding << ", "
       << "positive static precondition=" << positive_static_precondition << ", "
       << "negative static precondition=" << negative_static_precondition << ", "
       << "positive fluent precondition=" << positive_fluent_precondition << ", "
       << "negative fluent precondition=" << negative_fluent_precondition << ", "
       << "simple_delete=" << negative_simple_effects << ", "
       << "simple_add=" << positive_simple_effects << ", "
       << "conditional_effects=[";
    for (size_t i = 0; i < num_conditional_effects; ++i)
    {
        os << "[positive static precondition=" << positive_conditional_static_preconditions[i] << ", "
           << "negative static precondition=" << negative_conditional_static_preconditions[i] << ", "
           << "positive fluent precondition=" << positive_conditional_fluent_preconditions[i] << ", "
           << "negative fluent precondition=" << negative_conditional_fluent_preconditions[i] << ", "
           << "effect=" << std::make_tuple(conditional_effects[i], std::cref(pddl_factories)) << "], ";
    }
    os << "])";

    return os;
}

std::ostream& operator<<(std::ostream& os, const DenseGroundAction& action)
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
