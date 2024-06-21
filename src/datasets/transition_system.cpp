/*
 * Copyright (C) 2023 Simon Stahlberg
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

#include "mimir/datasets/transition_system.hpp"

namespace mimir
{

/**
 * Transition
 */

Transition::Transition(StateId successor_state, GroundAction creating_action) : m_successor_state(successor_state), m_creating_action(creating_action) {}

bool Transition::operator==(const Transition& other) const
{
    if (this != &other)
    {
        return (m_successor_state == other.m_successor_state) && (m_creating_action == other.m_creating_action);
    }
    return true;
}

size_t Transition::hash() const { return loki::hash_combine(m_successor_state, m_creating_action.hash()); }

StateId Transition::get_successor_state() const { return m_successor_state; }

GroundAction Transition::get_creating_action() const { return m_creating_action; }

/**
 * Distance computations
 */

std::vector<int>
compute_shortest_distances_from_states(const size_t num_total_states, const StateIdList& states, const std::vector<TransitionList>& transitions)
{
    auto distances = std::vector<int>(num_total_states, -1);
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
}
