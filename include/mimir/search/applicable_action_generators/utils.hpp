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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_UTILS_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_UTILS_HPP_

#include "mimir/search/applicable_action_generators/interface.hpp"
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

}

#endif
