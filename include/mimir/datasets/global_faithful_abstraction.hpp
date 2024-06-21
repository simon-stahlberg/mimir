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

#ifndef MIMIR_DATASETS_GLOBAL_FAITHFUL_ABSTRACTION_HPP_
#define MIMIR_DATASETS_GLOBAL_FAITHFUL_ABSTRACTION_HPP_

#include "mimir/datasets/abstraction.hpp"
#include "mimir/datasets/faithful_abstraction.hpp"
#include "mimir/datasets/transition_system.hpp"
#include "mimir/graphs/object_graph.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/successor_state_generators.hpp"

#include <loki/details/utils/filesystem.hpp>
#include <memory>
#include <optional>
#include <thread>
#include <unordered_set>
#include <vector>

namespace mimir
{

/// @brief CombinedAbstractState encodes information necessary to retrieve
/// information about the concrete underlying state such as Problem, PDDLFactories, SSG, AAG, etc.
class GlobalFaithfulAbstractState
{
private:
    StateId m_id;
    AbstractionId m_abstraction_id;
    FaithfulAbstractState m_abstract_state;

public:
    GlobalFaithfulAbstractState(StateId id, AbstractionId abstraction_id, FaithfulAbstractState abstract_state);

    StateId get_id() const;
    AbstractionId get_abstraction_id() const;
    FaithfulAbstractState get_abstract_state() const;
};

using GlobalFaithfulAbstractStateList = std::vector<GlobalFaithfulAbstractState>;

class GlobalFaithfulAbstraction
{
public:
private:
    // Memory
    std::shared_ptr<FaithfulAbstractionList> m_abstractions;

    // States
    GlobalFaithfulAbstractStateList m_states;
    StateId m_initial_state;
    StateIdSet m_goal_states;
    StateIdSet m_deadend_states;

    // Transitions
    size_t m_num_transitions;
    std::vector<TransitionList> m_forward_transitions;
    std::vector<TransitionList> m_backward_transitions;

    // Distances
    std::vector<int> m_goal_distances;

    GlobalFaithfulAbstraction(GlobalFaithfulAbstractStateList states,
                              StateId initial_state,
                              StateIdSet goal_states,
                              StateIdSet deadend_states,
                              size_t num_transitions,
                              std::vector<TransitionList> forward_transitions,
                              std::vector<TransitionList> backward_transitions,
                              std::vector<int> goal_distances);

public:
    static std::vector<GlobalFaithfulAbstraction> create(const fs::path& domain_filepath,
                                                         const std::vector<fs::path>& problem_filepaths,
                                                         const size_t max_num_states = std::numeric_limits<size_t>::max(),
                                                         const size_t timeout_ms = std::numeric_limits<size_t>::max(),
                                                         const size_t num_threads = std::thread::hardware_concurrency());

    /**
     * Getters
     */

    // States
    const GlobalFaithfulAbstractStateList& get_states() const;
    StateId get_initial_state() const;
    const StateIdSet& get_goal_states() const;
    const StateIdSet& get_deadend_states() const;
    size_t get_num_states() const;
    size_t get_num_goal_states() const;
    size_t get_num_deadend_states() const;

    // Transitions
    size_t get_num_transitions() const;
    const std::vector<TransitionList>& get_forward_transitions() const;
    const std::vector<TransitionList>& get_backward_transitions() const;

    // Distances
    const std::vector<int>& get_goal_distances() const;
};

}

#endif
