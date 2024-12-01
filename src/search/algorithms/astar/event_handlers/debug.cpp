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

#include "mimir/search/algorithms/astar/event_handlers/debug.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/search/plan.hpp"

namespace mimir
{
void DebugAStarAlgorithmEventHandler::on_expand_state_impl(State state, Problem problem, const PDDLRepositories& pddl_repositories) const
{
    std::cout << "[AStar] ----------------------------------------\n"
              << "[AStar] State: " << std::make_tuple(problem, state, std::cref(pddl_repositories)) << std::endl
              << std::endl;
}

void DebugAStarAlgorithmEventHandler::on_generate_state_impl(State state,
                                                             GroundAction action,
                                                             ContinuousCost action_cost,
                                                             Problem problem,
                                                             const PDDLRepositories& pddl_repositories) const
{
    std::cout << "[AStar] Action: " << std::make_tuple(action, std::cref(pddl_repositories), FullActionFormatterTag {}) << " cost:" << action_cost << "\n"
              << "[AStar] Successor: " << std::make_tuple(problem, state, std::cref(pddl_repositories)) << "\n"
              << std::endl;
}

void DebugAStarAlgorithmEventHandler::on_generate_state_relaxed_impl(State state,
                                                                     GroundAction action,
                                                                     ContinuousCost action_cost,
                                                                     Problem problem,
                                                                     const PDDLRepositories& pddl_repositories) const
{
}

void DebugAStarAlgorithmEventHandler::on_generate_state_not_relaxed_impl(State state,
                                                                         GroundAction action,
                                                                         ContinuousCost action_cost,
                                                                         Problem problem,
                                                                         const PDDLRepositories& pddl_repositories) const
{
}

void DebugAStarAlgorithmEventHandler::on_close_state_impl(State state, Problem problem, const PDDLRepositories& pddl_repositories) const {}

void DebugAStarAlgorithmEventHandler::on_finish_f_layer_impl(double f_value, uint64_t num_expanded_states, uint64_t num_generated_states) const
{
    std::cout << "[AStar] Finished state expansion until f-layer " << f_value << " with num expanded states " << num_expanded_states
              << " and num generated states " << num_generated_states << std::endl;
}

void DebugAStarAlgorithmEventHandler::on_prune_state_impl(State state, Problem problem, const PDDLRepositories& pddl_repositories) const {}

void DebugAStarAlgorithmEventHandler::on_start_search_impl(State start_state, Problem problem, const PDDLRepositories& pddl_repositories) const
{
    std::cout << "[AStar] Search started.\n"
              << "[AStar] Initial: " << std::make_tuple(problem, start_state, std::cref(pddl_repositories)) << std::endl;
}

void DebugAStarAlgorithmEventHandler::on_end_search_impl() const { std::cout << "[AStar] Search ended.\n" << m_statistics << std::endl; }

void DebugAStarAlgorithmEventHandler::on_solved_impl(const Plan& plan, const PDDLRepositories& pddl_repositories) const
{
    std::cout << "[AStar] Plan found.\n"
              << "[AStar] Plan cost: " << plan.get_cost() << "\n"
              << "[AStar] Plan length: " << plan.get_actions().size() << std::endl;
    for (size_t i = 0; i < plan.get_actions().size(); ++i)
    {
        std::cout << "[AStar] " << i + 1 << ". " << std::make_tuple(plan.get_actions()[i], std::cref(pddl_repositories), PlanActionFormatterTag {})
                  << std::endl;
    }
}

void DebugAStarAlgorithmEventHandler::on_unsolvable_impl() const { std::cout << "[AStar] Unsolvable!" << std::endl; }

void DebugAStarAlgorithmEventHandler::on_exhausted_impl() const { std::cout << "[AStar] Exhausted!" << std::endl; }
}
