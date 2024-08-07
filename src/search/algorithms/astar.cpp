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

#include "mimir/search/algorithms/astar.hpp"

#include "mimir/search/algorithms/astar/event_handlers.hpp"
#include "mimir/search/algorithms/strategies/goal_strategy.hpp"
#include "mimir/search/algorithms/strategies/pruning_strategy.hpp"
#include "mimir/search/heuristics/interface.hpp"
#include "mimir/search/openlists/interface.hpp"
#include "mimir/search/state_repository.hpp"

namespace mimir
{

AStarAlgorithm::AStarAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator, std::shared_ptr<IHeuristic> heuristic) :
    AStarAlgorithm(applicable_action_generator, std::make_shared<StateRepository>(applicable_action_generator), std::move(heuristic))
{
}

AStarAlgorithm::AStarAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                               std::shared_ptr<StateRepository> successor_state_generator,
                               std::shared_ptr<IHeuristic> heuristic) :
    m_aag(std::move(applicable_action_generator)),
    m_ssg(std::move(successor_state_generator)),
    m_initial_state(m_ssg->get_or_create_initial_state()),
    m_heuristic(std::move(heuristic)),
    m_search_nodes(FlatSearchNodeVector<double, double>(SearchNodeBuilder<double, double>(SearchNodeStatus::NEW,
                                                                                          std::optional<State>(std::nullopt),
                                                                                          std::optional<GroundAction>(std::nullopt),
                                                                                          (double) std::numeric_limits<double>::max(),
                                                                                          (double) 0)
                                                            .get_flatmemory_builder())),
    m_openlist()
{
}

SearchStatus AStarAlgorithm::find_solution(GroundActionList& out_plan) { return find_solution(m_initial_state, out_plan); }

SearchStatus AStarAlgorithm::find_solution(State start_state, GroundActionList& out_plan)
{
    std::optional<State> unused_out_state = std::nullopt;
    return find_solution(start_state, out_plan, unused_out_state);
}

SearchStatus AStarAlgorithm::find_solution(State start_state, GroundActionList& out_plan, std::optional<State>& out_goal_state)
{
    return find_solution(start_state, std::make_unique<ProblemGoal>(m_aag->get_problem()), std::make_unique<DuplicateStatePruning>(), out_plan, out_goal_state);
}

SearchStatus AStarAlgorithm::find_solution(State start_state,
                                           std::unique_ptr<IGoalStrategy>&& goal_strategy,
                                           std::unique_ptr<IPruningStrategy>&& pruning_strategy,
                                           GroundActionList& out_plan,
                                           std::optional<State>& out_goal_state)
{
    // Clear data structures
    m_search_nodes.clear();
    m_openlist.clear();

    const auto problem = m_aag->get_problem();
    const auto& pddl_factories = *m_aag->get_pddl_factories();
    // m_event_handler->on_start_search(problem, start_state, pddl_factories);

    const auto start_g_value = double();
    const auto start_h_value = m_heuristic->compute_heuristic(start_state);
    const auto start_f_value = start_g_value + start_h_value;

    auto start_search_node = SearchNode<double, double>(this->m_search_nodes[start_state.get_index()]);
    start_search_node.get_status() = SearchNodeStatus::OPEN;
    start_search_node.get_property<0>() = start_g_value;
    start_search_node.get_property<1>() = start_h_value;

    if (!goal_strategy->test_static_goal())
    {
        // m_event_handler->on_unsolvable();

        return SearchStatus::UNSOLVABLE;
    }

    auto applicable_actions = GroundActionList {};

    if (pruning_strategy->test_prune_initial_state(start_state))
    {
        return SearchStatus::FAILED;
    }

    m_openlist.insert(start_f_value, start_state);

    auto g_value = double();

    while (!m_openlist.empty())
    {
        const auto state = m_openlist.top();
        m_openlist.pop();

        // We need this before goal test for correct statistics reporting.
        auto search_node = SearchNode<double, double>(this->m_search_nodes[state.get_index()]);
        auto& status = search_node.get_status();
        if (status == SearchNodeStatus::CLOSED)
        {
            // Optimal cost to reach state was already proven.
            continue;
        }
        status = SearchNodeStatus::CLOSED;

        // TODO: how to report progress properly?
        if (search_node.get_property<0>() > g_value)
        {
            g_value = search_node.get_property<0>();
            m_aag->on_finish_f_layer();
            // m_event_handler->on_finish_f_layer();
        }

        if (goal_strategy->test_dynamic_goal(state))
        {
            set_plan(this->m_search_nodes, ConstSearchNode<double, double>(this->m_search_nodes[state.get_index()]), out_plan);
            out_goal_state = state;
            // m_event_handler->on_end_search();
            // if (!m_event_handler->is_quiet())
            // {
            //     m_aag->on_end_search();
            // }
            // m_event_handler->on_solved(out_plan);

            return SearchStatus::SOLVED;
        }

        // m_event_handler->on_expand_state(problem, state, pddl_factories);

        this->m_aag->generate_applicable_actions(state, applicable_actions);

        for (const auto& action : applicable_actions)
        {
            const auto successor_state = this->m_ssg->get_or_create_successor_state(state, action);
            // m_event_handler->on_generate_state(problem, action, successor_state, pddl_factories);

            auto successor_search_node = SearchNode<double, double>(this->m_search_nodes[successor_state.get_index()]);
            bool is_new_successor_state = (successor_search_node.get_status() == SearchNodeStatus::NEW);
            /* Customization point 1: pruning strategy, default never prunes. */
            if (!pruning_strategy->test_prune_successor_state(state, successor_state, is_new_successor_state))
            {
                // m_event_handler->on_prune_state(problem, successor_state, pddl_factories);
                continue;
            }

            const auto new_successor_g_value = search_node.get_property<0>() + action.get_cost();
            if (new_successor_g_value < successor_search_node.get_property<0>())
            {
                successor_search_node.get_status() = SearchNodeStatus::OPEN;
                successor_search_node.get_parent_state() = state;
                successor_search_node.get_creating_action() = action;
                successor_search_node.get_property<0>() = new_successor_g_value;
                if (is_new_successor_state)
                {
                    /* Compute heuristic if state is new. */
                    successor_search_node.get_property<1>() = m_heuristic->compute_heuristic(successor_state);
                }

                /* Reopen search node with updated f_value. */
                const auto successor_f_value = successor_search_node.get_property<0>() + successor_search_node.get_property<1>();
                // m_event_handler->on_reopen_state(successor_f_value, problem, action, successor_state, pddl_factories);
                m_openlist.insert(successor_f_value, successor_state);
            }
        }
    }

    // m_event_handler->on_end_search();
    // m_event_handler->on_exhausted();

    return SearchStatus::EXHAUSTED;
}

}