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
#include "mimir/datasets/transition_system.hpp"
#include "mimir/graphs/certificate.hpp"
#include "mimir/graphs/object_graph.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/successor_state_generators.hpp"

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
    FaithfulAbstractState(StateId id, StateIndex index, State state, Certificate certificate);

    [[nodiscard]] bool operator==(const FaithfulAbstractState& other) const;
    [[nodiscard]] size_t hash() const;

    StateId get_id() const;
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
    bool m_mark_true_goal_atoms;
    bool m_use_unit_cost_one;

    /* Memory */
    std::shared_ptr<PDDLParser> m_parser;
    std::shared_ptr<IAAG> m_aag;
    std::shared_ptr<ISSG> m_ssg;

    /* States */
    FaithfulAbstractStateList m_states;
    CertificateToStateIndexMap m_states_by_certificate;
    StateIndex m_initial_state;
    StateIndexSet m_goal_states;
    StateIndexSet m_deadend_states;

    /* Transitions */
    size_t m_num_transitions;
    std::vector<TransitionList> m_forward_transitions;
    std::vector<TransitionList> m_backward_transitions;

    /* Distances */
    std::vector<double> m_goal_distances;

    /* Preallocated memory to compute abstract state of concrete state. */
    nauty_wrapper::Graph m_nauty_graph;
    ObjectGraphFactory m_object_graph_factory;

    /// @brief Constructs a state state from data.
    /// The create function calls this constructor and ensures that
    /// the state space is in a legal state allowing other parts of
    /// the code base to operate on the invariants in the implementation.
    FaithfulAbstraction(bool mark_true_goal_atoms,
                        bool use_unit_cost_one,
                        std::shared_ptr<PDDLParser> parser,
                        std::shared_ptr<IAAG> aag,
                        std::shared_ptr<ISSG> ssg,
                        FaithfulAbstractStateList states,
                        CertificateToStateIndexMap states_by_certificate,
                        StateIndex initial_state,
                        StateIndexSet goal_states,
                        StateIndexSet deadend_states,
                        size_t num_transitions,
                        std::vector<TransitionList> forward_transitions,
                        std::vector<TransitionList> backward_transitions,
                        std::vector<double> goal_distances);

public:
    static std::optional<FaithfulAbstraction> create(const fs::path& domain_filepath,
                                                     const fs::path& problem_filepath,
                                                     bool mark_true_goal_atoms = false,
                                                     bool use_unit_cost_one = true,
                                                     bool remove_if_unsolvable = true,
                                                     uint32_t max_num_states = std::numeric_limits<uint32_t>::max(),
                                                     uint32_t timeout_ms = std::numeric_limits<uint32_t>::max());

    static std::optional<FaithfulAbstraction> create(std::shared_ptr<PDDLParser> parser,
                                                     std::shared_ptr<IAAG> aag,
                                                     std::shared_ptr<ISSG> ssg,
                                                     bool mark_true_goal_atoms = false,
                                                     bool use_unit_cost_one = true,
                                                     bool remove_if_unsolvable = true,
                                                     uint32_t max_num_states = std::numeric_limits<uint32_t>::max(),
                                                     uint32_t timeout_ms = std::numeric_limits<uint32_t>::max());

    static std::vector<FaithfulAbstraction> create(const fs::path& domain_filepath,
                                                   const std::vector<fs::path>& problem_filepaths,
                                                   bool mark_true_goal_atoms = false,
                                                   bool use_unit_cost_one = true,
                                                   bool remove_if_unsolvable = true,
                                                   bool sort_ascending_by_num_states = true,
                                                   uint32_t max_num_states = std::numeric_limits<uint32_t>::max(),
                                                   uint32_t timeout_ms = std::numeric_limits<uint32_t>::max(),
                                                   uint32_t num_threads = std::thread::hardware_concurrency());

    static std::vector<FaithfulAbstraction>
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
     * Getters.
     */

    /* Meta data */
    bool get_mark_true_goal_atoms() const;
    bool get_use_unit_cost_one() const;

    /* Memory */
    const std::shared_ptr<PDDLParser>& get_pddl_parser() const;
    const std::shared_ptr<IAAG>& get_aag() const;
    const std::shared_ptr<ISSG>& get_ssg() const;

    /* States */
    const FaithfulAbstractStateList& get_states() const;
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

    /* Transitions */
    size_t get_num_transitions() const;
    const std::vector<TransitionList>& get_forward_transitions() const;
    const std::vector<TransitionList>& get_backward_transitions() const;

    /* Distances */
    const std::vector<double>& get_goal_distances() const;
};

using FaithfulAbstractionList = std::vector<FaithfulAbstraction>;

/**
 * Static assertions
 */

static_assert(IsTransitionSystem<FaithfulAbstraction>);
static_assert(IsAbstraction<FaithfulAbstraction>);

}

#endif
