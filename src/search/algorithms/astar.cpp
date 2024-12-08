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
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/grounding/action_grounder.hpp"
#include "mimir/search/heuristics/interface.hpp"
#include "mimir/search/openlists/interface.hpp"
#include "mimir/search/openlists/priority_queue.hpp"
#include "mimir/search/plan.hpp"
#include "mimir/search/search_node.hpp"
#include "mimir/search/state_repository.hpp"

namespace mimir
{

/**
 * AStar search node
 */

using AStarSearchNodeImpl = SearchNodeImpl<ContinuousCost, ContinuousCost>;
using AStarSearchNode = AStarSearchNodeImpl*;
using ConstAStarSearchNode = const AStarSearchNodeImpl*;

static void set_g_value(AStarSearchNode node, ContinuousCost g_value) { return set_property<0>(node, g_value); }
static void set_h_value(AStarSearchNode node, ContinuousCost h_value) { return set_property<1>(node, h_value); }

static ContinuousCost get_g_value(ConstAStarSearchNode node) { return get_property<0>(node); }
static ContinuousCost get_h_value(ConstAStarSearchNode node) { return get_property<1>(node); }

static AStarSearchNode
get_or_create_search_node(size_t state_index, const AStarSearchNodeImpl& default_node, cista::storage::Vector<AStarSearchNodeImpl>& search_nodes)
{
    while (state_index >= search_nodes.size())
    {
        search_nodes.push_back(default_node);
    }
    return search_nodes[state_index];
}

/**
 * AStar
 */

AStarAlgorithm::AStarAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                               std::shared_ptr<StateRepository> state_repository,
                               std::shared_ptr<IHeuristic> heuristic) :
    AStarAlgorithm(std::move(applicable_action_generator),
                   std::move(state_repository),
                   std::move(heuristic),
                   std::make_shared<DefaultAStarAlgorithmEventHandler>())
{
}

AStarAlgorithm::AStarAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                               std::shared_ptr<StateRepository> state_repository,
                               std::shared_ptr<IHeuristic> heuristic,
                               std::shared_ptr<IAStarAlgorithmEventHandler> event_handler) :
    m_applicable_action_generator(std::move(applicable_action_generator)),
    m_state_repository(std::move(state_repository)),
    m_heuristic(std::move(heuristic)),
    m_event_handler(std::move(event_handler))
{
}

SearchResult AStarAlgorithm::find_solution() { return find_solution(m_state_repository->get_or_create_initial_state()); }

SearchResult AStarAlgorithm::find_solution(State start_state)
{
    return find_solution(start_state,
                         std::make_unique<ProblemGoal>(m_applicable_action_generator->get_action_grounder().get_problem()),
                         std::make_unique<NoStatePruning>());
}

