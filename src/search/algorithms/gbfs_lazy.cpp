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

#include "mimir/search/algorithms/gbfs_lazy.hpp"

#include "mimir/common/timers.hpp"
#include "mimir/formalism/ground_function_expressions.hpp"
#include "mimir/formalism/metric.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/algorithms/gbfs_lazy/event_handlers.hpp"
#include "mimir/search/algorithms/strategies/exploration_strategy.hpp"
#include "mimir/search/algorithms/strategies/goal_strategy.hpp"
#include "mimir/search/algorithms/strategies/pruning_strategy.hpp"
#include "mimir/search/applicability.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/axiom_evaluators/interface.hpp"
#include "mimir/search/heuristics/interface.hpp"
#include "mimir/search/openlists/alternating.hpp"
#include "mimir/search/openlists/interface.hpp"
#include "mimir/search/openlists/priority_queue.hpp"
#include "mimir/search/plan.hpp"
#include "mimir/search/search_context.hpp"
#include "mimir/search/search_node.hpp"
#include "mimir/search/search_space.hpp"
#include "mimir/search/state_repository.hpp"

using namespace mimir::formalism;

namespace mimir::search::gbfs_lazy
{

/**
 * GBFS search node
 */

using GBFSSearchNodeImpl = SearchNodeImpl<ContinuousCost, ContinuousCost, bool, bool>;
using GBFSSearchNode = GBFSSearchNodeImpl*;
using ConstGBFSSearchNode = const GBFSSearchNodeImpl*;

static void set_g_value(GBFSSearchNode node, ContinuousCost g_value) { node->get_property<0>() = g_value; }
static void set_h_value(GBFSSearchNode node, ContinuousCost h_value) { node->get_property<1>() = h_value; }
static void set_h_preferredness(GBFSSearchNode node, bool preferred) { node->get_property<2>() = preferred; }
static void set_pi_compatibility(GBFSSearchNode node, bool compatible) { node->get_property<3>() = compatible; }

static ContinuousCost get_g_value(ConstGBFSSearchNode node) { return node->get_property<0>(); }
[[maybe_unused]] static ContinuousCost get_h_value(ConstGBFSSearchNode node) { return node->get_property<1>(); }
[[maybe_unused]] static bool get_h_preferredness(ConstGBFSSearchNode node) { return node->get_property<2>(); }
[[maybe_unused]] static bool get_pi_compatibility(ConstGBFSSearchNode node) { return node->get_property<3>(); }

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
 * GBFS
 */

SearchResult find_solution(const SearchContext& context, const Heuristic& heuristic, const Options& options)
{
    assert(heuristic);

    auto& problem = *context->get_problem();
    auto& applicable_action_generator = *context->get_applicable_action_generator();
    auto& state_repository = *context->get_state_repository();

    const auto [start_state, start_g_value] = (options.start_state) ?
                                                  std::make_pair(options.start_state, compute_state_metric_value(options.start_state, problem)) :
                                                  state_repository.get_or_create_initial_state();
    const auto event_handler = (options.event_handler) ? options.event_handler : DefaultEventHandlerImpl::create(context->get_problem());
    const auto goal_strategy = (options.goal_strategy) ? options.goal_strategy : ProblemGoalStrategyImpl::create(context->get_problem());
    const auto pruning_strategy = (options.pruning_strategy) ? options.pruning_strategy : NoPruningStrategyImpl::create();
    const auto openlist_weights = options.openlist_weights;
    const auto exploration_stategy = options.exploration_strategy;

    const auto& ground_action_repository = boost::hana::at_key(problem.get_repositories().get_hana_repositories(), boost::hana::type<GroundActionImpl> {});
    const auto& ground_axiom_repository = boost::hana::at_key(problem.get_repositories().get_hana_repositories(), boost::hana::type<GroundAxiomImpl> {});

    auto step = Index(0);

    auto result = SearchResult();

    /* Test static goal. */

    if (!goal_strategy->test_static_goal())
    {
        event_handler->on_unsolvable();

        result.status = SearchStatus::UNSOLVABLE;
        return result;
    }

    auto default_search_node = GBFSSearchNodeImpl(SearchNodeStatus::NEW, MAX_INDEX, ContinuousCost(INFINITY_CONTINUOUS_COST), ContinuousCost(0), false, false);
    auto search_nodes = SearchNodeImplVector<ContinuousCost, ContinuousCost, bool, bool>();

    /* Test whether initial state is goal. */

    if (goal_strategy->test_dynamic_goal(start_state))
    {
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

        result.plan = Plan(context, StateList { start_state }, GroundActionList {}, 0);
        result.goal_state = start_state;
        result.status = SearchStatus::SOLVED;

        event_handler->on_solved(result.plan.value());

        return result;
    }

    using GreedyOpenListType = PriorityQueue<Index, State>;
    using OpenListType = PriorityQueue<std::tuple<double, double, Index>, State>;
    auto compatible_greedy_and_preferred_openlist = GreedyOpenListType();
    auto compatible_greedy_openlist = GreedyOpenListType();
    auto compatible_exhaustive_and_preferred_openlist = OpenListType();
    auto compatible_exhaustive_openlist = OpenListType();
    auto preferred_openlist = OpenListType();
    auto standard_openlist = OpenListType();
    auto openlist = AlternatingOpenList<GreedyOpenListType, GreedyOpenListType, OpenListType, OpenListType, OpenListType, OpenListType>(
        compatible_greedy_and_preferred_openlist,
        compatible_greedy_openlist,
        compatible_exhaustive_and_preferred_openlist,
        compatible_exhaustive_openlist,
        preferred_openlist,
        standard_openlist,
        openlist_weights);

    if (start_g_value == UNDEFINED_CONTINUOUS_COST)
    {
        throw std::runtime_error("find_solution(...): evaluating the metric on the start state yielded NaN.");
    }
    const auto start_h_value = heuristic->compute_heuristic(start_state, goal_strategy->test_dynamic_goal(start_state));
    auto best_h_value = start_h_value;
    const auto start_preferred = false;

    event_handler->on_start_search(start_state, start_g_value, start_h_value);

    auto start_search_node = get_or_create_search_node(start_state->get_index(), default_search_node, search_nodes);
    start_search_node->get_status() = (start_h_value == INFINITY_CONTINUOUS_COST) ? SearchNodeStatus::DEAD_END : SearchNodeStatus::OPEN;
    set_g_value(start_search_node, start_g_value);
    set_h_value(start_search_node, start_h_value);
    set_h_preferredness(start_search_node, start_preferred);
    set_pi_compatibility(start_search_node, false);

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

    const auto use_exploration_strategy = std::all_of(options.openlist_weights.begin(), options.openlist_weights.begin() + 4, [](double w) { return w > 0; });
    auto applicable_actions = GroundActionList {};
    standard_openlist.insert(std::make_tuple(start_h_value, start_g_value, step++), start_state);

    auto stopwatch = StopWatch(options.max_time_in_ms);
    stopwatch.start();

    while (!openlist.empty())
    {
        if (stopwatch.has_finished())
        {
            result.status = SearchStatus::OUT_OF_TIME;
            return result;
        }

        const auto state = openlist.top();
        openlist.pop();

        auto search_node = get_or_create_search_node(state->get_index(), default_search_node, search_nodes);

        /* Close state. */

        if (search_node->get_status() == SearchNodeStatus::CLOSED || search_node->get_status() == SearchNodeStatus::DEAD_END)
        {
            continue;
        }

        const auto state_h_value = heuristic->compute_heuristic(state, search_node->get_status() == SearchNodeStatus::GOAL);
        set_h_value(search_node, state_h_value);
        if (state_h_value == INFINITY_CONTINUOUS_COST)
        {
            search_node->get_status() = SearchNodeStatus::DEAD_END;
            continue;
        }

        if (state_h_value < best_h_value)
        {
            best_h_value = state_h_value;
            event_handler->on_new_best_h_value(best_h_value);
        }

        const auto& preferred_actions = heuristic->get_preferred_actions();
        // Ensure that preferred actions are applicable.
        assert(std::all_of(preferred_actions.data.begin(), preferred_actions.data.end(), [&](auto&& action) { return is_applicable(action, problem, state); }));

        /* Expand the successors of the state. */

        event_handler->on_expand_state(state);

        /* Ensure that the state is closed */

        search_node->get_status() = SearchNodeStatus::CLOSED;

        auto first_compatible = true;

        for (const auto& action : applicable_action_generator.create_applicable_action_generator(state))
        {
            const auto [successor_state, successor_state_metric_value] =
                state_repository.get_or_create_successor_state(state, action, get_g_value(search_node));
            auto successor_search_node = get_or_create_search_node(successor_state->get_index(), default_search_node, search_nodes);
            const auto action_cost = successor_state_metric_value - get_g_value(search_node);

            if (successor_state_metric_value == UNDEFINED_CONTINUOUS_COST)
            {
                throw std::runtime_error("find_solution(...): evaluating the metric on the successor state yielded NaN.");
            }

            const auto is_preferred = preferred_actions.data.contains(action);
            const auto is_new_successor_state = (successor_search_node->get_status() == SearchNodeStatus::NEW);

            if (is_new_successor_state && search_nodes.size() >= options.max_num_states)
            {
                result.status = SearchStatus::OUT_OF_STATES;
                return result;
            }

            /* Skip previously generated state. */

            if (!is_new_successor_state)
            {
                if (get_pi_compatibility(successor_search_node))
                {
                    first_compatible = false;
                }
                continue;
            }

            /* Customization point 1: pruning strategy, default never prunes. */

            if (pruning_strategy->test_prune_successor_state(state, successor_state, is_new_successor_state))
            {
                event_handler->on_prune_state(successor_state);
                continue;
            }

            /* Open new state. */

            successor_search_node->get_status() = SearchNodeStatus::OPEN;
            successor_search_node->get_parent_state() = state->get_index();
            set_g_value(successor_search_node, successor_state_metric_value);
            set_h_value(successor_search_node, state_h_value);
            set_h_preferredness(successor_search_node, is_preferred);

            /* Early goal test. */

            const auto successor_is_goal_state = goal_strategy->test_dynamic_goal(successor_state);

            if (successor_is_goal_state)
            {
                successor_search_node->get_status() = SearchNodeStatus::GOAL;

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

                result.plan =
                    extract_total_ordered_plan(start_state, start_g_value, successor_search_node, successor_state->get_index(), search_nodes, context);
                assert(result.plan->get_cost() == successor_state_metric_value);
                result.goal_state = state;
                result.status = SearchStatus::SOLVED;

                event_handler->on_solved(result.plan.value());

                return result;
            }

            event_handler->on_generate_state(state, action, action_cost, successor_state);

            /* Exploration strategy */

            auto is_compatible = false;

            if (exploration_stategy && use_exploration_strategy)
            {
                is_compatible = exploration_stategy->on_generate_state(state, action, successor_state);
                set_pi_compatibility(successor_search_node, is_compatible);
            }

            if (options.openlist_weights[5] > 0 && is_compatible && is_preferred && first_compatible)
            {
                first_compatible = false;
                compatible_greedy_and_preferred_openlist.insert(step++, successor_state);
            }
            else if (options.openlist_weights[4] > 0 && is_compatible && first_compatible)
            {
                first_compatible = false;
                compatible_greedy_openlist.insert(step++, successor_state);
            }
            else if (options.openlist_weights[3] > 0 && is_compatible && is_preferred)
            {
                compatible_exhaustive_and_preferred_openlist.insert(std::make_tuple(state_h_value, successor_state_metric_value, step++), successor_state);
            }
            else if (options.openlist_weights[2] > 0 && is_compatible)
            {
                compatible_exhaustive_openlist.insert(std::make_tuple(state_h_value, successor_state_metric_value, step++), successor_state);
            }
            else if (options.openlist_weights[1] > 0 && is_preferred)
            {
                preferred_openlist.insert(std::make_tuple(state_h_value, successor_state_metric_value, step++), successor_state);
            }
            else
            {
                standard_openlist.insert(std::make_tuple(state_h_value, successor_state_metric_value, step++), successor_state);
            }
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
