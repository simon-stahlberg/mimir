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

#include "mimir/search/algorithms/astar_lazy/event_handlers/default.hpp"

#include "mimir/common/formatter.hpp"
#include "mimir/formalism/ground_action.hpp"
#include "mimir/search/plan.hpp"
#include "mimir/search/state.hpp"

#include <chrono>

using namespace mimir::formalism;

namespace mimir::search::astar_lazy
{
void DefaultEventHandlerImpl::on_expand_state_impl(const State& state) const {}

void DefaultEventHandlerImpl::on_expand_goal_state_impl(const State& state) const {}

void DefaultEventHandlerImpl::on_generate_state_impl(const State& state, GroundAction action, ContinuousCost action_cost, const State& successor_state) const {}

void DefaultEventHandlerImpl::on_generate_state_relaxed_impl(const State& state,
                                                             GroundAction action,
                                                             ContinuousCost action_cost,
                                                             const State& successor_state) const
{
}

void DefaultEventHandlerImpl::on_generate_state_not_relaxed_impl(const State& state,
                                                                 GroundAction action,
                                                                 ContinuousCost action_cost,
                                                                 const State& successor_state) const
{
}

void DefaultEventHandlerImpl::on_close_state_impl(const State& state) const {}

void DefaultEventHandlerImpl::on_finish_f_layer_impl(ContinuousCost f_value, uint64_t num_expanded_states, uint64_t num_generated_states) const
{
    std::cout << "[AStar] Finished state expansion until f-layer " << f_value << " with num expanded states " << num_expanded_states
              << " and num generated states " << num_generated_states << " (" << get_statistics().get_current_search_time_ms().count() << " ms)" << std::endl;
}

void DefaultEventHandlerImpl::on_prune_state_impl(const State& state) const {}

void DefaultEventHandlerImpl::on_start_search_impl(const State& start_state, ContinuousCost g_value, ContinuousCost h_value) const
{
    std::cout << "[AStar] Search started.\n"
              << "[AStar] Initial g_value: " << g_value << "\n"
              << "[AStar] Initial h_value: " << h_value << std::endl;
}

void DefaultEventHandlerImpl::on_end_search_impl(uint64_t num_reached_fluent_atoms,
                                                 uint64_t num_reached_derived_atoms,
                                                 uint64_t num_states,
                                                 uint64_t num_nodes,
                                                 uint64_t num_actions,
                                                 uint64_t num_axioms) const
{
    std::cout << "[AStar] Search ended.\n" << m_statistics << std::endl;
}

void DefaultEventHandlerImpl::on_solved_impl(const Plan& plan) const
{
    std::cout << "[AStar] Plan found.\n"
              << "[AStar] Plan cost: " << plan.get_cost() << "\n"
              << "[AStar] Plan length: " << plan.get_actions().size() << std::endl;
    for (size_t i = 0; i < plan.get_actions().size(); ++i)
    {
        std::cout << "[AStar] " << i << ". ";
        mimir::operator<<(std::cout, std::make_tuple(plan.get_actions()[i], std::cref(*m_problem), GroundActionImpl::PlanFormatterTag {}));
        std::cout << std::endl;
    }
}

void DefaultEventHandlerImpl::on_unsolvable_impl() const { std::cout << "[AStar] Unsolvable!" << std::endl; }

void DefaultEventHandlerImpl::on_exhausted_impl() const { std::cout << "[AStar] Exhausted!" << std::endl; }

DefaultEventHandlerImpl::DefaultEventHandlerImpl(formalism::Problem problem, bool quiet) : EventHandlerBase<DefaultEventHandlerImpl>(problem, quiet) {}

DefaultEventHandler DefaultEventHandlerImpl::create(formalism::Problem problem, bool quiet)
{
    return std::make_shared<DefaultEventHandlerImpl>(problem, quiet);
}
}
