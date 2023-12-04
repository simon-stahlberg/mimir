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

#include "../../include/mimir/generators/state_space.hpp"
#include "../formalism/help_functions.hpp"

namespace mimir::planners
{
    StateSpaceImpl::StateSpaceImpl(const mimir::formalism::ProblemDescription& problem) : domain(problem->domain), problem(problem) {}

    StateSpaceImpl::~StateSpaceImpl()
    {
        domain = nullptr;
        problem = nullptr;
    }

    const std::vector<mimir::formalism::Transition>& StateSpaceImpl::get_forward_transitions(const mimir::formalism::State& state) const
    {
        throw std::runtime_error("not implemented");
    }

    const std::vector<mimir::formalism::Transition>& StateSpaceImpl::get_backward_transitions(const mimir::formalism::State& state) const
    {
        throw std::runtime_error("not implemented");
    }

    const std::vector<mimir::formalism::State>& StateSpaceImpl::get_states() const { throw std::runtime_error("not implemented"); }

    mimir::formalism::State StateSpaceImpl::get_initial_state() const { throw std::runtime_error("not implemented"); }

    uint64_t StateSpaceImpl::get_unique_index_of_state(const mimir::formalism::State& state) const { throw std::runtime_error("not implemented"); }

    bool StateSpaceImpl::is_dead_end_state(const mimir::formalism::State& state) const { throw std::runtime_error("not implemented"); }

    bool StateSpaceImpl::is_goal_state(const mimir::formalism::State& state) const { throw std::runtime_error("not implemented"); }

    int32_t StateSpaceImpl::get_distance_to_goal_state(const mimir::formalism::State& state) const { throw std::runtime_error("not implemented"); }

    int32_t StateSpaceImpl::get_distance_between_states(const mimir::formalism::State& from_state, const mimir::formalism::State& to_state) const
    {
        throw std::runtime_error("not implemented");
    }

    int32_t StateSpaceImpl::get_longest_distance_to_goal_state() const { throw std::runtime_error("not implemented"); }

    int32_t StateSpaceImpl::get_distance_from_initial_state(const mimir::formalism::State& state) const { throw std::runtime_error("not implemented"); }

    mimir::formalism::State StateSpaceImpl::sample_state() const { throw std::runtime_error("not implemented"); }

    mimir::formalism::State StateSpaceImpl::sample_state_with_distance_to_goal(int32_t distance) const { throw std::runtime_error("not implemented"); }

    mimir::formalism::State StateSpaceImpl::sample_dead_end_state() const { throw std::runtime_error("not implemented"); }

    std::vector<mimir::formalism::State> StateSpaceImpl::get_goal_states() const { throw std::runtime_error("not implemented"); }

    uint64_t StateSpaceImpl::num_states() const { throw std::runtime_error("not implemented"); }

    uint64_t StateSpaceImpl::num_transitions() const { throw std::runtime_error("not implemented"); }

    uint64_t StateSpaceImpl::num_goal_states() const { throw std::runtime_error("not implemented"); }

    uint64_t StateSpaceImpl::num_dead_end_states() const { throw std::runtime_error("not implemented"); }

    std::ostream& operator<<(std::ostream& os, const mimir::planners::StateSpace& state_space)
    {
        os << "# States: " << state_space->num_states() << "; # Transitions: " << state_space->num_transitions();
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const mimir::planners::StateSpaceList& state_spaces)
    {
        print_vector(os, state_spaces);
        return os;
    }
}  // namespace mimir::planners
