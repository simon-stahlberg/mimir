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

#include "mimir/search/applicable_action_generators/grounded/event_handlers/default.hpp"

#include "mimir/search/action.hpp"
#include "mimir/search/grounding/match_tree.hpp"
#include "mimir/search/state.hpp"

#include <iostream>

namespace mimir
{

void DefaultGroundedApplicableActionGeneratorEventHandler::on_finish_delete_free_exploration_impl(const GroundAtomList<Fluent>& reached_fluent_atoms,
                                                                                                  const GroundAtomList<Derived>& reached_derived_atoms,
                                                                                                  const GroundActionList& instantiated_actions)
{
    std::cout << "[GroundedApplicableActionGenerator] Number of fluent grounded atoms reachable in delete-free problem: " << reached_fluent_atoms.size() << "\n"
              << "[GroundedApplicableActionGenerator] Number of derived grounded atoms reachable in delete-free problem: " << reached_derived_atoms.size()
              << "\n"
              << "[GroundedApplicableActionGenerator] Number of delete-free grounded actions: " << instantiated_actions.size() << std::endl;
}

void DefaultGroundedApplicableActionGeneratorEventHandler::on_finish_grounding_unrelaxed_actions_impl(const GroundActionList& unrelaxed_actions)
{
    std::cout << "[GroundedApplicableActionGenerator] Number of grounded actions in problem: " << unrelaxed_actions.size() << std::endl;
}

void DefaultGroundedApplicableActionGeneratorEventHandler::on_finish_build_action_match_tree_impl(const MatchTree<GroundAction>& action_match_tree)
{
    std::cout << "[GroundedApplicableActionGenerator] Number of nodes in action match tree: " << action_match_tree.get_num_nodes() << std::endl;
}

void DefaultGroundedApplicableActionGeneratorEventHandler::on_finish_search_layer_impl() const
{  //
}

void DefaultGroundedApplicableActionGeneratorEventHandler::on_end_search_impl() const {}

}