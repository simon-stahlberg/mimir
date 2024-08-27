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
#include "mimir/search/openlists/priority_queue.hpp"
#include "mimir/search/search_node.hpp"
#include "mimir/search/state_repository.hpp"

namespace mimir
{

/**
 * AStar search node
 */

struct AStarSearchNodeTag
{
};

using GValue = double;
using HValue = double;

using AStarSearchNodeBuilder = SearchNodeBuilder<AStarSearchNodeTag, GValue, HValue>;
using AStarSearchNode = SearchNode<AStarSearchNodeTag, GValue, HValue>;
using ConstAStarSearchNode = ConstSearchNode<AStarSearchNodeTag, GValue, HValue>;

static void set_g_value(AStarSearchNode search_node, GValue g_value) { return search_node.set_property<0>(g_value); }
static void set_h_value(AStarSearchNode search_node, GValue g_value) { return search_node.set_property<1>(g_value); }

static GValue get_g_value(AStarSearchNode search_node) { return search_node.get_property<0>(); }
static HValue get_h_value(AStarSearchNode search_node) { return search_node.get_property<1>(); }

/**
 * AStar
 */

AStarAlgorithm::AStarAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator, std::shared_ptr<IHeuristic> heuristic) :
    AStarAlgorithm(applicable_action_generator,
                   std::make_shared<StateRepository>(applicable_action_generator),
                   std::move(heuristic),
                   std::make_shared<DefaultAStarAlgorithmEventHandler>())
{
}

AStarAlgorithm::AStarAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                               std::shared_ptr<StateRepository> successor_state_generator,
                               std::shared_ptr<IHeuristic> heuristic,
                               std::shared_ptr<IAStarAlgorithmEventHandler> event_handler) :
    m_aag(std::move(applicable_action_generator)),
    m_ssg(std::move(successor_state_generator)),
    m_heuristic(std::move(heuristic)),
    m_event_handler(std::move(event_handler))
{
}

SearchStatus AStarAlgorithm::find_solution(GroundActionList& out_plan) { return find_solution(m_ssg->get_or_create_initial_state(), out_plan); }

SearchStatus AStarAlgorithm::find_solution(State start_state, GroundActionList& out_plan)
{
    std::optional<State> unused_out_state = std::nullopt;
    return find_solution(start_state, out_plan, unused_out_state);
}

SearchStatus AStarAlgorithm::find_solution(State start_state, GroundActionList& out_plan, std::optional<State>& out_goal_state)
{
    return find_solution(start_state, std::make_unique<ProblemGoal>(m_aag->get_problem()), std::make_unique<NoStatePruning>(), out_plan, out_goal_state);
}

