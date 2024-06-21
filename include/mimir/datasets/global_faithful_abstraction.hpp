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

/// @brief GlobalFaithfulAbstractState encapsulates data to access
/// the representative abstract state of a faithful abstraction.
class GlobalFaithfulAbstractState
{
private:
    StateId m_id;
    AbstractionId m_abstraction_id;
    StateId m_abstract_state_id;

public:
    GlobalFaithfulAbstractState(StateId id, AbstractionId abstraction_id, StateId abstract_state_id);

    [[nodiscard]] bool operator==(const GlobalFaithfulAbstractState& other) const;
    [[nodiscard]] size_t hash() const;

    StateId get_id() const;
    AbstractionId get_abstraction_id() const;
    StateId get_abstract_state_id() const;
};

using GlobalFaithfulAbstractStateList = std::vector<GlobalFaithfulAbstractState>;

class GlobalFaithfulAbstraction
{
public:
private:
    // Memory
    AbstractionId m_id;
    std::shared_ptr<FaithfulAbstractionList> m_abstractions;

    // States
    GlobalFaithfulAbstractStateList m_states;
    size_t m_num_isomorphic_states;
    size_t m_num_non_isomorphic_states;

    GlobalFaithfulAbstraction(AbstractionId id,
                              std::shared_ptr<FaithfulAbstractionList> abstractions,
                              GlobalFaithfulAbstractStateList states,
                              size_t num_isomorphic_states,
                              size_t num_non_isomorphic_states);

public:
    static std::vector<GlobalFaithfulAbstraction> create(const fs::path& domain_filepath,
                                                         const std::vector<fs::path>& problem_filepaths,
                                                         const size_t max_num_states = std::numeric_limits<size_t>::max(),
                                                         const size_t timeout_ms = std::numeric_limits<size_t>::max(),
                                                         const size_t num_threads = std::thread::hardware_concurrency());

    /**
     * Getters
     */

    // Meta data
    const fs::path& get_domain_filepath() const;
    const fs::path& get_problem_filepath() const;

    // Memory
    const FaithfulAbstractionList& get_abstractions() const;

    // States
    const GlobalFaithfulAbstractStateList& get_states() const;
    StateId get_initial_state() const;
    const StateIdSet& get_goal_states() const;
    const StateIdSet& get_deadend_states() const;
    size_t get_num_states() const;
    size_t get_num_goal_states() const;
    size_t get_num_deadend_states() const;
    size_t get_num_isomorphic_states() const;
    size_t get_num_non_isomorphic_states() const;

    // Transitions
    size_t get_num_transitions() const;
    const std::vector<TransitionList>& get_forward_transitions() const;
    const std::vector<TransitionList>& get_backward_transitions() const;

    // Distances
    const std::vector<int>& get_goal_distances() const;
};

}

#endif
