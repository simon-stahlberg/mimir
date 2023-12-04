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

#include "../../include/mimir/generators/complete_state_space.hpp"
#include "../../include/mimir/generators/successor_generator_factory.hpp"
#include "../formalism/help_functions.hpp"

#include <algorithm>
#include <chrono>
#include <deque>
#include <limits>

namespace std
{
    template<>
    struct hash<pair<uint64_t, uint64_t>>
    {
        size_t operator()(const pair<uint64_t, uint64_t>& x) const { return (size_t) x.first; }
    };
}  // namespace std

namespace mimir::planners
{
    struct StateInfo
    {
        int32_t distance_from_initial_state;
        int32_t distance_to_goal_state;

        StateInfo(int32_t distance_from_initial_state, int32_t distance_to_goal_state) :
            distance_from_initial_state(distance_from_initial_state),
            distance_to_goal_state(distance_to_goal_state)
        {
        }
    };

    CompleteStateSpaceImpl::CompleteStateSpaceImpl(const mimir::formalism::ProblemDescription& problem) :
        StateSpaceImpl(problem),
        states_(),
        goal_states_(),
        state_infos_(),
        dead_end_states_(),
        states_by_distance_(),
        forward_transitions_(),
        backward_transitions_(),
        state_indices_(),
        state_distances_()
    {
    }

    CompleteStateSpaceImpl::~CompleteStateSpaceImpl()
    {
        states_.clear();
        goal_states_.clear();
        state_infos_.clear();
        forward_transitions_.clear();
        backward_transitions_.clear();
        state_indices_.clear();
        state_distances_.clear();
    }

    bool CompleteStateSpaceImpl::add_or_get_state(const mimir::formalism::State& state, uint64_t& out_index)
    {
        const auto index_handler = state_indices_.find(state);

        if (index_handler != state_indices_.end())
        {
            out_index = index_handler->second;
            return false;
        }
        else
        {
            out_index = states_.size();
            states_.push_back(state);
            state_infos_.push_back(StateInfo(-1, -1));
            forward_transitions_.push_back(std::vector<mimir::formalism::Transition>());
            backward_transitions_.push_back(std::vector<mimir::formalism::Transition>());
            state_indices_.insert(std::make_pair(state, out_index));

            if (literals_hold(problem->goal, state))
            {
                goal_states_.push_back(state);
            }

            return true;
        }
    }

    void CompleteStateSpaceImpl::add_goal_state(const mimir::formalism::State& state)
    {
        if (std::find(goal_states_.begin(), goal_states_.end(), state) == goal_states_.end())
        {
            goal_states_.push_back(state);
        }
    }

    void CompleteStateSpaceImpl::add_transition(uint64_t from_state_index,
                                                uint64_t to_state_index,
                                                const mimir::formalism::Action& action,
                                                uint64_t& out_from_forward_index,
                                                uint64_t& out_to_backward_index)
    {
        out_from_forward_index = forward_transitions_[from_state_index].size();
        out_to_backward_index = backward_transitions_[to_state_index].size();

        const auto transition = create_transition(get_state(from_state_index), action, get_state(to_state_index));
        forward_transitions_[from_state_index].push_back(transition);
        backward_transitions_[to_state_index].push_back(transition);
    }

    const mimir::formalism::Transition& CompleteStateSpaceImpl::get_forward_transition(uint64_t state_index, uint64_t transition_index) const
    {
        return forward_transitions_[state_index][transition_index];
    }

    const mimir::formalism::Transition& CompleteStateSpaceImpl::get_backward_transition(uint64_t state_index, uint64_t transition_index) const
    {
        return backward_transitions_[state_index][transition_index];
    }

    const mimir::formalism::TransitionList& CompleteStateSpaceImpl::get_forward(uint64_t state_index) const { return forward_transitions_[state_index]; }

    const mimir::formalism::TransitionList& CompleteStateSpaceImpl::get_backward(uint64_t state_index) const { return backward_transitions_[state_index]; }

    mimir::formalism::State CompleteStateSpaceImpl::get_state(uint64_t state_index) const { return states_[state_index]; }

    int32_t CompleteStateSpaceImpl::get_distance_to_goal(uint64_t state_index) const { return state_infos_[state_index].distance_to_goal_state; }

    int32_t CompleteStateSpaceImpl::get_longest_distance_to_goal_state() const
    {
        int32_t longest_distance_to_goal_state = 0;

        for (std::size_t index = 0; index < state_infos_.size(); ++index)
        {
            longest_distance_to_goal_state = std::max(longest_distance_to_goal_state, state_infos_[index].distance_to_goal_state);
        }

        return longest_distance_to_goal_state;
    }

    int32_t CompleteStateSpaceImpl::get_distance_from_initial(uint64_t state_index) const { return state_infos_[state_index].distance_from_initial_state; }

