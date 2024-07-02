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

#include "mimir/common/concepts.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/openlists.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/successor_state_generator.hpp"

#include <concepts>

namespace mimir
{

struct Forward
{
};
struct Backward
{
};

template<typename T>
concept IsDirection = std::is_same<T, Forward>::value || std::is_same<T, Backward>::value;

/// @brief Transition encapsulates data of a transition in a transition system.
class Transition
{
private:
    StateIndex m_forward_successor;
    StateIndex m_backward_successor;
    GroundAction m_creating_action;

public:
    Transition(StateIndex forward_successor, StateIndex backward_successor, GroundAction creating_action);

    [[nodiscard]] bool operator==(const Transition& other) const;
    [[nodiscard]] size_t hash() const;

    template<IsDirection Direction>
    StateIndex get_successor() const;
    GroundAction get_creating_action() const;
};

using TransitionList = std::vector<Transition>;
using TransitionIndex = int;

template<IsDirection Direction>
struct TransitionIndexList
{
    std::vector<TransitionIndex> indices;
};

template<typename T>
concept IsTransitionSystem = requires(T a, StateIndex state_index) {
    /* Memory */
    {
        a.get_pddl_parser()
    } -> std::convertible_to<const std::shared_ptr<PDDLParser>&>;
    {
        a.get_aag()
    } -> std::convertible_to<const std::shared_ptr<IAAG>&>;
    {
        a.get_ssg()
    } -> std::convertible_to<const std::shared_ptr<SuccessorStateGenerator>&>;

    /* States */
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

    /* Transitions */
    {
        a.get_num_transitions()
    } -> std::convertible_to<size_t>;
    {
        a.get_transitions()
    } -> std::convertible_to<const TransitionList&>;
    {
        a.get_forward_transition_adjacency_lists()
    } -> std::convertible_to<const std::vector<TransitionIndexList<Forward>>&>;
    {
        a.get_backward_transition_adjacency_lists()
    } -> std::convertible_to<const std::vector<TransitionIndexList<Backward>>&>;

    // Distances
    {
        a.get_goal_distances()
    } -> std::convertible_to<const std::vector<double>&>;
};

/**
 * Declarations
 */

// Requirement of boost::VertexListGraph
template<IsTransitionSystem TransitionSystem>
class StateIndexIterator
{
private:
public:
    class const_iterator
    {
    private:
        void advance();

    public:
        using difference_type = StateIndex;
        using value_type = StateIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        // const_iterator(StateTupleIndexGenerator* data, bool begin);
        [[nodiscard]] value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        [[nodiscard]] bool operator==(const const_iterator& other) const;
        [[nodiscard]] bool operator!=(const const_iterator& other) const;
    };
};

// Requirement of boost::AdjacencyGraph
template<IsTransitionSystem TransitionSystem>
class OutAdjacentStateIndexIterator
{
private:
public:
};

template<IsTransitionSystem TransitionSystem>
class InAdjacentStateIndexIterator
{
private:
public:
};

// Requirement of boost::EdgeListGraph
template<IsTransitionSystem TransitionSystem>
class TransitionIndexIterator
{
private:
public:
};

// Requirement of boost::IncidenceGraph
template<IsTransitionSystem TransitionSystem>
class OutTransitionIndexIterator
{
private:
public:
};

// Requirement of boost::BidirectionalGraph
template<IsTransitionSystem TransitionSystem>
class InTransitionIndexIterator
{
private:
public:
};

/// @brief Compute shortest distances from the given states using Dijkstra.
template<IsDirection Direction>
std::vector<double> compute_shortest_distances_from_states(size_t num_total_states,
                                                           const StateIndexList& states,
                                                           const TransitionList& transitions,
                                                           const std::vector<TransitionIndexList<Direction>>& transition_indices,
                                                           bool use_unit_cost_one = true);

/// @brief Compute shortest distances from the given states using Dijkstra.
template<IsTransitionSystem TransitionSystem>
std::vector<double>
compute_shortest_distances_from_states(const TransitionSystem& ts, const StateIndexList& states, bool use_unit_cost_one = true, bool forward = true);

/**
 * Implementations
 */

template<IsDirection Direction>
StateIndex Transition::get_successor() const
{
    if constexpr (std::is_same_v<Direction, Forward>)
    {
        return m_forward_successor;
    }
    else if constexpr (std::is_same_v<Direction, Backward>)
    {
        return m_backward_successor;
    }
    else
    {
        static_assert(dependent_false<Direction>::value, "Missing implementation for PredicateCategory.");
    }
}

template<IsDirection Direction>
std::vector<double> compute_shortest_distances_from_states(const size_t num_total_states,
                                                           const StateIndexList& states,
                                                           const TransitionList& transitions,
                                                           const std::vector<TransitionIndexList<Direction>>& transition_indices,
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
        const auto state_index = priority_queue.top();
        priority_queue.pop();
        const auto cost = distances.at(state_index);

        if (closed.at(state_index))
        {
            continue;
        }
        closed.at(state_index) = true;

        for (const auto& transition_index : transition_indices.at(state_index).indices)
        {
            const auto& transition = transitions.at(transition_index);
            const auto successor_state = transition.template get_successor<Direction>();

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

template<IsTransitionSystem TransitionSystem>
std::vector<double> compute_shortest_distances_from_states(const TransitionSystem& ts, const StateIndexList& states, bool use_unit_cost_one, bool forward)
{
    // Fetch data
    const auto num_states = ts.get_num_states();
    const auto& transitions = (forward) ? ts.get_forward_transition_adjacency_lists() : ts.get_backward_transition_adjacency_lists();

    return compute_shortest_distances_from_states(num_states, states, transitions, use_unit_cost_one);
}

}

#endif
