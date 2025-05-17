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

#ifndef MIMIR_SEARCH_SEARCH_SPACE_HPP_
#define MIMIR_SEARCH_SEARCH_SPACE_HPP_

#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/search_node.hpp"
#include "mimir/search/state_repository.hpp"

namespace mimir::search
{

/// @brief Compute the state trajectory that ends in the the `final_state_index` associated with the `final_search_node`.
/// @tparam ...SearchNodeProperties
/// @param search_nodes are all search nodes.
/// @param final_search_node is the final search node.
/// @param final_state_index is the final state index.
/// @param out_trajectory is the resulting state trajectory that ends in the `final_state_index`
template<typename... SearchNodeProperties>
IndexList extract_state_trajectory(const SearchNodeImplVector<SearchNodeProperties...>& search_nodes,
                                   ConstSearchNode<SearchNodeProperties...> final_search_node,
                                   Index final_state_index)
{
    auto trajectory = IndexList {};
    trajectory.push_back(final_state_index);

    auto cur_search_node = final_search_node;

    while (cur_search_node->get_parent_state() != std::numeric_limits<Index>::max())
    {
        trajectory.push_back(cur_search_node->get_parent_state());

        cur_search_node = search_nodes.at(cur_search_node->get_parent_state());
    }

    std::reverse(trajectory.begin(), trajectory.end());

    return trajectory;
}

/// @brief Compute the sequence of ground actions that generates the state_trajectory starting from the start_state.
/// @param start_state is the start state.
/// @param start_state_metric_value is the metric value of the start state
/// @param final_search_node is the final search node.
/// @param final_state_index is the final state index.
/// @param search_nodes are all search nodes.
/// @param context is the search context.
template<typename... SearchNodeProperties>
inline Plan extract_total_ordered_plan(State start_state,
                                       ContinuousCost start_state_metric_value,
                                       ConstSearchNode<SearchNodeProperties...> final_search_node,
                                       Index final_state_index,
                                       const SearchNodeImplVector<SearchNodeProperties...>& search_nodes,  //
                                       const SearchContext& context)
{
    const auto state_trajectory = extract_state_trajectory(search_nodes, final_search_node, final_state_index);

    auto actions = formalism::GroundActionList {};
    auto states = StateList { start_state };

    auto state = start_state;
    auto state_metric_value = start_state_metric_value;

    for (size_t i = 0; i < state_trajectory.size() - 1; ++i)
    {
        // We have to take the (state,action) pair that yields lowest metric value.
        auto lowest_action = formalism::GroundAction { nullptr };
        auto lowest_state = State {};
        auto lowest_metric_value = std::numeric_limits<ContinuousCost>::infinity();

        for (const auto& action : context->get_applicable_action_generator()->create_applicable_action_generator(state))
        {
            const auto [successor_state, successor_state_metric_value] =
                context->get_state_repository()->get_or_create_successor_state(state, action, state_metric_value);
            if (successor_state->get_index() == state_trajectory.at(i + 1))
            {
                if (successor_state_metric_value < lowest_metric_value)
                {
                    lowest_action = action;
                    lowest_state = successor_state;
                    lowest_metric_value = successor_state_metric_value;
                }
            }
        }
        // std::cout << std::make_tuple(lowest_action, std::cref(*state_repository.get_problem_context().get_repositories()), FullActionFormatterTag {})
        //           << std::endl;
        // std::cout << std::make_tuple(state_repository.get_problem_context().get_problem(),
        //                              lowest_state,
        //                              std::cref(*state_repository.get_problem_context().get_repositories()))
        //           << std::endl;
        // std::cout << lowest_metric_value << " " << lowest_metric_value - state_metric_value << std::endl << std::endl << std::endl;
        actions.push_back(lowest_action);
        states.push_back(lowest_state);
        state = lowest_state;
        state_metric_value = lowest_metric_value;
    }

    return Plan(context, std::move(states), std::move(actions), state_metric_value);
}

}

#endif