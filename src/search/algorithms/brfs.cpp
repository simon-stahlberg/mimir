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

#include "mimir/search/algorithms/brfs.hpp"

#include "mimir/common/segmented_vector.hpp"
#include "mimir/common/timers.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/algorithms/brfs/event_handlers.hpp"
#include "mimir/search/algorithms/brfs/event_handlers/interface.hpp"
#include "mimir/search/algorithms/strategies/goal_strategy.hpp"
#include "mimir/search/algorithms/strategies/pruning_strategy.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/axiom_evaluators/interface.hpp"
#include "mimir/search/plan.hpp"
#include "mimir/search/search_context.hpp"
#include "mimir/search/search_node.hpp"
#include "mimir/search/search_space.hpp"
#include "mimir/search/state_repository.hpp"

#include <deque>

using namespace mimir::formalism;

namespace mimir::search::brfs
{

/**
 * BrFS search node
 */

struct SearchNode
{
    DiscreteCost g_value;
    Index parent_state;
    SearchNodeStatus status;
};

using SearchNodeVector = SegmentedVector<SearchNode>;

static SearchNode& get_or_create_search_node(size_t state_index, SearchNodeVector& search_nodes)
{
    static constexpr auto default_node = SearchNode { DiscreteCost(0), std::numeric_limits<Index>::max(), SearchNodeStatus::NEW };

    while (state_index >= search_nodes.size())
    {
        search_nodes.push_back(default_node);
    }
    return search_nodes[state_index];
}

/**
 * BrFS
 */

SearchResult find_solution(const SearchContext& context, const Options& options)
{
    const auto& problem = *context->get_problem();
    auto& applicable_action_generator = *context->get_applicable_action_generator();
    auto& state_repository = *context->get_state_repository();

    const auto [start_state, start_g_value] = (options.start_state) ?
                                                  std::make_pair(options.start_state.value(), compute_state_metric_value(options.start_state.value())) :
                                                  state_repository.get_or_create_initial_state();
    const auto event_handler = (options.event_handler) ? options.event_handler : DefaultEventHandlerImpl::create(context->get_problem());
    const auto goal_strategy = (options.goal_strategy) ? options.goal_strategy : ProblemGoalStrategyImpl::create(context->get_problem());
    const auto pruning_strategy = (options.pruning_strategy) ? options.pruning_strategy : DuplicatePruningStrategyImpl::create();

    auto result = SearchResult();
    auto search_nodes = SearchNodeVector();
    auto queue = std::deque<PackedState>();

    auto& start_search_node = get_or_create_search_node(start_state.get_index(), search_nodes);
    start_search_node.status = SearchNodeStatus::OPEN;
    start_search_node.g_value = 0;

    event_handler->on_start_search(start_state);

    if (!goal_strategy->test_static_goal())
    {
        event_handler->on_unsolvable();

        result.status = SearchStatus::UNSOLVABLE;
        return result;
    }

    const auto& ground_action_repository = boost::hana::at_key(problem.get_repositories().get_hana_repositories(), boost::hana::type<GroundActionImpl> {});
    const auto& ground_axiom_repository = boost::hana::at_key(problem.get_repositories().get_hana_repositories(), boost::hana::type<GroundAxiomImpl> {});

    auto applicable_actions = GroundActionList {};

    if (pruning_strategy->test_prune_initial_state(start_state))
    {
        result.status = SearchStatus::FAILED;
        return result;
    }

    queue.emplace_back(start_state.get_packed_state());

    auto g_value = DiscreteCost(0);

    event_handler->on_finish_g_layer(g_value);

    auto stopwatch = StopWatch(options.max_time_in_ms);
    stopwatch.start();

    while (!queue.empty())
    {
        if (stopwatch.has_finished())
        {
            result.status = SearchStatus::OUT_OF_TIME;
            return result;
        }

        const auto state = state_repository.get_state(*queue.front());
        queue.pop_front();

        auto& search_node = get_or_create_search_node(state.get_index(), search_nodes);

        /* Close state. */

        if (search_node.status == SearchNodeStatus::CLOSED || search_node.status == SearchNodeStatus::DEAD_END)
        {
            continue;
        }

        if (search_node.g_value > g_value)
        {
            applicable_action_generator.on_finish_search_layer();
            state_repository.get_axiom_evaluator()->on_finish_search_layer();
            event_handler->on_finish_g_layer(g_value);
            g_value = search_node.g_value;
        }

        if (goal_strategy->test_dynamic_goal(state))
        {
            event_handler->on_expand_goal_state(state);

            if (options.stop_if_goal)
            {
                event_handler->on_end_search(state_repository.get_reached_fluent_ground_atoms_bitset().count(),
                                             state_repository.get_reached_derived_ground_atoms_bitset().count(),
                                             state_repository.get_state_count(),
                                             search_nodes.size(),
                                             ground_action_repository.size(),
                                             ground_axiom_repository.size());

                applicable_action_generator.on_end_search();
                state_repository.get_axiom_evaluator()->on_end_search();

                result.goal_state = state;
                result.plan = extract_total_ordered_plan(start_state, start_g_value, search_node, state.get_index(), search_nodes, context);
                result.status = SearchStatus::SOLVED;

                event_handler->on_solved(result.plan.value());

                return result;
            }
        }

        /* Expand the successors of the state. */

        event_handler->on_expand_state(state);

        /* Ensure that the state is closed */

        search_node.status = SearchNodeStatus::CLOSED;

        for (const auto& action : applicable_action_generator.create_applicable_action_generator(state))
        {
            /* Open state. */
            const auto [successor_state, successor_state_metric_value] = state_repository.get_or_create_successor_state(state, action, search_node.g_value);
            auto& successor_search_node = get_or_create_search_node(successor_state.get_index(), search_nodes);
            auto action_cost = successor_state_metric_value - search_node.g_value;

            event_handler->on_generate_state(state, action, action_cost, successor_state);
            if (pruning_strategy->test_prune_successor_state(state, successor_state, (successor_search_node.status == SearchNodeStatus::NEW)))
            {
                event_handler->on_generate_state_not_in_search_tree(state, action, action_cost, successor_state);
                continue;
            }
            event_handler->on_generate_state_in_search_tree(state, action, action_cost, successor_state);

            successor_search_node.status = SearchNodeStatus::OPEN;
            successor_search_node.parent_state = state.get_index();
            successor_search_node.g_value = search_node.g_value + 1;

            queue.emplace_back(successor_state.get_packed_state());

            if (search_nodes.size() >= options.max_num_states)
            {
                result.status = SearchStatus::OUT_OF_STATES;
                return result;
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
