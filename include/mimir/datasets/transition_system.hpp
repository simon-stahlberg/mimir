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

#ifndef MIMIR_DATASETS_TRANSITION_SYSTEM_HPP_
#define MIMIR_DATASETS_TRANSITION_SYSTEM_HPP_

#include "mimir/search/action.hpp"
#include "mimir/search/state.hpp"

#include <concepts>

namespace mimir
{

using StateId = int;
using StateIdList = std::vector<StateId>;
using StateIdSet = std::unordered_set<StateId>;

/// @brief Transition encapsulates data of a transition in a transition system.
class Transition
{
private:
    StateId m_successor_state;
    GroundAction m_creating_action;

public:
    Transition(StateId successor_state, GroundAction creating_action);

    [[nodiscard]] bool operator==(const Transition& other) const;
    [[nodiscard]] size_t hash() const;

    StateId get_successor_state() const;
    GroundAction get_creating_action() const;
};

using TransitionList = std::vector<Transition>;

template<typename T>
concept IsTransitionSystem = requires(T a) {
    // States
    {
        a.get_initial_state()
    } -> std::convertible_to<StateId>;
    {
        a.get_goal_states()
    } -> std::convertible_to<const StateIdSet&>;
    {
        a.get_deadend_states()
    } -> std::convertible_to<const StateIdSet&>;
    {
        a.get_num_states()
    } -> std::convertible_to<size_t>;
    {
        a.get_num_goal_states()
    } -> std::convertible_to<size_t>;
    {
        a.get_num_deadend_states()
    } -> std::convertible_to<size_t>;

    // Transitions
    {
        a.get_num_transitions()
    } -> std::convertible_to<size_t>;
    {
        a.get_forward_transitions()
    } -> std::convertible_to<const std::vector<TransitionList>&>;
    {
        a.get_backward_transitions()
    } -> std::convertible_to<const std::vector<TransitionList>&>;

    // Distances
    {
        a.get_goal_distances()
    } -> std::convertible_to<const std::vector<int>&>;
};

/**
 * Declarations
 */

extern std::vector<int>
compute_shortest_distances_from_states(const size_t num_total_states, const StateIdList& states, const std::vector<TransitionList>& transitions);

template<IsTransitionSystem TransitionSystem>
std::vector<int> compute_shortest_distances_from_states(const TransitionSystem& ts, const StateIdList& states, bool forward = true);

template<IsTransitionSystem TransitionSystem>
std::vector<std::vector<int>> compute_pairwise_shortest_state_distances(const TransitionSystem& ts, bool forward = true);

/**
 * Implementations
 */

template<IsTransitionSystem TransitionSystem>
std::vector<int> compute_shortest_distances_from_states(const TransitionSystem& ts, const StateIdList& states, bool forward)
{
    // Fetch data
    const auto num_states = ts.get_num_states();
    const auto& transitions = (forward) ? ts.get_forward_transitions() : ts.get_backward_transitions();

    auto distances = std::vector<int>(num_states, -1);
    auto fifo_queue = std::deque<int>();
    for (const auto& state : states)
    {
        distances.at(state) = 0;
        fifo_queue.push_back(state);
    }

    while (!fifo_queue.empty())
    {
        const auto& state = fifo_queue.front();
        fifo_queue.pop_front();
        const auto cost = distances.at(state);

        for (const auto& transition : transitions.at(state))
        {
            const auto successor_state = transition.get_successor_state();

            if (distances.at(successor_state) != -1)
            {
                continue;
            }

            distances.at(successor_state) = cost + 1;

            fifo_queue.push_back(successor_state);
        }
    }
    return distances;
}

template<IsTransitionSystem TransitionSystem>
std::vector<std::vector<int>> compute_pairwise_shortest_state_distances(const TransitionSystem& ts, bool forward)
{
    // Fetch data
    const auto num_states = ts.get_num_states();
    const auto& transitions = (forward) ? ts.get_forward_transitions() : ts.get_backward_transitions();

    auto distances = std::vector<std::vector<int>> { num_states, std::vector<int>(num_states, -1) };

    // Initialize distance adjacency matrix
    for (size_t state = 0; state < num_states; ++state)
    {
        distances.at(state).at(state) = 0;
        for (const auto& transition : transitions.at(state))
        {
            distances.at(state).at(transition.get_successor_state()) = 1;
        }
    }

    // Compute transitive closure
    for (size_t state_k = 0; state_k < num_states; ++state_k)
    {
        for (size_t state_i = 0; state_i < num_states; ++state_i)
        {
            for (size_t state_j = 0; state_j < num_states; ++state_j)
            {
                if (distances.at(state_i).at(state_j) > distances.at(state_i).at(state_k) + distances.at(state_k).at(state_j))
                {
                    distances.at(state_i).at(state_j) = distances.at(state_i).at(state_k) + distances.at(state_k).at(state_j);
                }
            }
        }
    }

    return distances;
}

}

#endif