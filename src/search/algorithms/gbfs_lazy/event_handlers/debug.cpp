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

#include "mimir/search/algorithms/gbfs_lazy/event_handlers/debug.hpp"

#include "mimir/common/formatter.hpp"
#include "mimir/formalism/formatter.hpp"
#include "mimir/search/formatter.hpp"
#include "mimir/search/plan.hpp"  // remove this eventually

using namespace mimir::formalism;

namespace mimir::search::gbfs_lazy
{
void DebugEventHandlerImpl::on_expand_state_impl(const State& state) const
{
    std::cout << "[GBFS] ----------------------------------------\n"
              << "[GBFS] State: " << state << "\n"
              << std::endl;
}

void DebugEventHandlerImpl::on_expand_goal_state_impl(const State& state) const {}

void DebugEventHandlerImpl::on_generate_state_impl(const State& state, GroundAction action, ContinuousCost action_cost, const State& successor_state) const
{
    std::cout << "[GBFS] Action: ";
    mimir::print(std::cout, std::make_tuple(std::cref(*action), std::cref(*m_problem), FullFormatterTag {}));
    std::cout << "\n"
              << "[GBFS] Successor: " << successor_state << "\n"
              << std::endl;
}

void DebugEventHandlerImpl::on_prune_state_impl(const State& state) const {}

void DebugEventHandlerImpl::on_start_search_impl(const State& start_state, ContinuousCost g_value, ContinuousCost h_value) const
{
    std::cout << "[GBFS] Search started.\n"
              << "[GBFS] Initial g_value: " << g_value << "\n"
              << "[GBFS] Initial h_value: " << h_value << "\n"
              << "[GBFS] Initial state: " << start_state << std::endl;
}

void DebugEventHandlerImpl::on_new_best_h_value_impl(ContinuousCost h_value, uint64_t num_expanded_states, uint64_t num_generated_states) const
{
    std::cout << "[GBFS] New best h_value: " << h_value << " with num expanded states " << num_expanded_states << " and num generated states "
              << num_generated_states << " (" << get_statistics().get_current_search_time_ms().count() << " ms)" << std::endl;
}

void DebugEventHandlerImpl::on_end_search_impl(uint64_t num_reached_fluent_atoms,
                                               uint64_t num_reached_derived_atoms,
                                               uint64_t num_states,
                                               uint64_t num_nodes,
                                               uint64_t num_actions,
                                               uint64_t num_axioms) const
{
    std::cout << "[GBFS] Search ended.\n" << m_statistics << std::endl;
}

void DebugEventHandlerImpl::on_solved_impl(const Plan& plan) const
{
    std::cout << "[GBFS] Plan found.\n"
              << "[GBFS] Plan cost: " << plan.get_cost() << "\n"
              << "[GBFS] Plan length: " << plan.get_actions().size() << std::endl;
    for (size_t i = 0; i < plan.get_actions().size(); ++i)
    {
        std::cout << "[GBFS] " << i << ". ";
        mimir::print(std::cout, std::make_tuple(std::cref(*plan.get_actions()[i]), std::cref(*m_problem), PlanFormatterTag {}));
        std::cout << std::endl;
    }
}

void DebugEventHandlerImpl::on_unsolvable_impl() const { std::cout << "[GBFS] Unsolvable!" << std::endl; }

void DebugEventHandlerImpl::on_exhausted_impl() const { std::cout << "[GBFS] Exhausted!" << std::endl; }

DebugEventHandlerImpl::DebugEventHandlerImpl(formalism::Problem problem, bool quiet) : EventHandlerBase<DebugEventHandlerImpl>(problem, quiet) {}

DebugEventHandler DebugEventHandlerImpl::create(formalism::Problem problem, bool quiet) { return std::make_shared<DebugEventHandlerImpl>(problem, quiet); }
}
