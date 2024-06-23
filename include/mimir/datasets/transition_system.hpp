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
#include "mimir/search/openlists.hpp"
#include "mimir/search/state.hpp"

#include <concepts>

namespace mimir
{

/// @brief Transition encapsulates data of a transition in a transition system.
class Transition
{
private:
    StateIndex m_successor_state;
    GroundAction m_creating_action;

public:
    Transition(StateIndex successor_state, GroundAction creating_action);

    [[nodiscard]] bool operator==(const Transition& other) const;
    [[nodiscard]] size_t hash() const;

    StateIndex get_successor_state() const;
    GroundAction get_creating_action() const;
};

using TransitionList = std::vector<Transition>;

template<typename T>
concept IsTransitionSystem = requires(T a, StateIndex state_index) {
    // States
    {
        a.get_initial_state()
    } -> std::convertible_to<StateIndex>;
    {
        a.get_goal_states()
    } -> std::convertible_to<const StateIndexSet&>;
    {
        a.get_deadend_states()
    } -> std::convertible_to<const StateIndexSet&>;
    {
        a.get_num_states()
    } -> std::convertible_to<size_t>;
    {
        a.get_num_goal_states()
    } -> std::convertible_to<size_t>;
    {
        a.get_num_deadend_states()
    } -> std::convertible_to<size_t>;
    {
        a.is_goal_state(state_index)
    } -> std::convertible_to<bool>;
    {
        a.is_deadend_state(state_index)
    } -> std::convertible_to<bool>;
    {
        a.is_alive_state(state_index)
    } -> std::convertible_to<bool>;

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
    } -> std::convertible_to<const std::vector<double>&>;
};

/**
 * Declarations
 */

/// @brief Compute shortest distances from the given states using Dijkstra.
extern std::vector<double> compute_shortest_distances_from_states(size_t num_total_states,
                                                                  const StateIndexList& states,
                                                                  const std::vector<TransitionList>& transitions,
                                                                  bool use_unit_cost_one = true);

/// @brief Compute shortest distances from the given states using Dijkstra.
template<IsTransitionSystem TransitionSystem>
std::vector<double>
compute_shortest_distances_from_states(const TransitionSystem& ts, const StateIndexList& states, bool use_unit_cost_one = true, bool forward = true);

/// @brief Compute shortest distances from the given states using Floyd-Warshall.
template<IsTransitionSystem TransitionSystem>
std::vector<std::vector<double>> compute_pairwise_shortest_state_distances(const TransitionSystem& ts, bool use_unit_cost_one = true, bool forward = true);

/**
 * Implementations
 */

template<IsTransitionSystem TransitionSystem>
std::vector<double> compute_shortest_distances_from_states(const TransitionSystem& ts, const StateIndexList& states, bool use_unit_cost_one, bool forward)
{
    // Fetch data
    const auto num_states = ts.get_num_states();
    const auto& transitions = (forward) ? ts.get_forward_transitions() : ts.get_backward_transitions();

    return compute_shortest_distances_from_states(num_states, states, transitions, use_unit_cost_one);
}

template<IsTransitionSystem TransitionSystem>
std::vector<std::vector<double>> compute_pairwise_shortest_state_distances(const TransitionSystem& ts, bool use_unit_cost_one, bool forward)
{
    // Fetch data
    const auto num_states = ts.get_num_states();
    const auto& transitions = (forward) ? ts.get_forward_transitions() : ts.get_backward_transitions();

    auto distances = std::vector<std::vector<double>> { num_states, std::vector<double>(num_states, std::numeric_limits<double>::max()) };

    // Initialize distance adjacency matrix
    for (StateIndex state = 0; state < num_states; ++state)
    {
        distances.at(state).at(state) = 0.;
        for (const auto& transition : transitions.at(state))
        {
            distances.at(state).at(transition.get_successor_state()) = (use_unit_cost_one) ? 1. : transition.get_creating_action().get_cost();
        }
    }

    // Compute transitive closure
    for (StateIndex state_k = 0; state_k < num_states; ++state_k)
    {
        for (StateIndex state_i = 0; state_i < num_states; ++state_i)
        {
            for (StateIndex state_j = 0; state_j < num_states; ++state_j)
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
