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
/// @brief FaithfulAbstractState encapsulates data of an abstract state in a faithful abstraction.
class FaithfulAbstractState
{
private:
    StateId m_id;
    State m_state;

public:
    FaithfulAbstractState(StateId id, State state);

    StateId get_id() const;
    State get_state() const;
};

using FaithfulAbstractStateList = std::vector<FaithfulAbstractState>;

/// @brief FaithfulAbstraction implements abstractions based on isomorphism testing.
/// Source: https://mrlab.ai/papers/drexler-et-al-icaps2024wsprl.pdf
class FaithfulAbstraction
{
private:
    // Meta data
    fs::path m_domain_filepath;
    fs::path m_problem_filepath;

    // Memory
    std::shared_ptr<PDDLParser> m_parser;
    std::shared_ptr<LiftedAAG> m_aag;
    std::shared_ptr<SSG> m_ssg;

    // States
    FaithfulAbstractStateList m_states;
    std::unordered_map<std::string, StateId> m_states_by_certificate;
    StateId m_initial_state;
    StateIdSet m_goal_states;
    StateIdSet m_deadend_states;

    // Transitions
    size_t m_num_transitions;
    std::vector<TransitionList> m_forward_transitions;
    std::vector<TransitionList> m_backward_transitions;

    // Distances
    std::vector<int> m_goal_distances;

    /// @brief Constructs a state state from data.
    /// The create function calls this constructor and ensures that
    /// the state space is in a legal state allowing other parts of
    /// the code base to operate on the invariants in the implementation.
    FaithfulAbstraction(fs::path domain_filepath,
                        fs::path problem_filepath,
                        std::shared_ptr<PDDLParser> parser,
                        std::shared_ptr<LiftedAAG> aag,
                        std::shared_ptr<SSG> ssg,
                        FaithfulAbstractStateList states,
                        std::unordered_map<std::string, StateId> states_by_certificate,
                        StateId initial_state,
                        StateIdSet goal_states,
                        StateIdSet deadend_states,
                        size_t num_transitions,
                        std::vector<TransitionList> forward_transitions,
                        std::vector<TransitionList> backward_transitions,
                        std::vector<int> goal_distances);

public:
    /// @brief Perform BrFS from the initial state while pruning isomorphic states.
    /// @param domain_filepath
    /// @param problem_filpath
    /// @param max_num_states
    /// @param timeout_ms
    /// @return
    static std::optional<FaithfulAbstraction> create(const fs::path& domain_filepath,
                                                     const fs::path& problem_filepath,
                                                     const size_t max_num_states = std::numeric_limits<size_t>::max(),
                                                     const size_t timeout_ms = std::numeric_limits<size_t>::max());

    static std::vector<FaithfulAbstraction> create(const fs::path& domain_filepath,
                                                   const std::vector<fs::path>& problem_filepaths,
                                                   const size_t max_num_states = std::numeric_limits<size_t>::max(),
                                                   const size_t timeout_ms = std::numeric_limits<size_t>::max(),
                                                   const size_t num_threads = std::thread::hardware_concurrency());

    /**
     * Extended functionality
     */

    std::vector<int> compute_shortest_distances_from_states(const StateIdList& states, bool forward = true) const;

    std::vector<std::vector<int>> compute_pairwise_shortest_state_distances(bool forward = true) const;

    /**
     * Getters.
     */

    // Meta data
    const fs::path& get_domain_filepath() const;
    const fs::path& get_problem_filepath() const;

    // States
    const FaithfulAbstractStateList& get_states() const;
    const std::unordered_map<std::string, StateId>& get_states_by_certificate() const;
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

using FaithfulAbstractionList = std::vector<FaithfulAbstraction>;

using CombinedStateId = int;
using CombinedStateIdList = std::vector<CombinedStateId>;
using CombinedStateIdSet = std::unordered_set<CombinedStateId>;

class CombinedFaithfulAbstraction
{
public:
    /// @brief CombinedAbstractState encodes information necessary to retrieve
    /// information about the concrete underlying state such as Problem, PDDLFactories, SSG, AAG, etc.
    class CombinedAbstractState
    {
    private:
        CombinedStateId m_id;
        AbstractionId m_abstraction_id;
        StateId m_abstract_state_id;

    public:
        CombinedAbstractState(CombinedStateId id, AbstractionId abstraction_id, StateId abstract_state_id);

        CombinedStateId get_id() const;
        AbstractionId get_abstraction_id() const;
        StateId get_abstract_state_id() const;
    };

    using CombinedAbstractStateList = std::vector<CombinedAbstractState>;

private:
    // States
    CombinedStateIdList m_states;
    CombinedStateId m_initial_state;
    CombinedStateIdSet m_goal_states;
    CombinedStateIdSet m_deadend_states;

    // Transitions
    size_t m_num_transitions;
    std::vector<CombinedStateIdList> m_forward_successors;
    std::vector<CombinedStateIdList> m_backward_successors;

    // Distances
    std::vector<int> m_goal_distances;

    CombinedFaithfulAbstraction(CombinedStateIdList states,
                                CombinedStateId initial_state,
                                CombinedStateIdSet goal_states,
                                CombinedStateIdSet deadend_states,
                                size_t num_transitions,
                                std::vector<CombinedStateIdList> forward_successors,
                                std::vector<CombinedStateIdList> backward_successors,
                                std::vector<int> goal_distances);

    friend class CombinedFaithfulAbstractions;

public:
    /**
     * Getters
     */

    // States
    const CombinedStateIdList& get_states() const;
    CombinedStateId get_initial_state() const;
    const CombinedStateIdSet& get_goal_states() const;
    const CombinedStateIdSet& get_deadend_states() const;
    size_t get_num_states() const;
    size_t get_num_goal_states() const;
    size_t get_num_deadend_states() const;

    // Transitions
    size_t get_num_transitions() const;
    const std::vector<CombinedStateIdList>& get_forward_successors() const;
    const std::vector<CombinedStateIdList>& get_backward_successors() const;

    // Distances
    const std::vector<int>& get_goal_distances() const;
};

using CombinedFaithfulAbstractionList = std::vector<CombinedFaithfulAbstraction>;

/// @brief CombinedFaithfulAbstractions applies isomorphism reduction across instances.
/// Faithful abstractions with no novel states get pruned.
/// The result depends on the order in which the abstractions were given.
class CombinedFaithfulAbstractions
{
private:
    FaithfulAbstractionList m_abstractions;
    CombinedFaithfulAbstraction::CombinedAbstractStateList m_combined_states;
    CombinedFaithfulAbstractionList m_combined_abstractions;

public:
    explicit CombinedFaithfulAbstractions(FaithfulAbstractionList abstractions);

    /**
     * Getters
     */

    const FaithfulAbstractionList& get_abstractions() const;
    const CombinedFaithfulAbstraction::CombinedAbstractStateList& get_combined_states() const;
    const CombinedFaithfulAbstractionList& get_combined_abstractions() const;
};

}

#endif
