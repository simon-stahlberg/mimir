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

/// @brief Compute the sequence of ground actions that generates the state_trajectory starting from the start_state.
/// @param start_state is the start state.
/// @param state_trajectory is the state trajectory given by their indices.
/// @param applicable_action_generator is the applicable action generator.
/// @param state_repository is the state repository.
/// @param out_ground_action_sequence is the resulting state trajectory.
inline ContinuousCost extract_ground_action_sequence(State start_state,
                                                     ContinuousCost start_state_metric_value,
                                                     const IndexList& state_trajectory,
                                                     IApplicableActionGenerator& applicable_action_generator,
                                                     StateRepositoryImpl& state_repository,
                                                     formalism::GroundActionList& out_ground_action_sequence)
{
    if (start_state->get_index() != state_trajectory.front())
    {
        throw std::runtime_error("extract_ground_action_sequence: the given start state must be the first state in the given state trajectory.");
    }

    out_ground_action_sequence.clear();

    auto state = start_state;
    auto state_metric_value = start_state_metric_value;

    for (size_t i = 0; i < state_trajectory.size() - 1; ++i)
    {
        // We have to take the (state,action) pair that yields lowest metric value.
        auto lowest_action = formalism::GroundAction { nullptr };
        auto lowest_state = State {};
        auto lowest_metric_value = std::numeric_limits<ContinuousCost>::infinity();

        for (const auto& action : applicable_action_generator.create_applicable_action_generator(state))
        {
            const auto [successor_state, successor_state_metric_value] = state_repository.get_or_create_successor_state(state, action, state_metric_value);
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
        out_ground_action_sequence.push_back(lowest_action);
        state = lowest_state;
        state_metric_value = lowest_metric_value;
    }

    return state_metric_value;
}

/// @brief Compute the state trajectory that ends in the the `final_state_index` associated with the `final_search_node`.
/// @tparam ...SearchNodeProperties
/// @param search_nodes are all search nodes.
/// @param final_search_node is the final search node.
/// @param final_state_index is the final state index.
/// @param out_trajectory is the resulting state trajectory that ends in the `final_state_index`
template<typename... SearchNodeProperties>
void extract_state_trajectory(const SearchNodeImplVector<SearchNodeProperties...>& search_nodes,  //
                              ConstSearchNode<SearchNodeProperties...> final_search_node,
                              Index final_state_index,
                              IndexList& out_trajectory)
{
    out_trajectory.clear();
    out_trajectory.push_back(final_state_index);

    auto cur_search_node = final_search_node;

    while (cur_search_node->get_parent_state() != std::numeric_limits<Index>::max())
    {
        out_trajectory.push_back(cur_search_node->get_parent_state());

        cur_search_node = search_nodes.at(cur_search_node->get_parent_state());
    }

    std::reverse(out_trajectory.begin(), out_trajectory.end());
}

}

#endif