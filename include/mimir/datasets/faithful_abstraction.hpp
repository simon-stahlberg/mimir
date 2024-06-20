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

#include "mimir/graphs/object_graph.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/successor_state_generators.hpp"

#include <loki/details/utils/filesystem.hpp>
#include <memory>
#include <thread>
#include <unordered_set>
#include <vector>

namespace mimir
{
using AbstractStateId = int;
using AbstractStateIdList = std::vector<AbstractStateId>;
using AbstractStateIdSet = std::unordered_set<AbstractStateId>;

using AbstractionId = int;

class FaithfulAbstractionImpl;
using FaithfulAbstraction = std::shared_ptr<const FaithfulAbstractionImpl>;
using FaithfulAbstractionList = std::vector<FaithfulAbstraction>;

class FaithfulAbstractionImpl
{
public:
    class AbstractState
    {
    private:
        AbstractStateId m_id;
        State m_state;

    public:
        AbstractState(AbstractStateId id, State state);

        AbstractStateId get_id() const;
        State get_state() const;
    };

    using AbstractStateList = std::vector<AbstractState>;

private:
    // Memory
    std::unique_ptr<PDDLParser> m_parser;
    std::shared_ptr<GroundedAAG> m_aag;
    std::shared_ptr<SSG> m_ssg;

    // States
    AbstractStateList m_abstract_states;
    std::unordered_map<std::string, AbstractStateId> m_abstract_states_by_certificate;
    AbstractStateId m_abstract_initial_state;
    AbstractStateIdSet m_abstract_goal_states;
    AbstractStateIdSet m_abstract_deadend_states;

    // Transitions
    size_t m_num_abstract_transitions;
    std::vector<AbstractStateIdList> m_forward_successors;
    std::vector<AbstractStateIdList> m_backward_successors;

    // Distances
    std::vector<int> m_abstract_goal_distances;

    /// @brief Constructs a state state from data.
    /// The create function calls this constructor and ensures that
    /// the state space is in a legal state allowing other parts of
    /// the code base to operate on the invariants in the implementation.
    FaithfulAbstractionImpl(std::unique_ptr<PDDLParser>&& parser,
                            std::shared_ptr<GroundedAAG> aag,
                            std::shared_ptr<SSG> ssg,
                            AbstractStateList abstract_states,
                            std::unordered_map<std::string, AbstractStateId> abstract_states_by_certificate,
                            AbstractStateId abstract_initial_state,
                            AbstractStateIdSet abstract_goal_states,
                            AbstractStateIdSet abstract_deadend_states,
                            size_t num_abstract_transitions,
                            std::vector<AbstractStateIdList> forward_successors,
                            std::vector<AbstractStateIdList> backward_successors,
                            std::vector<int> abstract_goal_distances);

public:
    /// @brief Perform BrFS from the initial state while pruning isomorphic states.
    /// @param domain_filepath
    /// @param problem_filpath
    /// @param max_num_states
    /// @param timeout_ms
    /// @return
    static FaithfulAbstraction
    create(const fs::path& domain_file_path, const fs::path& problem_file_path, const size_t max_num_states, const size_t timeout_ms);

    static FaithfulAbstractionList create(const fs::path& domain_file_path,
                                          const std::vector<fs::path>& problem_file_paths,
                                          const size_t max_num_states,
                                          const size_t timeout_ms,
                                          const size_t num_threads = std::thread::hardware_concurrency());

    /**
     * Extended functionality
     */

    std::vector<int> compute_shortest_distances_from_states(const AbstractStateIdList& abstract_states, bool forward = true) const;

    /**
     * Getters.
     */

    // States
    const AbstractStateList& get_abstract_states() const;
    const std::unordered_map<std::string, AbstractStateId>& get_abstract_states_by_certificate() const;
    AbstractStateId get_abstract_initial_state() const;
    const AbstractStateIdSet& get_abstract_goal_states() const;
    const AbstractStateIdSet& get_abstract_deadend_states() const;
    size_t get_num_abstract_states() const;
    size_t get_num_abstract_goal_states() const;
    size_t get_num_abstract_deadend_states() const;

    // Transitions
    size_t get_num_abstract_transitions() const;
    const std::vector<AbstractStateIdList>& get_forward_successors() const;
    const std::vector<AbstractStateIdList>& get_backward_successors() const;

    // Distances
    const std::vector<int>& get_abstract_goal_distances() const;
};

using CombinedAbstractStateId = int;
using CombinedAbstractStateIdList = std::vector<CombinedAbstractStateId>;
using CombinedAbstractStateIdSet = std::unordered_set<CombinedAbstractStateId>;

class CombinedFaithfulAbstraction
{
public:
    /// @brief CombinedAbstractState encodes information necessary to retrieve
    /// information about the concrete underlying state such as Problem, PDDLFactories, SSG, AAG, etc.
    class CombinedAbstractState
    {
    private:
        CombinedAbstractStateId m_id;
        AbstractionId m_abstraction_id;
        AbstractStateId m_abstract_state_id;

    public:
        CombinedAbstractState(CombinedAbstractStateId id, AbstractionId abstraction_id, AbstractStateId abstract_state_id);

        CombinedAbstractStateId get_id() const;
        AbstractionId get_abstraction_id() const;
        AbstractStateId get_abstract_state_id() const;
    };

    using CombinedAbstractStateList = std::vector<CombinedAbstractState>;

private:
    // States
    CombinedAbstractStateIdList m_combined_states;
    CombinedAbstractStateId m_combined_initial_state;
    CombinedAbstractStateIdSet m_combined_goal_states;
    CombinedAbstractStateIdSet m_combined_deadend_states;

    // Transitions
    size_t m_num_abstract_transitions;
    std::vector<CombinedAbstractStateIdList> m_forward_successors;
    std::vector<CombinedAbstractStateIdList> m_backward_successors;

    // Distances
    std::vector<int> m_combined_goal_distances;

public:
    explicit CombinedFaithfulAbstraction(FaithfulAbstractionList abstractions);

    /**
     * Getters
     */

    // States
    const CombinedAbstractStateIdList& get_combined_states() const;
    CombinedAbstractStateId get_combined_initial_state() const;
    const CombinedAbstractStateIdSet& get_combined_goal_states() const;
    const CombinedAbstractStateIdSet& get_combined_deadend_states() const;
    size_t get_num_combined_states() const;
    size_t get_num_combined_goal_states() const;
    size_t get_num_combined_deadend_states() const;

    // Transitions
    size_t get_num_combined_transitions() const;
    const std::vector<CombinedAbstractStateIdList>& get_forward_successors() const;
    const std::vector<CombinedAbstractStateIdList>& get_backward_successors() const;

    // Distances
    const std::vector<int>& get_combined_goal_distances() const;
};

using CombinedFaithfulAbstractionList = std::vector<CombinedFaithfulAbstraction>;

/// @brief CombinedFaithfulAbstractions applies isomorphism reduction across instances.
/// Faithful abstractions with no novel states get pruned.
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
