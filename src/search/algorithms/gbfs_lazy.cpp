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

#include "mimir/common/segmented_vector.hpp"
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

struct SearchNode
{
    ContinuousCost g_value;
    ContinuousCost h_value;
    Index parent_state;
    SearchNodeStatus status;
    bool preferred;
    bool compatible;
};

static_assert(sizeof(SearchNode) == 24);

using SearchNodeVector = SegmentedVector<SearchNode>;

static SearchNode& get_or_create_search_node(size_t state_index, SearchNodeVector& search_nodes)
{
    static constexpr auto default_node =
        SearchNode { ContinuousCost(INFINITY_CONTINUOUS_COST), ContinuousCost(0), MAX_INDEX, SearchNodeStatus::NEW, false, false };

    while (state_index >= search_nodes.size())
    {
        search_nodes.push_back(default_node);
    }
    return search_nodes[state_index];
}

/**
 * GBFS queue
 */

struct GreedyQueueEntry
{
    using KeyType = std::tuple<Index, SearchNodeStatus>;
    using ItemType = PackedState;

    PackedState packed_state;
    Index step;
    SearchNodeStatus status;

    KeyType get_key() const { return std::make_tuple(step, status); }
    ItemType get_item() const { return packed_state; }
};

static_assert(sizeof(GreedyQueueEntry) == 16);

struct ExhaustiveQueueEntry
{
    using KeyType = std::tuple<ContinuousCost, ContinuousCost, Index, SearchNodeStatus>;
    using ItemType = PackedState;

    ContinuousCost g_value;
    ContinuousCost h_value;
    PackedState packed_state;
    Index step;
    SearchNodeStatus status;

    KeyType get_key() const { return std::make_tuple(h_value, g_value, step, status); }
    ItemType get_item() const { return packed_state; }
};

static_assert(sizeof(ExhaustiveQueueEntry) == 32);

