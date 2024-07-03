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

#ifndef MIMIR_DATASETS_FAITHFUL_ABSTRACTION_HPP_
#define MIMIR_DATASETS_FAITHFUL_ABSTRACTION_HPP_

#include "mimir/datasets/abstraction.hpp"
#include "mimir/datasets/state_space.hpp"
#include "mimir/datasets/transition_system.hpp"
#include "mimir/graphs/certificate.hpp"
#include "mimir/graphs/object_graph.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/successor_state_generator.hpp"

#include <loki/details/utils/filesystem.hpp>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

namespace mimir
{

/// @brief FaithfulAbstractState encapsulates data of an abstract state in a faithful abstraction.
class FaithfulAbstractState
{
private:
    StateId m_id;
    StateIndex m_index;
    State m_state;
    Certificate m_certificate;

public:
    FaithfulAbstractState(StateIndex index, State state, Certificate certificate);

    [[nodiscard]] bool operator==(const FaithfulAbstractState& other) const;
    [[nodiscard]] size_t hash() const;

    StateIndex get_index() const;
    State get_state() const;
    const Certificate& get_certificate() const;
};

using FaithfulAbstractStateList = std::vector<FaithfulAbstractState>;
template<typename T>
using FaithfulAbstractStateMap = std::unordered_map<FaithfulAbstractState, T, loki::Hash<FaithfulAbstractState>, loki::EqualTo<FaithfulAbstractState>>;
using CertificateToStateIndexMap = std::unordered_map<Certificate, StateIndex, loki::Hash<Certificate>, loki::EqualTo<Certificate>>;

/// @brief FaithfulAbstraction implements abstractions based on isomorphism testing.
/// Source: https://mrlab.ai/papers/drexler-et-al-icaps2024wsprl.pdf
class FaithfulAbstraction
{
private:
    /* Meta data */
    bool m_mark_true_goal_literals;
    bool m_use_unit_cost_one;

    /* Memory */
    std::shared_ptr<PDDLParser> m_parser;
    std::shared_ptr<IAAG> m_aag;
    std::shared_ptr<SuccessorStateGenerator> m_ssg;

    /* States */
    FaithfulAbstractStateList m_states;
    StateMap<StateIndex> m_concrete_to_abstract_state;
    CertificateToStateIndexMap m_states_by_certificate;
    StateIndex m_initial_state;
    StateIndexSet m_goal_states;
    StateIndexSet m_deadend_states;
    std::vector<StateIndexList> m_forward_successor_adjacency_lists;
    std::vector<StateIndexList> m_backward_successor_adjacency_lists;

    /* Transitions */
    TransitionList m_transitions;
    std::vector<TransitionIndexList> m_forward_transition_adjacency_lists;
    std::vector<TransitionIndexList> m_backward_transition_adjacency_lists;

    /* Distances */
    std::vector<double> m_goal_distances;

    /* Preallocated memory to compute abstract state of concrete state. */
    nauty_wrapper::Graph m_nauty_graph;
    ObjectGraphFactory m_object_graph_factory;

    /// @brief Constructs a state state from data.
    /// The create function calls this constructor and ensures that
    /// the state space is in a legal state allowing other parts of
    /// the code base to operate on the invariants in the implementation.
    FaithfulAbstraction(bool mark_true_goal_literals,
                        bool use_unit_cost_one,
                        std::shared_ptr<PDDLParser> parser,
                        std::shared_ptr<IAAG> aag,
                        std::shared_ptr<SuccessorStateGenerator> ssg,
                        FaithfulAbstractStateList states,
                        StateMap<StateIndex> concrete_to_abstract_state,
                        CertificateToStateIndexMap states_by_certificate,
                        StateIndex initial_state,
                        StateIndexSet goal_states,
                        StateIndexSet deadend_states,
                        std::vector<StateIndexList> forward_successor_adjacency_lists,
                        std::vector<StateIndexList> backward_successor_adjacency_lists,
                        TransitionList transitions,
                        std::vector<TransitionIndexList> forward_transition_adjacency_lists,
                        std::vector<TransitionIndexList> backward_transition_adjacency_lists,
                        std::vector<double> goal_distances);

public:
    static std::optional<FaithfulAbstraction> create(const fs::path& domain_filepath,
                                                     const fs::path& problem_filepath,
                                                     bool mark_true_goal_literals = false,
                                                     bool use_unit_cost_one = true,
                                                     bool remove_if_unsolvable = true,
                                                     bool compute_complete_abstraction_mapping = false,
                                                     uint32_t max_num_states = std::numeric_limits<uint32_t>::max(),
                                                     uint32_t timeout_ms = std::numeric_limits<uint32_t>::max());