SearchResult
AStarAlgorithm::find_solution(State start_state, std::unique_ptr<IGoalStrategy>&& goal_strategy, std::unique_ptr<IPruningStrategy>&& pruning_strategy)
{
    auto result = SearchResult();
    auto default_search_node = AStarSearchNodeImpl { SearchNodeStatus::NEW,
                                                     std::numeric_limits<Index>::max(),
                                                     std::numeric_limits<Index>::max(),
                                                     std::numeric_limits<ContinuousCost>::infinity(),
                                                     ContinuousCost(0) };
    auto search_nodes = cista::storage::Vector<AStarSearchNodeImpl>();

    auto openlist = PriorityQueue<State>();

    const auto problem = m_applicable_action_generator->get_action_grounder().get_problem();
    const auto& pddl_repositories = *m_applicable_action_generator->get_action_grounder().get_pddl_repositories();
    m_event_handler->on_start_search(start_state, problem, pddl_repositories);

    const auto start_g_value = ContinuousCost(0);
    const auto start_h_value = m_heuristic->compute_heuristic(start_state);
    const auto start_f_value = start_g_value + start_h_value;

    auto start_search_node = get_or_create_search_node(start_state->get_index(), default_search_node, search_nodes);
    set_status(start_search_node, (start_h_value == std::numeric_limits<ContinuousCost>::infinity()) ? SearchNodeStatus::DEAD_END : SearchNodeStatus::OPEN);
    set_g_value(start_search_node, start_g_value);
    set_h_value(start_search_node, start_h_value);

    /* Test whether start state is deadend. */

    if (get_status(start_search_node) == SearchNodeStatus::DEAD_END)
    {
        m_event_handler->on_unsolvable();

        result.status = SearchStatus::UNSOLVABLE;
        return result;
    }

    /* Test static goal. */

    if (!goal_strategy->test_static_goal())
    {
        m_event_handler->on_unsolvable();

        result.status = SearchStatus::UNSOLVABLE;
        return result;
    }

    /* Test pruning of start state. */

    if (pruning_strategy->test_prune_initial_state(start_state))
    {
        result.status = SearchStatus::FAILED;
        return result;
    }

    auto applicable_actions = GroundActionList {};
    auto f_value = ContinuousCost(0);
    openlist.insert(start_f_value, start_state);

    while (!openlist.empty())
    {
        const auto state = openlist.top();
        openlist.pop();

        auto search_node = get_or_create_search_node(state->get_index(), default_search_node, search_nodes);

        /* Avoid unnecessary extra work by testing whether shortest distance was proven. */

        if (get_status(search_node) == SearchNodeStatus::CLOSED)
        {
            continue;
        }

        /* Report search progress. */

        const auto search_node_f_value = get_g_value(search_node) + get_h_value(search_node);

        if (search_node_f_value > f_value)
        {
            f_value = search_node_f_value;
            m_applicable_action_generator->on_finish_search_layer();
            m_state_repository->get_axiom_evaluator()->on_finish_search_layer();
            m_event_handler->on_finish_f_layer(f_value);
        }

        /* Test whether state is a deadend. */

        if (get_status(search_node) == SearchNodeStatus::DEAD_END)
        {
            m_event_handler->on_unsolvable();

            result.status = SearchStatus::UNSOLVABLE;
            return result;
        }

        /* Test whether state achieves the dynamic goal. */

        if (goal_strategy->test_dynamic_goal(state))
        {
            auto plan_actions = GroundActionList {};
            set_plan(search_nodes, m_applicable_action_generator->get_action_grounder().get_ground_actions(), search_node, plan_actions);
            result.plan = Plan(std::move(plan_actions), get_g_value(search_node));
            result.goal_state = state;
            m_event_handler->on_end_search();
            if (!m_event_handler->is_quiet())
            {
                m_applicable_action_generator->on_end_search();
                m_state_repository->get_axiom_evaluator()->on_end_search();
            }
            m_event_handler->on_solved(result.plan.value(), pddl_repositories);

            result.status = SearchStatus::SOLVED;
            return result;
        }

        /* Expand the successors of the state. */

        m_event_handler->on_expand_state(state, problem, pddl_repositories);

        for (const auto& action : m_applicable_action_generator->create_applicable_action_generator(state))
        {
            const auto [successor_state, action_cost] = m_state_repository->get_or_create_successor_state(state, action);
            auto successor_search_node = get_or_create_search_node(successor_state->get_index(), default_search_node, search_nodes);

            m_event_handler->on_generate_state(successor_state, action, action_cost, problem, pddl_repositories);

            const bool is_new_successor_state = (get_status(successor_search_node) == SearchNodeStatus::NEW);

            /* Customization point 1: pruning strategy, default never prunes. */

            if (pruning_strategy->test_prune_successor_state(state, successor_state, is_new_successor_state))
            {
                m_event_handler->on_prune_state(successor_state, problem, pddl_repositories);
                continue;
            }

            /* Check whether state must be reopened or not. */

            const auto new_successor_g_value = get_g_value(search_node) + action_cost;
            if (new_successor_g_value < get_g_value(successor_search_node))
            {
                /* Open/Reopen state with updated f_value. */

                set_status(successor_search_node, SearchNodeStatus::OPEN);
                set_parent_state(successor_search_node, state->get_index());
                set_creating_action(successor_search_node, action->get_index());
                set_g_value(successor_search_node, new_successor_g_value);
                if (is_new_successor_state)
                {
                    // Compute heuristic if state is new.
                    const auto successor_h_value = m_heuristic->compute_heuristic(successor_state);
                    set_h_value(successor_search_node, successor_h_value);

                    if (successor_h_value == std::numeric_limits<ContinuousCost>::infinity())
                    {
                        set_status(successor_search_node, SearchNodeStatus::DEAD_END);
                        continue;
                    }
                }
                m_event_handler->on_generate_state_relaxed(successor_state, action, action_cost, problem, pddl_repositories);

                const auto successor_f_value = get_g_value(successor_search_node) + get_h_value(successor_search_node);
                openlist.insert(successor_f_value, successor_state);
            }
            else
            {
                m_event_handler->on_generate_state_not_relaxed(successor_state, action, action_cost, problem, pddl_repositories);
            }
        }

        /* Close state. */

        set_status(search_node, SearchNodeStatus::CLOSED);
        m_event_handler->on_close_state(state, problem, pddl_repositories);
    }

    m_event_handler->on_end_search();
    m_event_handler->on_exhausted();

    result.status = SearchStatus::EXHAUSTED;
    return result;
}

}