using GreedyQueue = PriorityQueue<GreedyQueueEntry>;
using ExhaustiveQueue = PriorityQueue<ExhaustiveQueueEntry>;

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
                                                  std::make_pair(options.start_state.value(), compute_state_metric_value(options.start_state.value())) :
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

    auto search_nodes = SearchNodeVector();

    /* Test whether initial state is goal. */

    if (goal_strategy->test_dynamic_goal(start_state))
    {
        event_handler->on_end_search(state_repository.get_reached_fluent_ground_atoms_bitset().count(),
                                     state_repository.get_reached_derived_ground_atoms_bitset().count(),
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

    auto compatible_greedy_and_preferred_openlist = GreedyQueue();
    auto compatible_greedy_openlist = GreedyQueue();
    auto compatible_exhaustive_and_preferred_openlist = ExhaustiveQueue();
    auto compatible_exhaustive_openlist = ExhaustiveQueue();
    auto preferred_openlist = ExhaustiveQueue();
    auto standard_openlist = ExhaustiveQueue();
    auto openlist = AlternatingOpenList<GreedyQueue, GreedyQueue, ExhaustiveQueue, ExhaustiveQueue, ExhaustiveQueue, ExhaustiveQueue>(
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

    auto& start_search_node = get_or_create_search_node(start_state.get_index(), search_nodes);
    start_search_node.status = (start_h_value == INFINITY_CONTINUOUS_COST) ? SearchNodeStatus::DEAD_END : SearchNodeStatus::OPEN;
    start_search_node.g_value = start_g_value;
    start_search_node.h_value = start_h_value;
    start_search_node.preferred = start_preferred;
    start_search_node.compatible = false;

    /* Test whether start state is deadend. */

    if (start_search_node.status == SearchNodeStatus::DEAD_END)
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

    const auto use_exploration_strategy = std::any_of(options.openlist_weights.begin(), options.openlist_weights.begin() + 4, [](double w) { return w > 0; });
    auto applicable_actions = GroundActionList {};
    standard_openlist.insert(ExhaustiveQueueEntry { start_g_value, start_h_value, start_state.get_packed_state(), step++, start_search_node.status });

    auto stopwatch = StopWatch(options.max_time_in_ms);
    stopwatch.start();

    while (!openlist.empty())
    {
        if (stopwatch.has_finished())
        {
            result.status = SearchStatus::OUT_OF_TIME;
            return result;
        }

        const auto state = state_repository.get_state(*openlist.top());
        openlist.pop();

        auto& search_node = get_or_create_search_node(state.get_index(), search_nodes);

        /* Close state. */

        if (search_node.status == SearchNodeStatus::CLOSED || search_node.status == SearchNodeStatus::DEAD_END)
        {
            continue;
        }

        const auto state_h_value = heuristic->compute_heuristic(state, search_node.status == SearchNodeStatus::GOAL);
        search_node.h_value = state_h_value;
        if (state_h_value == INFINITY_CONTINUOUS_COST)
        {
            search_node.status = SearchNodeStatus::DEAD_END;
            continue;
        }

        if (state_h_value < best_h_value)
        {
            best_h_value = state_h_value;
            event_handler->on_new_best_h_value(best_h_value);
        }

        const auto& preferred_actions = heuristic->get_preferred_actions();
        // Ensure that preferred actions are applicable.
        assert(std::all_of(preferred_actions.data.begin(), preferred_actions.data.end(), [&](auto&& action) { return is_applicable(action, state); }));

        /* Expand the successors of the state. */

        event_handler->on_expand_state(state);

        /* Ensure that the state is closed */

        search_node.status = SearchNodeStatus::CLOSED;

        auto first_compatible = true;

        for (const auto& action : applicable_action_generator.create_applicable_action_generator(state))
        {
            const auto [successor_state, successor_state_metric_value] = state_repository.get_or_create_successor_state(state, action, search_node.g_value);
            auto& successor_search_node = get_or_create_search_node(successor_state.get_index(), search_nodes);
            const auto action_cost = successor_state_metric_value - search_node.g_value;

            if (successor_state_metric_value == UNDEFINED_CONTINUOUS_COST)
            {
                throw std::runtime_error("find_solution(...): evaluating the metric on the successor state yielded NaN.");
            }

            const auto is_preferred = preferred_actions.data.contains(action);
            const auto is_new_successor_state = (successor_search_node.status == SearchNodeStatus::NEW);

            if (is_new_successor_state && search_nodes.size() >= options.max_num_states)
            {
                result.status = SearchStatus::OUT_OF_STATES;
                return result;
            }

            /* Skip previously generated state. */

            if (!is_new_successor_state)
            {
                if (successor_search_node.compatible)
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

            successor_search_node.status = SearchNodeStatus::OPEN;
            successor_search_node.parent_state = state.get_index();
            successor_search_node.g_value = successor_state_metric_value;
            successor_search_node.h_value = state_h_value;
            successor_search_node.preferred = is_preferred;

            /* Early goal test. */

            const auto successor_is_goal_state = goal_strategy->test_dynamic_goal(successor_state);

            if (successor_is_goal_state)
            {
                successor_search_node.status = SearchNodeStatus::GOAL;

                event_handler->on_expand_goal_state(state);

                event_handler->on_end_search(state_repository.get_reached_fluent_ground_atoms_bitset().count(),
                                             state_repository.get_reached_derived_ground_atoms_bitset().count(),
                                             state_repository.get_state_count(),
                                             search_nodes.size(),
                                             ground_action_repository.size(),
                                             ground_axiom_repository.size());
                applicable_action_generator.on_end_search();
                state_repository.get_axiom_evaluator()->on_end_search();

                result.plan = extract_total_ordered_plan(start_state, start_g_value, successor_search_node, successor_state.get_index(), search_nodes, context);
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
                successor_search_node.compatible = is_compatible;
            }

            if (options.openlist_weights[0] > 0 && is_compatible && is_preferred && first_compatible)
            {
                first_compatible = false;
                compatible_greedy_and_preferred_openlist.insert(GreedyQueueEntry { successor_state.get_packed_state(), step++, successor_search_node.status });
            }
            else if (options.openlist_weights[1] > 0 && is_compatible && first_compatible)
            {
                first_compatible = false;
                compatible_greedy_openlist.insert(GreedyQueueEntry { successor_state.get_packed_state(), step++, successor_search_node.status });
            }
            else if (options.openlist_weights[2] > 0 && is_compatible && is_preferred)
            {
                compatible_exhaustive_and_preferred_openlist.insert(ExhaustiveQueueEntry { successor_state_metric_value,
                                                                                           state_h_value,
                                                                                           successor_state.get_packed_state(),
                                                                                           step++,
                                                                                           successor_search_node.status });
            }
            else if (options.openlist_weights[3] > 0 && is_compatible)
            {
                compatible_exhaustive_openlist.insert(ExhaustiveQueueEntry { successor_state_metric_value,
                                                                             state_h_value,
                                                                             successor_state.get_packed_state(),
                                                                             step++,
                                                                             successor_search_node.status });
            }
            else if (options.openlist_weights[4] > 0 && is_preferred)
            {
                preferred_openlist.insert(ExhaustiveQueueEntry { successor_state_metric_value,
                                                                 state_h_value,
                                                                 successor_state.get_packed_state(),
                                                                 step++,
                                                                 successor_search_node.status });
            }
            else
            {
                standard_openlist.insert(ExhaustiveQueueEntry { successor_state_metric_value,
                                                                state_h_value,
                                                                successor_state.get_packed_state(),
                                                                step++,
                                                                successor_search_node.status });
            }
        }
    }

    event_handler->on_end_search(state_repository.get_reached_fluent_ground_atoms_bitset().count(),
                                 state_repository.get_reached_derived_ground_atoms_bitset().count(),
                                 state_repository.get_state_count(),
                                 search_nodes.size(),
                                 ground_action_repository.size(),
                                 ground_axiom_repository.size());
    event_handler->on_exhausted();

    result.status = SearchStatus::EXHAUSTED;
    return result;
}
}
