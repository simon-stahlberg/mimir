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

struct BrFSSearchNodeTag
{
};

using BrFSSearchNodeImpl = SearchNodeImpl<DiscreteCost>;
using BrFSSearchNode = BrFSSearchNodeImpl*;
using ConstBrFSSearchNode = const BrFSSearchNodeImpl*;

static void set_g_value(BrFSSearchNode node, DiscreteCost g_value) { node->get_property<0>() = g_value; }

static DiscreteCost get_g_value(ConstBrFSSearchNode node) { return node->get_property<0>(); }

static BrFSSearchNode
get_or_create_search_node(size_t state_index, const BrFSSearchNodeImpl& default_node, mimir::buffering::Vector<BrFSSearchNodeImpl>& search_nodes)
{
    while (state_index >= search_nodes.size())
    {
        search_nodes.push_back(default_node);
    }
    return search_nodes[state_index];
}

/**
 * BrFS
 */

SearchResult find_solution(const SearchContext& context,
                           State start_state_,
                           EventHandler event_handler_,
                           GoalStrategy goal_strategy_,
                           PruningStrategy pruning_strategy_,
                           bool exhaustive)
{
    const auto& problem = *context->get_problem();
    auto& applicable_action_generator = *context->get_applicable_action_generator();
    auto& state_repository = *context->get_state_repository();

    const auto [start_state, start_g_value] =
        (start_state_) ? std::make_pair(start_state_, compute_state_metric_value(start_state_, problem)) : state_repository.get_or_create_initial_state();
    const auto event_handler = (event_handler_) ? event_handler_ : DefaultEventHandlerImpl::create(context->get_problem());
    const auto goal_strategy = (goal_strategy_) ? goal_strategy_ : ProblemGoalStrategyImpl::create(context->get_problem());
    const auto pruning_strategy = (pruning_strategy_) ? pruning_strategy_ : DuplicatePruningStrategyImpl::create();

    auto result = SearchResult();
    auto default_search_node = BrFSSearchNodeImpl { SearchNodeStatus::NEW, std::numeric_limits<Index>::max(), DiscreteCost(0) };
    auto search_nodes = SearchNodeImplVector<DiscreteCost>();
    auto queue = std::deque<State>();

    event_handler->on_start_search(start_state);

    auto start_search_node = get_or_create_search_node(start_state->get_index(), default_search_node, search_nodes);
    start_search_node->get_status() = SearchNodeStatus::OPEN;
    set_g_value(start_search_node, 0);

    if (!goal_strategy->test_static_goal())
    {
        event_handler->on_unsolvable();

        result.status = SearchStatus::UNSOLVABLE;
        return result;
    }

    auto applicable_actions = GroundActionList {};

    if (pruning_strategy->test_prune_initial_state(start_state))
    {
        result.status = SearchStatus::FAILED;
        return result;
    }

    queue.emplace_back(start_state);

    auto g_value = DiscreteCost(0);

    event_handler->on_finish_g_layer();

    while (!queue.empty())
    {
        const auto state = queue.front();
        queue.pop_front();

        // We need this before goal test for correct statistics reporting.
        auto search_node = get_or_create_search_node(state->get_index(), default_search_node, search_nodes);

        if (get_g_value(search_node) > g_value)
        {
            applicable_action_generator.on_finish_search_layer();
            state_repository.get_axiom_evaluator()->on_finish_search_layer();
            event_handler->on_finish_g_layer();
            g_value = get_g_value(search_node);
        }

        if (goal_strategy->test_dynamic_goal(state))
        {
            event_handler->on_expand_goal_state(state);

            if (!exhaustive)
            {
                event_handler->on_end_search(state_repository.get_reached_fluent_ground_atoms_bitset().count(),
                                             state_repository.get_reached_derived_ground_atoms_bitset().count(),
                                             state_repository.get_estimated_memory_usage_in_bytes_for_unextended_state_portion(),
                                             state_repository.get_estimated_memory_usage_in_bytes_for_extended_state_portion(),
                                             search_nodes.get_estimated_memory_usage_in_bytes(),
                                             problem.get_estimated_memory_usage_in_bytes_for_actions(),
                                             problem.get_estimated_memory_usage_in_bytes_for_axioms(),
                                             state_repository.get_state_count(),
                                             search_nodes.size(),
                                             problem.get_num_ground_actions(),
                                             problem.get_num_ground_axioms());
                if (!event_handler->is_quiet())
                {
                    applicable_action_generator.on_end_search();
                    state_repository.get_axiom_evaluator()->on_end_search();
                }

                auto plan_actions = GroundActionList {};
                auto state_trajectory = IndexList {};
                extract_state_trajectory(search_nodes, search_node, state->get_index(), state_trajectory);
                extract_ground_action_sequence(start_state, 0, state_trajectory, applicable_action_generator, state_repository, plan_actions);
                result.goal_state = state;
                result.plan = Plan(std::move(plan_actions), get_g_value(search_node));
                result.status = SearchStatus::SOLVED;

                event_handler->on_solved(result.plan.value());

                return result;
            }
        }

        event_handler->on_expand_state(state);

        for (const auto& action : applicable_action_generator.create_applicable_action_generator(state))
        {
            /* Open state. */
            const auto [successor_state, successor_state_metric_value] =
                state_repository.get_or_create_successor_state(state, action, get_g_value(search_node));
            auto successor_search_node = get_or_create_search_node(successor_state->get_index(), default_search_node, search_nodes);
            auto action_cost = successor_state_metric_value - get_g_value(search_node);

            event_handler->on_generate_state(state, action, action_cost, successor_state);
            if (pruning_strategy->test_prune_successor_state(state, successor_state, (successor_search_node->get_status() == SearchNodeStatus::NEW)))
            {
                event_handler->on_generate_state_not_in_search_tree(state, action, action_cost, successor_state);
                continue;
            }
            event_handler->on_generate_state_in_search_tree(state, action, action_cost, successor_state);

            successor_search_node->get_status() = SearchNodeStatus::OPEN;
            successor_search_node->get_parent_state() = state->get_index();
            set_g_value(successor_search_node, get_g_value(search_node) + 1);

            queue.emplace_back(successor_state);
        }

        /* Close state. */
        search_node->get_status() = SearchNodeStatus::CLOSED;
    }

    event_handler->on_end_search(state_repository.get_reached_fluent_ground_atoms_bitset().count(),
                                 state_repository.get_reached_derived_ground_atoms_bitset().count(),
                                 state_repository.get_estimated_memory_usage_in_bytes_for_unextended_state_portion(),
                                 state_repository.get_estimated_memory_usage_in_bytes_for_extended_state_portion(),
                                 search_nodes.get_estimated_memory_usage_in_bytes(),
                                 problem.get_estimated_memory_usage_in_bytes_for_actions(),
                                 problem.get_estimated_memory_usage_in_bytes_for_axioms(),
                                 state_repository.get_state_count(),
                                 search_nodes.size(),
                                 problem.get_num_ground_actions(),
                                 problem.get_num_ground_axioms());
    event_handler->on_exhausted();

    result.status = SearchStatus::EXHAUSTED;
    return result;
}
}