    /// @brief Compute a faithful abstraction if within resource limits.
    /// @param parser is the external PDDL parser.
    /// @param aag is the external applicable action generator.
    /// @param ssg is the external successor state generator.
    /// @param mark_true_goal_literals whether satisfied goal atoms should be marked.
    /// @param use_unit_cost_one whether costs along transitions are 1 or the action costs.
    /// @param remove_if_unsolvable whether an abstraction should be discared if unsolvable.
    /// @param compute_complete_abstraction_mapping whether an to compute the complete abstraction mapping.
    /// @param max_num_states the maximum number of abstract states.
    /// @param timeout_ms the maximum time to compute the abstraction.
    /// @return std::nullopt if discarded, or otherwise, a FaithfulAbstraction.
    static std::optional<FaithfulAbstraction> create(std::shared_ptr<PDDLParser> parser,
                                                     std::shared_ptr<IAAG> aag,
                                                     std::shared_ptr<SuccessorStateGenerator> ssg,
                                                     bool mark_true_goal_literals = false,
                                                     bool use_unit_cost_one = true,
                                                     bool remove_if_unsolvable = true,
                                                     bool compute_complete_abstraction_mapping = false,
                                                     uint32_t max_num_states = std::numeric_limits<uint32_t>::max(),
                                                     uint32_t timeout_ms = std::numeric_limits<uint32_t>::max());

    static std::vector<FaithfulAbstraction> create(const fs::path& domain_filepath,
                                                   const std::vector<fs::path>& problem_filepaths,
                                                   bool mark_true_goal_literals = false,
                                                   bool use_unit_cost_one = true,
                                                   bool remove_if_unsolvable = true,
                                                   bool compute_complete_abstraction_mapping = false,
                                                   bool sort_ascending_by_num_states = true,
                                                   uint32_t max_num_states = std::numeric_limits<uint32_t>::max(),
                                                   uint32_t timeout_ms = std::numeric_limits<uint32_t>::max(),
                                                   uint32_t num_threads = std::thread::hardware_concurrency());

    static std::vector<FaithfulAbstraction>
    create(const std::vector<std::tuple<std::shared_ptr<PDDLParser>, std::shared_ptr<IAAG>, std::shared_ptr<SuccessorStateGenerator>>>& memories,
           bool mark_true_goal_literals = false,
           bool use_unit_cost_one = true,
           bool remove_if_unsolvable = true,
           bool compute_complete_abstraction_mapping = false,
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
     * Getters.
     */

    /* Meta data */
    bool get_mark_true_goal_literals() const;
    bool get_use_unit_cost_one() const;

    /* Memory */
    const std::shared_ptr<PDDLParser>& get_pddl_parser() const;
    const std::shared_ptr<IAAG>& get_aag() const;
    const std::shared_ptr<SuccessorStateGenerator>& get_ssg() const;

    /* States */
    const FaithfulAbstractStateList& get_states() const;
    const StateMap<StateIndex>& get_concrete_to_abstract_state() const;
    const CertificateToStateIndexMap& get_states_by_certificate() const;
    StateIndex get_initial_state() const;
    const StateIndexSet& get_goal_states() const;
    const StateIndexSet& get_deadend_states() const;
    const std::vector<StateIndexList>& get_forward_successor_adjacency_lists() const;
    const std::vector<StateIndexList>& get_backward_successor_adjacency_lists() const;
    size_t get_num_states() const;
    size_t get_num_goal_states() const;
    size_t get_num_deadend_states() const;
    bool is_goal_state(StateIndex state) const;
    bool is_deadend_state(StateIndex state) const;
    bool is_alive_state(StateIndex state) const;

    /* Transitions */
    const TransitionList& get_transitions() const;
    TransitionCost get_transition_cost(TransitionIndex transition) const;
    const std::vector<TransitionIndexList>& get_forward_transition_adjacency_lists() const;
    const std::vector<TransitionIndexList>& get_backward_transition_adjacency_lists() const;
    size_t get_num_transitions() const;

    /* Distances */
    const std::vector<double>& get_goal_distances() const;
};

using FaithfulAbstractionList = std::vector<FaithfulAbstraction>;

/**
 * Static assertions
 */

static_assert(IsAbstraction<FaithfulAbstraction>);

}

#endif
