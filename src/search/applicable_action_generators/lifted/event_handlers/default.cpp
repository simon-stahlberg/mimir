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

#include "mimir/search/applicable_action_generators/lifted/event_handlers/default.hpp"

#include <iostream>

namespace mimir
{
void DefaultLiftedAAGEventHandler::on_start_generating_applicable_actions_impl() const
{  //
}

void DefaultLiftedAAGEventHandler::on_ground_action_impl(Action action, const ObjectList& binding) const
{  //
}

void DefaultLiftedAAGEventHandler::on_ground_action_cache_hit_impl(Action action, const ObjectList& binding) const
{  //
}

void DefaultLiftedAAGEventHandler::on_ground_action_cache_miss_impl(Action action, const ObjectList& binding) const
{  //
}

void DefaultLiftedAAGEventHandler::on_end_generating_applicable_actions_impl(const GroundActionList& ground_actions, const PDDLFactories& pddl_factories) const
{
    //
}

void DefaultLiftedAAGEventHandler::on_start_generating_applicable_axioms_impl() const
{  //
}

void DefaultLiftedAAGEventHandler::on_ground_axiom_impl(Axiom axiom, const ObjectList& binding) const
{
    //
}

void DefaultLiftedAAGEventHandler::on_ground_axiom_cache_hit_impl(Axiom axiom, const ObjectList& binding) const
{
    //
}

void DefaultLiftedAAGEventHandler::on_ground_axiom_cache_miss_impl(Axiom axiom, const ObjectList& binding) const
{
    //
}

void DefaultLiftedAAGEventHandler::on_end_generating_applicable_axioms_impl(const GroundAxiomList& ground_axioms, const PDDLFactories& pddl_factories) const
{
    //
}

void DefaultLiftedAAGEventHandler::on_finish_f_layer_impl() const
{  //
}

void DefaultLiftedAAGEventHandler::on_end_search_impl() const
{  //
    std::cout << get_statistics() << std::endl;
}
}
