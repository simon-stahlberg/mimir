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

#include "mimir/datasets/global_faithful_abstraction.hpp"

#include "mimir/algorithms/BS_thread_pool.hpp"
#include "mimir/common/timers.hpp"

#include <algorithm>
#include <cstdlib>
#include <deque>

namespace mimir
{
/**
 * GlobalFaithfulAbstractState
 */

GlobalFaithfulAbstractState::GlobalFaithfulAbstractState(StateId id, AbstractionId abstraction_id, FaithfulAbstractState abstract_state) :
    m_id(id),
    m_abstraction_id(abstraction_id),
    m_abstract_state(abstract_state)
{
}

StateId GlobalFaithfulAbstractState::get_id() const { return m_id; }

AbstractionId GlobalFaithfulAbstractState::get_abstraction_id() const { return m_abstraction_id; }

FaithfulAbstractState GlobalFaithfulAbstractState::get_abstract_state() const { return m_abstract_state; }

/**
 * GlobalFaithfulAbstraction
 */

GlobalFaithfulAbstraction::GlobalFaithfulAbstraction(GlobalFaithfulAbstractStateList states,
                                                     StateId initial_state,
                                                     StateIdSet goal_states,
                                                     StateIdSet deadend_states,
                                                     size_t num_transitions,
                                                     std::vector<TransitionList> forward_transitions,
                                                     std::vector<TransitionList> backward_transitions,
                                                     std::vector<int> goal_distances) :
    m_states(std::move(states)),
    m_initial_state(initial_state),
    m_goal_states(std::move(goal_states)),
    m_deadend_states(std::move(deadend_states)),
    m_num_transitions(num_transitions),
    m_forward_transitions(std::move(forward_transitions)),
    m_backward_transitions(std::move(backward_transitions)),
    m_goal_distances(std::move(goal_distances))
{
}

std::vector<GlobalFaithfulAbstraction> GlobalFaithfulAbstraction::create(const fs::path& domain_filepath,
                                                                         const std::vector<fs::path>& problem_filepaths,
                                                                         const size_t max_num_states = std::numeric_limits<size_t>::max(),
                                                                         const size_t timeout_ms = std::numeric_limits<size_t>::max(),
                                                                         const size_t num_threads = std::thread::hardware_concurrency())
{
    auto abstractions = std::vector<GlobalFaithfulAbstraction> {};

    return abstractions;
}

const GlobalFaithfulAbstractStateList& GlobalFaithfulAbstraction::get_states() const { return m_states; }

StateId GlobalFaithfulAbstraction::get_initial_state() const { return m_initial_state; }

const StateIdSet& GlobalFaithfulAbstraction::get_goal_states() const { return m_goal_states; }

const StateIdSet& GlobalFaithfulAbstraction::get_deadend_states() const { return m_deadend_states; }

size_t GlobalFaithfulAbstraction::get_num_states() const { return m_states.size(); }

size_t GlobalFaithfulAbstraction::get_num_goal_states() const { return m_goal_states.size(); }

size_t GlobalFaithfulAbstraction::get_num_deadend_states() const { return m_deadend_states.size(); }

size_t GlobalFaithfulAbstraction::get_num_transitions() const { return m_num_transitions; }

const std::vector<TransitionList>& GlobalFaithfulAbstraction::get_forward_transitions() const { return m_forward_transitions; }

const std::vector<TransitionList>& GlobalFaithfulAbstraction::get_backward_transitions() const { return m_backward_transitions; }

const std::vector<int>& GlobalFaithfulAbstraction::get_goal_distances() const { return m_goal_distances; }

}
