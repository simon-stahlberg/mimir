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

namespace mimir
{

/// @brief Compute the sequence of ground actions that generates the state_trajectory starting from the start_state.
/// @param start_state is the start state.
/// @param state_trajectory is the state trajectory given by their indices.
/// @param applicable_action_generator is the applicable action generator.
/// @param state_repository is the state repository.
/// @param out_ground_action_sequence is the resulting state trajectory.
inline void extract_ground_action_sequence(State start_state,
                                           const IndexList& state_trajectory,
                                           IApplicableActionGenerator& applicable_action_generator,
                                           StateRepository& state_repository,
                                           GroundActionList& out_ground_action_sequence)
{
    if (start_state->get_index() != state_trajectory.front())
    {
        throw std::runtime_error("extract_ground_action_sequence: the given start state must be the first state in the given state trajectory.");
    }

    out_ground_action_sequence.clear();

    auto state = start_state;

    for (size_t i = 0; i < state_trajectory.size() - 1; ++i)
    {
        for (const auto& action : applicable_action_generator.create_applicable_action_generator(state))
        {
            const auto [successor_state, cost] = state_repository.get_or_create_successor_state(state, action);

            if (successor_state->get_index() == state_trajectory.at(i + 1))
            {
                out_ground_action_sequence.push_back(action);
                state = successor_state;
                break;
            }
        }
    }
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