    const std::vector<mimir::formalism::Transition>& CompleteStateSpaceImpl::get_forward_transitions(const mimir::formalism::State& state) const
    {
        const auto index = get_state_index(state);
        return forward_transitions_[index];
    }

    const std::vector<mimir::formalism::Transition>& CompleteStateSpaceImpl::get_backward_transitions(const mimir::formalism::State& state) const
    {
        const auto index = get_state_index(state);
        return backward_transitions_[index];
    }

    const std::vector<mimir::formalism::State>& CompleteStateSpaceImpl::get_states() const { return states_; }

    mimir::formalism::State CompleteStateSpaceImpl::get_initial_state() const { return mimir::formalism::create_state(problem->initial, problem); }

    uint64_t CompleteStateSpaceImpl::get_unique_index_of_state(const mimir::formalism::State& state) const { return get_state_index(state); }

    uint64_t CompleteStateSpaceImpl::get_state_index(const mimir::formalism::State& state) const
    {
        const auto index_handler = state_indices_.find(state);

        if (index_handler == state_indices_.end())
        {
            throw std::invalid_argument("state");
        }
        else
        {
            return index_handler->second;
        }
    }

    bool CompleteStateSpaceImpl::is_dead_end_state(const mimir::formalism::State& state) const
    {
        const auto index = get_state_index(state);
        const auto& info = state_infos_[index];
        return info.distance_to_goal_state < 0;
    }

    bool CompleteStateSpaceImpl::is_goal_state(const mimir::formalism::State& state) const
    {
        const auto index = get_state_index(state);
        const auto& info = state_infos_[index];
        return info.distance_to_goal_state == 0;
    }

    int32_t CompleteStateSpaceImpl::get_distance_to_goal_state(const mimir::formalism::State& state) const
    {
        const auto index = get_state_index(state);
        return get_distance_to_goal(index);
    }

    int32_t CompleteStateSpaceImpl::get_distance_between_states(const mimir::formalism::State& from_state, const mimir::formalism::State& to_state) const
    {
        // Check if the Floyd-Warshall distance matrix has been cached, if not, compute it.

        if (state_distances_.size() == 0)
        {
            const auto size = num_states();
            const auto inf = std::numeric_limits<int32_t>::max();

            state_distances_ = std::vector(size, std::vector<int32_t>(size, inf));

            for (const auto& transitions : forward_transitions_)
            {
                for (const auto& transition : transitions)
                {
                    const auto source_index = get_state_index(transition->source_state);
                    const auto target_index = get_state_index(transition->target_state);
                    state_distances_[source_index][target_index] = 1;
                }
            }

            for (const auto& state : states_)
            {
                const auto state_index = get_state_index(state);
                state_distances_[state_index][state_index] = 0;
            }

            for (uint64_t k = 0; k < size; ++k)
            {
                for (uint64_t i = 0; i < size; ++i)
                {
                    for (uint64_t j = 0; j < size; ++j)
                    {
                        const auto ik = state_distances_[i][k];
                        const auto kj = state_distances_[k][j];

                        if ((ik < inf) && (kj < inf))
                        {
                            auto& ij = state_distances_[i][j];
                            auto ikj = ik + kj;

                            if (ikj < ij)
                            {
                                ij = ikj;
                            }
                        }
                    }
                }
            }
        }

        // Return the distance between the given states.

        const auto from_index = get_state_index(from_state);
        const auto to_index = get_state_index(to_state);
        return state_distances_[from_index][to_index];
    }

    int32_t CompleteStateSpaceImpl::get_distance_from_initial_state(const mimir::formalism::State& state) const
    {
        const auto index = get_state_index(state);
        return get_distance_from_initial(index);
    }

    mimir::formalism::State CompleteStateSpaceImpl::sample_state() const
    {
        const auto index = std::rand() % static_cast<int>(states_.size());
        return states_[index];
    }

    mimir::formalism::State CompleteStateSpaceImpl::sample_state_with_distance_to_goal(int32_t distance) const
    {
        if (static_cast<std::size_t>(distance) < states_by_distance_.size())
        {
            const auto index = std::rand() % static_cast<int>(states_by_distance_[distance].size());
            return states_by_distance_[distance][index];
        }
        else
        {
            throw std::invalid_argument("distance is out of range");
        }
    }

    mimir::formalism::State CompleteStateSpaceImpl::sample_dead_end_state() const
    {
        if (dead_end_states_.size() == 0)
        {
            throw std::invalid_argument("no dead end states to sample");
        }

        const auto index = std::rand() % static_cast<int>(dead_end_states_.size());
        return dead_end_states_[index];
    }

    void CompleteStateSpaceImpl::set_distance_from_initial_state(uint64_t state_index, int32_t value)
    {
        state_infos_[state_index].distance_from_initial_state = value;
    }

    void CompleteStateSpaceImpl::set_distance_to_goal_state(uint64_t state_index, int32_t value) { state_infos_[state_index].distance_to_goal_state = value; }

