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

#include "mimir/search/algorithms/gbfs.hpp"

#include "mimir/formalism/ground_function_expressions.hpp"
#include "mimir/formalism/metric.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/algorithms/gbfs/event_handlers.hpp"
#include "mimir/search/algorithms/strategies/goal_strategy.hpp"
#include "mimir/search/algorithms/strategies/pruning_strategy.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/axiom_evaluators/interface.hpp"
#include "mimir/search/heuristics/interface.hpp"
#include "mimir/search/openlists/interface.hpp"
#include "mimir/search/openlists/priority_queue.hpp"
#include "mimir/search/plan.hpp"
#include "mimir/search/search_context.hpp"
#include "mimir/search/search_node.hpp"
#include "mimir/search/search_space.hpp"
#include "mimir/search/state_repository.hpp"

using namespace mimir::formalism;

namespace mimir::search::gbfs
{

/**
 * AStar search node
 */

using GBFSSearchNodeImpl = SearchNodeImpl<ContinuousCost, ContinuousCost>;
using GBFSSearchNode = GBFSSearchNodeImpl*;
using ConstGBFSSearchNode = const GBFSSearchNodeImpl*;

static void set_g_value(GBFSSearchNode node, ContinuousCost g_value) { node->get_property<0>() = g_value; }
static void set_h_value(GBFSSearchNode node, ContinuousCost h_value) { node->get_property<1>() = h_value; }

static ContinuousCost get_g_value(ConstGBFSSearchNode node) { return node->get_property<0>(); }
static ContinuousCost get_h_value(ConstGBFSSearchNode node) { return node->get_property<1>(); }

static GBFSSearchNode
get_or_create_search_node(size_t state_index, const GBFSSearchNodeImpl& default_node, mimir::buffering::Vector<GBFSSearchNodeImpl>& search_nodes)
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

SearchResult find_solution(const SearchContext& context,
                           const Heuristic& heuristic,
                           State start_state_,
                           EventHandler event_handler_,
                           GoalStrategy goal_strategy_,
                           PruningStrategy pruning_strategy_)
{
    assert(heuristic);

    auto& problem = *context->get_problem();
    auto& applicable_action_generator = *context->get_applicable_action_generator();
    auto& state_repository = *context->get_state_repository();

    const auto [start_state, start_g_value] =
        (start_state_) ? std::make_pair(start_state_, compute_state_metric_value(start_state_, problem)) : state_repository.get_or_create_initial_state();
    const auto event_handler = (event_handler_) ? event_handler_ : DefaultEventHandlerImpl::create(context->get_problem());
    const auto goal_strategy = (goal_strategy_) ? goal_strategy_ : ProblemGoalStrategyImpl::create(context->get_problem());
    const auto pruning_strategy = (pruning_strategy_) ? pruning_strategy_ : NoPruningStrategyImpl::create();

    auto result = SearchResult();

    /* Test static goal. */

    if (!goal_strategy->test_static_goal())
    {
        event_handler->on_unsolvable();

        result.status = SearchStatus::UNSOLVABLE;
        return result;
    }

    auto default_search_node =
        GBFSSearchNodeImpl(SearchNodeStatus::NEW,
                           std::numeric_limits<Index>::max(),
                           cista::tuple<ContinuousCost, ContinuousCost> { std::numeric_limits<ContinuousCost>::infinity(), ContinuousCost(0) });
    auto search_nodes = SearchNodeImplVector<ContinuousCost, ContinuousCost>();

    auto openlist = PriorityQueue<State>();

    event_handler->on_start_search(start_state);

    if (start_g_value == UNDEFINED_CONTINUOUS_COST)
    {
        throw std::runtime_error("find_solution_astar(...): evaluating the metric on the start state yielded NaN.");
    }
    const auto start_h_value = heuristic->compute_heuristic(start_state, goal_strategy->test_dynamic_goal(start_state));

    auto start_search_node = get_or_create_search_node(start_state->get_index(), default_search_node, search_nodes);
    start_search_node->get_status() = (start_h_value == std::numeric_limits<ContinuousCost>::infinity()) ? SearchNodeStatus::DEAD_END : SearchNodeStatus::OPEN;
    set_g_value(start_search_node, start_g_value);
    set_h_value(start_search_node, start_h_value);

    /* Test whether start state is deadend. */

    if (start_search_node->get_status() == SearchNodeStatus::DEAD_END)
    {
        event_handler->on_unsolvable();

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
    openlist.insert(start_h_value, start_state);

    const auto& ground_action_repository = boost::hana::at_key(problem.get_repositories().get_hana_repositories(), boost::hana::type<GroundActionImpl> {});
    const auto& ground_axiom_repository = boost::hana::at_key(problem.get_repositories().get_hana_repositories(), boost::hana::type<GroundAxiomImpl> {});

    while (!openlist.empty())
    {
        const auto state = openlist.top();
        openlist.pop();

        auto search_node = get_or_create_search_node(state->get_index(), default_search_node, search_nodes);

        /* Avoid unnecessary extra work by testing whether shortest distance was proven. */

        if (search_node->get_status() == SearchNodeStatus::CLOSED || search_node->get_status() == SearchNodeStatus::DEAD_END)
        {
            continue;
        }

        /* Test whether state achieves the dynamic goal. */

        if (search_node->get_status() == SearchNodeStatus::GOAL)
        {
            event_handler->on_expand_goal_state(state);

            event_handler->on_end_search(state_repository.get_reached_fluent_ground_atoms_bitset().count(),
                                         state_repository.get_reached_derived_ground_atoms_bitset().count(),
                                         problem.get_estimated_memory_usage_in_bytes(),
                                         search_nodes.get_estimated_memory_usage_in_bytes(),
                                         state_repository.get_state_count(),
                                         search_nodes.size(),
                                         ground_action_repository.size(),
                                         ground_axiom_repository.size());

            applicable_action_generator.on_end_search();
            state_repository.get_axiom_evaluator()->on_end_search();

            auto plan_actions = GroundActionList {};
            auto state_trajectory = IndexList {};
            extract_state_trajectory(search_nodes, search_node, state->get_index(), state_trajectory);
            const auto final_state_metric_value =
                extract_ground_action_sequence(start_state, start_g_value, state_trajectory, applicable_action_generator, state_repository, plan_actions);
            assert(final_state_metric_value == get_g_value(search_node));
            result.plan = Plan(std::move(plan_actions), final_state_metric_value);
            result.goal_state = state;
            result.status = SearchStatus::SOLVED;

            event_handler->on_solved(result.plan.value());

            return result;
        }

        /* Expand the successors of the state. */

        event_handler->on_expand_state(state);

        for (const auto& action : applicable_action_generator.create_applicable_action_generator(state))
        {
            const auto [successor_state, successor_state_metric_value] =
                state_repository.get_or_create_successor_state(state, action, get_g_value(search_node));
            auto successor_search_node = get_or_create_search_node(successor_state->get_index(), default_search_node, search_nodes);
            const auto action_cost = successor_state_metric_value - get_g_value(search_node);

            if (successor_state_metric_value == UNDEFINED_CONTINUOUS_COST)
            {
                throw std::runtime_error("find_solution_astar(...): evaluating the metric on the successor state yielded NaN.");
            }

            const bool is_new_successor_state = (successor_search_node->get_status() == SearchNodeStatus::NEW);

            /* Skip visited states. */

            if (!is_new_successor_state)
            {
                continue;
            }

            /* Close state. */

            search_node->get_status() = SearchNodeStatus::CLOSED;

            /* Customization point 1: pruning strategy, default never prunes. */

            if (pruning_strategy->test_prune_successor_state(state, successor_state, is_new_successor_state))
            {
                event_handler->on_prune_state(successor_state);
                continue;
            }

            /* Open state with updated h_value. */

            successor_search_node->get_status() = SearchNodeStatus::OPEN;
            successor_search_node->get_parent_state() = state->get_index();
            set_g_value(successor_search_node, successor_state_metric_value);

            // Compute heuristic if state is new.
            const auto successor_is_goal_state = goal_strategy->test_dynamic_goal(successor_state);
            if (successor_is_goal_state)
            {
                successor_search_node->get_status() = SearchNodeStatus::GOAL;
            }
            const auto successor_h_value = heuristic->compute_heuristic(successor_state, successor_is_goal_state);
            set_h_value(successor_search_node, successor_h_value);

            if (successor_h_value == std::numeric_limits<ContinuousCost>::infinity())
            {
                successor_search_node->get_status() = SearchNodeStatus::DEAD_END;
                continue;
            }

            event_handler->on_generate_state(state, action, action_cost, successor_state);

            openlist.insert(successor_h_value, successor_state);
        }
    }

    event_handler->on_end_search(state_repository.get_reached_fluent_ground_atoms_bitset().count(),
                                 state_repository.get_reached_derived_ground_atoms_bitset().count(),
                                 problem.get_estimated_memory_usage_in_bytes(),
                                 search_nodes.get_estimated_memory_usage_in_bytes(),
                                 state_repository.get_state_count(),
                                 search_nodes.size(),
                                 ground_action_repository.size(),
                                 ground_axiom_repository.size());
    event_handler->on_exhausted();

    result.status = SearchStatus::EXHAUSTED;
    return result;
}
}