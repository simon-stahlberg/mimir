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
template<typename T>
using GlobalFaithfulAbstractStateMap = std::unordered_map<GlobalFaithfulAbstractState, T, loki::Hash<GlobalFaithfulAbstractState>>;

class GlobalFaithfulAbstraction
{
private:
    /* Meta data */
    bool m_mark_true_goal_atoms;
    bool m_use_unit_cost_one;
    AbstractionId m_id;

    /* Memory */
    std::shared_ptr<FaithfulAbstractionList> m_abstractions;

    /* States */
    // Note that state.get_id() does not yield the index within the abstraction.
    // Use abstraction.get_state_index(state) instead.
    GlobalFaithfulAbstractStateList m_states;
    GlobalFaithfulAbstractStateMap<StateIndex> m_state_to_index;
    CertificateToStateIndexMap m_states_by_certificate;
    size_t m_num_isomorphic_states;
    size_t m_num_non_isomorphic_states;

    /* Preallocated memory to compute abstract state of concrete state. */
    nauty_wrapper::Graph m_nauty_graph;
    ObjectGraphFactory m_object_graph_factory;

    GlobalFaithfulAbstraction(bool mark_true_goal_atoms,
                              bool use_unit_cost_one,
                              AbstractionId id,
                              std::shared_ptr<FaithfulAbstractionList> abstractions,
                              GlobalFaithfulAbstractStateList states,
                              GlobalFaithfulAbstractStateMap<StateIndex> state_to_index,
                              CertificateToStateIndexMap states_by_certificate,
                              size_t num_isomorphic_states,
                              size_t num_non_isomorphic_states);

public:
    static std::vector<GlobalFaithfulAbstraction> create(const fs::path& domain_filepath,
                                                         const std::vector<fs::path>& problem_filepaths,
                                                         bool mark_true_goal_atoms = false,
                                                         bool use_unit_cost_one = true,
                                                         bool remove_if_unsolvable = true,
                                                         bool sort_ascending_by_num_states = true,
                                                         uint32_t max_num_states = std::numeric_limits<uint32_t>::max(),
                                                         uint32_t timeout_ms = std::numeric_limits<uint32_t>::max(),
                                                         uint32_t num_threads = std::thread::hardware_concurrency());

    static std::vector<GlobalFaithfulAbstraction>
    create(const std::vector<std::tuple<std::shared_ptr<PDDLParser>, std::shared_ptr<IAAG>, std::shared_ptr<ISSG>>>& memories,
           bool mark_true_goal_atoms = false,
           bool use_unit_cost_one = true,
           bool remove_if_unsolvable = true,
           bool sort_ascending_by_num_states = true,
           uint32_t max_num_states = std::numeric_limits<uint32_t>::max(),
           uint32_t timeout_ms = std::numeric_limits<uint32_t>::max(),
           uint32_t num_threads = std::thread::hardware_concurrency());

    /**
     * Abstraction functionality
     */

    StateIndex get_abstract_state_index(State concrete_state);

    /**
     * Extended functionality
     */

    std::vector<double> compute_shortest_distances_from_states(const StateIndexList& states, bool forward = true) const;

    std::vector<std::vector<double>> compute_pairwise_shortest_state_distances(bool forward = true) const;

    /**
     * Getters
     */

    /* Meta data */
    bool get_mark_true_goal_atoms() const;
    bool get_use_unit_cost_one() const;
    AbstractionId get_id() const;

    /* Memory */
    const FaithfulAbstractionList& get_abstractions() const;

    /* States */
    const GlobalFaithfulAbstractStateList& get_states() const;
    StateIndex get_state_index(const GlobalFaithfulAbstractState& state) const;
    const CertificateToStateIndexMap& get_states_by_certificate() const;
    StateIndex get_initial_state() const;
    const StateIndexSet& get_goal_states() const;
    const StateIndexSet& get_deadend_states() const;
    size_t get_num_states() const;
    size_t get_num_goal_states() const;
    size_t get_num_deadend_states() const;
    bool is_goal_state(StateIndex state) const;
    bool is_deadend_state(StateIndex state) const;
    bool is_alive_state(StateIndex state) const;
    size_t get_num_isomorphic_states() const;
    size_t get_num_non_isomorphic_states() const;

    /* Transitions */
    size_t get_num_transitions() const;
    const std::vector<TransitionList>& get_forward_transitions() const;
    const std::vector<TransitionList>& get_backward_transitions() const;

    /* Distances */
    const std::vector<double>& get_goal_distances() const;
};

/**
 * Static assertions
 */

static_assert(IsTransitionSystem<GlobalFaithfulAbstraction>);
static_assert(IsAbstraction<GlobalFaithfulAbstraction>);

}

#endif