    std::vector<mimir::formalism::State> CompleteStateSpaceImpl::get_goal_states() const { return goal_states_; }

    uint64_t CompleteStateSpaceImpl::num_states() const { return (uint64_t) states_.size(); }

    uint64_t CompleteStateSpaceImpl::num_transitions() const
    {
        uint64_t num_transitions = 0;

        for (std::size_t i = 0; i < forward_transitions_.size(); ++i)
        {
            num_transitions += forward_transitions_[i].size();
        }

        return num_transitions;
    }

    uint64_t CompleteStateSpaceImpl::num_goal_states() const { return (uint64_t) goal_states_.size(); }

    uint64_t CompleteStateSpaceImpl::num_dead_end_states() const
    {
        uint64_t num_dead_ends = 0;

        for (const auto& info : state_infos_)
        {
            if (info.distance_to_goal_state < 0)
            {
                ++num_dead_ends;
            }
        }

        return num_dead_ends;
    }

    CompleteStateSpace create_complete_state_space(const mimir::formalism::ProblemDescription& problem,
                                                   const mimir::planners::SuccessorGenerator& successor_generator,
                                                   uint32_t max_states)
    {
        if (problem != successor_generator->get_problem())
        {
            throw std::invalid_argument("the successor generator is not for the given problem");
        }

        auto state_space = new CompleteStateSpaceImpl(problem);

        std::vector<uint64_t> goal_indices;
        std::vector<bool> is_expanded;
        std::deque<uint64_t> queue;

        {
            uint64_t initial_index;

            if (state_space->add_or_get_state(mimir::formalism::create_state(problem->initial, problem), initial_index))
            {
                state_space->set_distance_from_initial_state(initial_index, 0);
                is_expanded.push_back(false);
            }

            queue.push_back(initial_index);
        }

        const auto goal = problem->goal;

        while ((queue.size() > 0) && (state_space->num_states() < max_states))
        {
            const auto state_index = queue.front();
            queue.pop_front();

            if (is_expanded[state_index])
            {
                continue;
            }

            const auto state = state_space->get_state(state_index);
            const auto distance_from_initial_state = state_space->get_distance_from_initial(state_index);
            const auto is_goal_state = mimir::formalism::literals_hold(goal, state);

            if (is_goal_state)
            {
                goal_indices.push_back(state_index);
                state_space->add_goal_state(state);
            }

            is_expanded[state_index] = true;
            const auto ground_actions = successor_generator->get_applicable_actions(state);
            std::vector<uint64_t> successor_indices;

            for (const auto& ground_action : ground_actions)
            {
                const auto successor_state = mimir::formalism::apply(ground_action, state);
                uint64_t successor_state_index;
                bool successor_is_new_state;

                successor_is_new_state = state_space->add_or_get_state(successor_state, successor_state_index);

                uint64_t from_transition_index, to_transition_index;
                state_space->add_transition(state_index, successor_state_index, ground_action, from_transition_index, to_transition_index);

                if (successor_is_new_state)
                {
                    state_space->set_distance_from_initial_state(successor_state_index, distance_from_initial_state + 1);
                    is_expanded.push_back(false);
                    queue.push_back(successor_state_index);
                }
            }
        }

        // Check if every state was expanded (i.e., if max_states stopped the search).

        if (queue.size() > 0)
        {
            delete state_space;
            return nullptr;
        }

        queue.insert(queue.end(), goal_indices.begin(), goal_indices.end());

        for (const auto& goal_state_index : goal_indices)
        {
            state_space->set_distance_to_goal_state(goal_state_index, 0);
        }

        std::fill(is_expanded.begin(), is_expanded.end(), false);

        while (queue.size() > 0)
        {
            const auto state_index = queue.front();
            queue.pop_front();

            if (is_expanded[state_index])
            {
                continue;
            }

            is_expanded[state_index] = true;

            const auto distance_to_goal_state = state_space->get_distance_to_goal(state_index);
            const auto& backward_transitions = state_space->get_backward(state_index);

            for (const auto& backward_transition : backward_transitions)
            {
                const auto predecessor_state_index = state_space->get_state_index(backward_transition->source_state);
                const auto predecessor_is_new_state = state_space->get_distance_to_goal(predecessor_state_index) < 0;

                if (predecessor_is_new_state)
                {
                    state_space->set_distance_to_goal_state(predecessor_state_index, distance_to_goal_state + 1);
                    queue.push_back(predecessor_state_index);
                }
            }
        }

        const auto max_distance = state_space->get_longest_distance_to_goal_state();
        state_space->states_by_distance_.resize(max_distance + 1);

        for (const auto& state : state_space->get_states())
        {
            const auto distance = state_space->get_distance_to_goal_state(state);

            if (distance >= 0)
            {
                state_space->states_by_distance_[distance].push_back(state);
            }
            else
            {
                state_space->dead_end_states_.push_back(state);
            }
        }

        return CompleteStateSpace(state_space);
    }
}  // namespace mimir::planners
