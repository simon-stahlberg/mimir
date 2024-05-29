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

#include "mimir/search/algorithms/event_handlers/debug.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/search/plan.hpp"

namespace mimir
{
void DebugAlgorithmEventHandler::on_generate_state_impl(GroundAction action, State successor_state, const PDDLFactories& pddl_factories) const
{
    std::cout << "[Algorithm] Action: " << std::make_tuple(action, std::cref(pddl_factories)) << "\n"
              << "[Algorithm] Successor: " << std::make_tuple(successor_state, std::cref(pddl_factories)) << "\n"
              << std::endl;
}

void DebugAlgorithmEventHandler::on_finish_f_layer_impl(uint64_t f_value, uint64_t num_expanded_states, uint64_t num_generated_states) const
{
    std::cout << "[Algorithm] Finished state expansion until f-layer " << f_value << " with num expanded states " << num_expanded_states
              << " and num generated states " << num_generated_states << std::endl;
}

void DebugAlgorithmEventHandler::on_expand_state_impl(State state, const PDDLFactories& pddl_factories) const
{
    std::cout << "[Algorithm] ----------------------------------------\n"
              << "[Algorithm] State: " << std::make_tuple(state, std::cref(pddl_factories)) << std::endl
              << std::endl;
}

void DebugAlgorithmEventHandler::on_start_search_impl(State initial_state, const PDDLFactories& pddl_factories) const
{
    std::cout << "[Algorithm] Search started.\n"
              << "[Algorithm] Initial: " << std::make_tuple(initial_state, std::cref(pddl_factories)) << std::endl;
}

void DebugAlgorithmEventHandler::on_end_search_impl() const { std::cout << "[Algorithm] Search ended.\n" << m_statistics << std::endl; }

void DebugAlgorithmEventHandler::on_solved_impl(const GroundActionList& ground_action_plan) const
{
    auto plan = to_plan(ground_action_plan);
    std::cout << "[Algorithm] Plan found.\n"
              << "[Algorithm] Plan cost: " << plan.get_cost() << "\n"
              << "[Algorithm] Plan length: " << plan.get_actions().size() << std::endl;
    for (size_t i = 0; i < plan.get_actions().size(); ++i)
    {
        std::cout << "[Algorithm] " << i + 1 << ". " << plan.get_actions()[i] << std::endl;
    }
}

void DebugAlgorithmEventHandler::on_unsolvable_impl() const { std::cout << "[Algorithm] Unsolvable!" << std::endl; }

void DebugAlgorithmEventHandler::on_exhausted_impl() const { std::cout << "[Algorithm] Exhausted!" << std::endl; }
}