SearchStatus AStarAlgorithm::find_solution(State start_state,
                                           std::unique_ptr<IGoalStrategy>&& goal_strategy,
                                           std::unique_ptr<IPruningStrategy>&& pruning_strategy,
                                           GroundActionList& out_plan,
                                           std::optional<State>& out_goal_state)
{
    auto search_nodes =
        FlatSearchNodeVector<double, double>(FlatSearchNodeVector<GValue, HValue>(AStarSearchNodeBuilder(SearchNodeStatus::NEW,
                                                                                                         std::numeric_limits<StateIndex>::max(),
                                                                                                         std::numeric_limits<GroundActionIndex>::max(),
                                                                                                         std::numeric_limits<double>::infinity(),
                                                                                                         double(0))
                                                                                      .get_flatmemory_builder()));
    auto openlist = PriorityQueue<State>();

    const auto problem = m_aag->get_problem();
    const auto& pddl_factories = *m_aag->get_pddl_factories();
    m_event_handler->on_start_search(start_state, problem, pddl_factories);

    const auto start_g_value = GValue();
    const auto start_h_value = m_heuristic->compute_heuristic(start_state);
    const auto start_f_value = start_g_value + start_h_value;

    const auto flat_start_search_node = search_nodes[start_state.get_index()];
    auto start_search_node = AStarSearchNode(flat_start_search_node);
    set_status(start_search_node, (start_h_value == std::numeric_limits<HValue>::infinity()) ? SearchNodeStatus::DEAD_END : SearchNodeStatus::OPEN);
    set_g_value(start_search_node, start_g_value);
    set_h_value(start_search_node, start_h_value);

    /* Test whether start state is deadend. */

    if (start_search_node.get_status() == SearchNodeStatus::DEAD_END)
    {
        m_event_handler->on_unsolvable();

        return SearchStatus::UNSOLVABLE;
    }

    /* Test static goal. */

    if (!goal_strategy->test_static_goal())
    {
        m_event_handler->on_unsolvable();

        return SearchStatus::UNSOLVABLE;
    }

    /* Test pruning of start state. */

    if (pruning_strategy->test_prune_initial_state(start_state))
    {
        return SearchStatus::FAILED;
    }

    auto applicable_actions = GroundActionList {};
    auto f_value = GValue();
    openlist.insert(start_f_value, start_state);

    while (!openlist.empty())
    {
        const auto state = openlist.top();
        openlist.pop();

        const auto flat_search_node = search_nodes[state.get_index()];
        auto search_node = AStarSearchNode(flat_search_node);

        /* Avoid unnecessary extra work by testing whether shortest distance was proven. */

        if (search_node.get_status() == SearchNodeStatus::CLOSED)
        {
            continue;
        }

        /* Report search progress. */

        const auto search_node_f_value = get_g_value(search_node) + get_h_value(search_node);

        if (search_node_f_value > f_value)
        {
            f_value = search_node_f_value;
            m_aag->on_finish_search_layer();
            m_event_handler->on_finish_f_layer(f_value);
        }

        /* Test whether state is a deadend. */

        if (search_node.get_status() == SearchNodeStatus::DEAD_END)
        {
            m_event_handler->on_unsolvable();

            return SearchStatus::UNSOLVABLE;
        }

        /* Test whether state achieves the dynamic goal. */

        if (goal_strategy->test_dynamic_goal(state))
        {
            set_plan(search_nodes, m_aag->get_ground_actions(), ConstAStarSearchNode(flat_search_node), out_plan);
            out_goal_state = state;
            m_event_handler->on_end_search();
            if (!m_event_handler->is_quiet())
            {
                m_aag->on_end_search();
            }
            m_event_handler->on_solved(out_plan);

            return SearchStatus::SOLVED;
        }

        /* Expand the successors of the state. */

        m_event_handler->on_expand_state(state, problem, pddl_factories);

        m_aag->generate_applicable_actions(state, applicable_actions);

        for (const auto& action : applicable_actions)
        {
            const auto successor_state = m_ssg->get_or_create_successor_state(state, action);
            const auto flat_successors_search_node = search_nodes[successor_state.get_index()];
            auto successor_search_node = AStarSearchNode(flat_successors_search_node);

            m_event_handler->on_generate_state(successor_state, action, problem, pddl_factories);

            const bool is_new_successor_state = (successor_search_node.get_status() == SearchNodeStatus::NEW);

            /* Customization point 1: pruning strategy, default never prunes. */

            if (pruning_strategy->test_prune_successor_state(state, successor_state, is_new_successor_state))
            {
                m_event_handler->on_prune_state(successor_state, problem, pddl_factories);
                continue;
            }

            /* Check whether state must be reopened or not. */

            const auto new_successor_g_value = get_g_value(search_node) + action.get_cost();
            if (new_successor_g_value < get_g_value(successor_search_node))
            {
                /* Open/Reopen state with updated f_value. */

                set_status(successor_search_node, SearchNodeStatus::OPEN);
                set_parent_state(successor_search_node, state.get_index());
                set_creating_action(successor_search_node, action.get_index());
                set_g_value(successor_search_node, new_successor_g_value);
                if (is_new_successor_state)
                {
                    // Compute heuristic if state is new.
                    const auto successor_h_value = m_heuristic->compute_heuristic(successor_state);
                    set_h_value(successor_search_node, successor_h_value);

                    if (successor_h_value == std::numeric_limits<HValue>::infinity())
                    {
                        set_status(successor_search_node, SearchNodeStatus::DEAD_END);
                        continue;
                    }
                }
                m_event_handler->on_generate_state_relaxed(successor_state, action, problem, pddl_factories);

                const auto successor_f_value = get_g_value(successor_search_node) + get_h_value(successor_search_node);
                openlist.insert(successor_f_value, successor_state);
            }
            else
            {
                m_event_handler->on_generate_state_not_relaxed(successor_state, action, problem, pddl_factories);
            }
        }

        /* Close state. */

        search_node.set_status(SearchNodeStatus::CLOSED);
        m_event_handler->on_close_state(state, problem, pddl_factories);
    }

    m_event_handler->on_end_search();
    m_event_handler->on_exhausted();

    return SearchStatus::EXHAUSTED;
}

}