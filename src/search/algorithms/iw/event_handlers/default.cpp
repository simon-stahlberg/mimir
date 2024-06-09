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

#include "mimir/search/algorithms/iw/event_handlers/default.hpp"

#include "mimir/search/plan.hpp"

namespace mimir
{
void DefaultIWAlgorithmEventHandler::on_start_search_impl(const Problem problem, const State initial_state, const PDDLFactories& pddl_factories) const {}

void DefaultIWAlgorithmEventHandler::on_start_arity_search_impl(const Problem problem,
                                                                const State initial_state,
                                                                const PDDLFactories& pddl_factories,
                                                                int arity) const
{
    std::cout << "[IW] Start search with arity " << arity << std::endl;
}

void DefaultIWAlgorithmEventHandler::on_end_arity_search_impl(const BrFSAlgorithmStatistics& brfs_statistics) const {}

void DefaultIWAlgorithmEventHandler::on_end_search_impl() const { std::cout << "[IW] Search ended.\n" << m_statistics << std::endl; }

void DefaultIWAlgorithmEventHandler::on_solved_impl(const GroundActionList& ground_action_plan) const
{
    auto plan = to_plan(ground_action_plan);
    std::cout << "[IW] Plan found.\n"
              << "[IW] Plan cost: " << plan.get_cost() << "\n"
              << "[IW] Plan length: " << plan.get_actions().size() << std::endl;
    for (size_t i = 0; i < plan.get_actions().size(); ++i)
    {
        std::cout << "[IW] " << i + 1 << ". " << plan.get_actions()[i] << std::endl;
    }
}

void DefaultIWAlgorithmEventHandler::on_unsolvable_impl() const { std::cout << "[IW] Unsolvable!" << std::endl; }

void DefaultIWAlgorithmEventHandler::on_exhausted_impl() const { std::cout << "[IW] Exhausted!" << std::endl; }

DefaultIWAlgorithmEventHandler::DefaultIWAlgorithmEventHandler(bool quiet) : IWAlgorithmEventHandlerBase<DefaultIWAlgorithmEventHandler>(quiet) {}
}