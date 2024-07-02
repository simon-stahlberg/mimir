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

Transition::Transition(StateIndex forward_successor, StateIndex backward_successor, GroundAction creating_action) :
    m_forward_successor(forward_successor),
    m_backward_successor(backward_successor),
    m_creating_action(creating_action)
{
}

bool Transition::operator==(const Transition& other) const
{
    if (this != &other)
    {
        return (m_forward_successor == other.m_forward_successor) && (m_backward_successor == other.m_backward_successor)
               && (m_creating_action == other.m_creating_action);
    }
    return true;
}

size_t Transition::hash() const { return loki::hash_combine(m_forward_successor, m_backward_successor, m_creating_action.hash()); }

GroundAction Transition::get_creating_action() const { return m_creating_action; }

StateIndex Transition::get_forward_successor() const { return m_forward_successor; }

StateIndex Transition::get_backward_successor() const { return m_backward_successor; }

/**
 * Distances
 */

std::vector<double> compute_shortest_goal_distances_from_states(size_t num_total_states,
                                                                const StateIndexSet& goal_states,
                                                                const TransitionList& transitions,
                                                                const std::vector<TransitionIndexList>& backward_transition_adjacency_lists,
                                                                bool use_unit_cost_one)
{
    auto distances = std::vector<double>(num_total_states, std::numeric_limits<double>::max());
    auto closed = std::vector<bool>(num_total_states, false);
    auto priority_queue = PriorityQueue<int>();
    for (const auto& state : goal_states)
    {
        distances.at(state) = 0.;
        priority_queue.insert(0., state);
    }

    while (!priority_queue.empty())
    {
        const auto state_index = priority_queue.top();
        priority_queue.pop();
        const auto cost = distances.at(state_index);

        if (closed.at(state_index))
        {
            continue;
        }
        closed.at(state_index) = true;

        for (const auto& transition_index : backward_transition_adjacency_lists.at(state_index))
        {
            const auto& transition = transitions.at(transition_index);
            const auto successor_state = transition.get_backward_successor();

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
