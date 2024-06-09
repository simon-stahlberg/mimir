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

#include "mimir/search/algorithms/siw/event_handlers/default.hpp"

#include "mimir/search/plan.hpp"

namespace mimir
{
void DefaultSIWAlgorithmEventHandler::on_start_search_impl(const Problem problem, const State initial_state, const PDDLFactories& pddl_factories) const {}

void DefaultSIWAlgorithmEventHandler::on_start_subproblem_search_impl(const Problem problem,
                                                                      const State initial_state,
                                                                      const PDDLFactories& pddl_factories) const
{
    std::cout << "[SIW] Started search." << std::endl;
}

void DefaultSIWAlgorithmEventHandler::on_end_subproblem_search_impl(const IWAlgorithmStatistics& iw_statistics) const {}

void DefaultSIWAlgorithmEventHandler::on_end_search_impl() const { std::cout << "[IW] Search ended.\n" << m_statistics << std::endl; }

void DefaultSIWAlgorithmEventHandler::on_solved_impl(const GroundActionList& ground_action_plan) const
{
    auto plan = to_plan(ground_action_plan);
    std::cout << "[SIW] Plan found.\n"
              << "[SIW] Plan cost: " << plan.get_cost() << "\n"
              << "[SIW] Plan length: " << plan.get_actions().size() << std::endl;
    for (size_t i = 0; i < plan.get_actions().size(); ++i)
    {
        std::cout << "[SIW] " << i + 1 << ". " << plan.get_actions()[i] << std::endl;
    }
}

void DefaultSIWAlgorithmEventHandler::on_unsolvable_impl() const { std::cout << "[SIW] Unsolvable!" << std::endl; }

void DefaultSIWAlgorithmEventHandler::on_exhausted_impl() const { std::cout << "[SIW] Exhausted!" << std::endl; }

DefaultSIWAlgorithmEventHandler::DefaultSIWAlgorithmEventHandler(bool quiet) : SIWAlgorithmEventHandlerBase<DefaultSIWAlgorithmEventHandler>(quiet) {}
}
