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

Transition::Transition(StateIndex successor_state, GroundAction creating_action) : m_successor_state(successor_state), m_creating_action(creating_action) {}

bool Transition::operator==(const Transition& other) const
{
    if (this != &other)
    {
        return (m_successor_state == other.m_successor_state) && (m_creating_action == other.m_creating_action);
    }
    return true;
}

size_t Transition::hash() const { return loki::hash_combine(m_successor_state, m_creating_action.hash()); }

StateIndex Transition::get_successor_state() const { return m_successor_state; }

GroundAction Transition::get_creating_action() const { return m_creating_action; }

/**
 * Distance computations
 */

std::vector<double> compute_shortest_distances_from_states(const size_t num_total_states,
                                                           const StateIndexList& states,
                                                           const std::vector<TransitionList>& transitions,
                                                           bool use_unit_cost_one)
{
    auto distances = std::vector<double>(num_total_states, std::numeric_limits<double>::max());
    auto closed = std::vector<bool>(num_total_states, false);
    auto priority_queue = PriorityQueue<int>();
    for (const auto& state : states)
    {
        distances.at(state) = 0.;
        priority_queue.insert(0., state);
    }

    while (!priority_queue.empty())
    {
        const auto state = priority_queue.top();
        priority_queue.pop();
        const auto cost = distances.at(state);

        if (closed.at(state))
        {
            continue;
        }
        closed.at(state) = true;

        for (const auto& transition : transitions.at(state))
        {
            const auto successor_state = transition.get_successor_state();

            auto succ_cost = distances.at(successor_state);
            auto new_succ_cost = cost + ((use_unit_cost_one) ? 1. : transition.get_creating_action().get_cost());

            if (new_succ_cost < succ_cost)
            {
                distances.at(successor_state) = new_succ_cost;
                // decrease priority
                priority_queue.insert(new_succ_cost, successor_state);
            }
        }
    }

    return distances;
}
}
