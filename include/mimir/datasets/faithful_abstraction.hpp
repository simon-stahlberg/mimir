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

#include "mimir/common/grouped_vector.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/common/types.hpp"
#include "mimir/datasets/abstraction_interface.hpp"
#include "mimir/datasets/state_space.hpp"
#include "mimir/datasets/transitions.hpp"
#include "mimir/graphs/certificate.hpp"
#include "mimir/graphs/object_graph.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/declarations.hpp"
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

/// @brief FaithfulAbstractionOptions enscapsulates options for creation of a single faithful abstraction with default arguments.
struct FaithfulAbstractionOptions
{
    bool mark_true_goal_literals = false;
    bool use_unit_cost_one = true;
    bool remove_if_unsolvable = true;
    bool compute_complete_abstraction_mapping = false;
    uint32_t max_num_concrete_states = std::numeric_limits<uint32_t>::max();
    uint32_t max_num_abstract_states = std::numeric_limits<uint32_t>::max();
    uint32_t timeout_ms = std::numeric_limits<uint32_t>::max();
    ObjectGraphPruningStrategyEnum pruning_strategy = ObjectGraphPruningStrategyEnum::None;
};

/// @brief FaithfulAbstractionOptions enscapsulates options for creation of a collection of faithful abstractions with default arguments.
struct FaithfulAbstractionsOptions
{
    FaithfulAbstractionOptions fa_options;
    bool sort_ascending_by_num_states = true;
    uint32_t num_threads = std::thread::hardware_concurrency();
};

/// @brief FaithfulAbstractState encapsulates data of an abstract state in a faithful abstraction.
class FaithfulAbstractState
{
private:
    StateIndex m_index;
    std::span<const State> m_states;
    std::shared_ptr<const Certificate> m_certificate;

public:
    FaithfulAbstractState(StateIndex index, std::span<const State> states, std::shared_ptr<const Certificate> certificate);

    bool operator==(const FaithfulAbstractState& other) const;
    size_t hash() const;

    StateIndex get_index() const;
    std::span<const State> get_states() const;
    State get_representative_state() const;
    const std::shared_ptr<const Certificate>& get_certificate() const;
};

using FaithfulAbstractStateList = std::vector<FaithfulAbstractState>;
template<typename T>
using FaithfulAbstractStateMap = std::unordered_map<FaithfulAbstractState, T, loki::Hash<FaithfulAbstractState>, loki::EqualTo<FaithfulAbstractState>>;
using CertificateToStateIndexMap = std::unordered_map<std::shared_ptr<const Certificate>, StateIndex, SharedPtrHash<Certificate>, SharedPtrEqual<Certificate>>;

/// @brief FaithfulAbstraction implements abstractions based on isomorphism testing.
/// Source: https://mrlab.ai/papers/drexler-et-al-icaps2024wsprl.pdf
class FaithfulAbstraction
{
private:
    /* Meta data */
    Problem m_problem;
    bool m_mark_true_goal_literals;
    bool m_use_unit_cost_one;

    /* Memory */
    std::shared_ptr<PDDLFactories> m_pddl_factories;
    std::shared_ptr<IAAG> m_aag;
    std::shared_ptr<SuccessorStateGenerator> m_ssg;

    /* States */
    FaithfulAbstractStateList m_states;
    // Persistent and sorted to store slices in the abstract states.
    std::shared_ptr<const StateList> m_concrete_states_by_abstract_state;
    StateMap<StateIndex> m_concrete_to_abstract_state;
    StateIndex m_initial_state;
    StateIndexSet m_goal_states;
    StateIndexSet m_deadend_states;

    /* Transitions */
    IndexGroupedVector<const AbstractTransition> m_transitions;
    // Persistent and sorted to store slices in the abstract transitions.
    std::shared_ptr<const GroundActionList> m_ground_actions_by_source_and_target;

    /* Distances */
    std::vector<double> m_goal_distances;

    /* Additional */
    std::map<double, StateIndexList> m_states_by_goal_distance;

    /// @brief Constructs a state state from data.
    /// The create function calls this constructor and ensures that
    /// the state space is in a legal state allowing other parts of
    /// the code base to operate on the invariants in the implementation.
    FaithfulAbstraction(Problem problem,
                        bool mark_true_goal_literals,
                        bool use_unit_cost_one,
                        std::shared_ptr<PDDLFactories> factories,
                        std::shared_ptr<IAAG> aag,
                        std::shared_ptr<SuccessorStateGenerator> ssg,
                        FaithfulAbstractStateList states,
                        std::shared_ptr<const StateList> concrete_states_by_abstract_state,
                        StateMap<StateIndex> concrete_to_abstract_state,
                        StateIndex initial_state,
                        StateIndexSet goal_states,
                        StateIndexSet deadend_states,
                        IndexGroupedVector<const AbstractTransition> transitions,
                        std::shared_ptr<const GroundActionList> ground_actions_by_source_and_target,
                        std::vector<double> goal_distances);

public:
    using StateType = FaithfulAbstractState;
    using TransitionType = AbstractTransition;

    static std::optional<FaithfulAbstraction>
    create(const fs::path& domain_filepath, const fs::path& problem_filepath, const FaithfulAbstractionOptions& options = FaithfulAbstractionOptions());

    /// @brief Compute a faithful abstraction if within resource limits.
    /// @param problem is the problem.
    /// @param factories is the external PDDL factories.
    /// @param aag is the external applicable action generator.
    /// @param ssg is the external successor state generator.
    /// @param mark_true_goal_literals whether satisfied goal literals should be marked.
    /// @param use_unit_cost_one whether to use unit cost one or action costs.
    /// @param remove_if_unsolvable whether an abstraction should be discared if unsolvable.
    /// @param compute_complete_abstraction_mapping whether an to compute the complete abstraction mapping.
    /// @param max_num_concrete_states the maximum number of abstract states.
    /// @param timeout_ms the maximum time to compute the abstraction.
    /// @return std::nullopt if discarded, or otherwise, a FaithfulAbstraction.
    static std::optional<FaithfulAbstraction> create(Problem problem,
                                                     std::shared_ptr<PDDLFactories> factories,
                                                     std::shared_ptr<IAAG> aag,
                                                     std::shared_ptr<SuccessorStateGenerator> ssg,
                                                     const FaithfulAbstractionOptions& options = FaithfulAbstractionOptions());

    /// @brief Convenience function when sharing parsers, aags, ssgs is not relevant.
    static std::vector<FaithfulAbstraction> create(const fs::path& domain_filepath,
                                                   const std::vector<fs::path>& problem_filepaths,
                                                   const FaithfulAbstractionsOptions& options = FaithfulAbstractionsOptions());

    /// @brief Try to create a FaithfulAbstractionList from the given data and the given resource limits.
    /// @param memories External memory to problem, factories, aags, ssgs.
    /// @param mark_true_goal_literals whether satisfied goal literals should be marked.
    /// @param use_unit_cost_one whether to use unit cost one or action costs.
    /// @param remove_if_unsolvable whether to remove abstractions of unsolvable problems.
    /// @param sort_ascending_by_num_states whether the abstractions should be sorted ascending by the number of states.
    /// @param max_num_concrete_states The maximum number of states allowed in an abstraction.
    /// @param timeout_ms The maximum time spent on creating an abstraction.
    /// @param num_threads The number of threads used for construction.
    /// @return FaithfulAbstractionList contains the FaithfulAbstractions for which the construction is within the given resource limits.
    static std::vector<FaithfulAbstraction>
    create(const std::vector<std::tuple<Problem, std::shared_ptr<PDDLFactories>, std::shared_ptr<IAAG>, std::shared_ptr<SuccessorStateGenerator>>>& memories,
           const FaithfulAbstractionsOptions& options = FaithfulAbstractionsOptions());

    /**
     * Abstraction functionality
     */

    StateIndex get_abstract_state_index(State concrete_state) const;

    /**
     * Extended functionality
     */

    std::vector<double> compute_shortest_distances_from_states(const StateIndexList& states, bool forward = true) const;

    std::vector<std::vector<double>> compute_pairwise_shortest_state_distances(bool forward = true) const;

    /**
     * Getters.
     */

    /* Meta data */
    Problem get_problem() const;
    bool get_mark_true_goal_literals() const;
    bool get_use_unit_cost_one() const;

    /* Memory */
    const std::shared_ptr<PDDLFactories>& get_pddl_factories() const;
    const std::shared_ptr<IAAG>& get_aag() const;
    const std::shared_ptr<SuccessorStateGenerator>& get_ssg() const;

    /* States */
    const FaithfulAbstractStateList& get_states() const;
    const StateMap<StateIndex>& get_concrete_to_abstract_state() const;
    StateIndex get_initial_state() const;
    const StateIndexSet& get_goal_states() const;
    const StateIndexSet& get_deadend_states() const;
    TargetStateIndexIterator<AbstractTransition> get_target_states(StateIndex source) const;
    SourceStateIndexIterator<AbstractTransition> get_source_states(StateIndex target) const;
    size_t get_num_states() const;
    size_t get_num_goal_states() const;
    size_t get_num_deadend_states() const;
    bool is_goal_state(StateIndex state) const;
    bool is_deadend_state(StateIndex state) const;
    bool is_alive_state(StateIndex state) const;

    /* Transitions */
    const AbstractTransitionList& get_transitions() const;
    TransitionCost get_transition_cost(TransitionIndex transition) const;
    ForwardTransitionIndexIterator<AbstractTransition> get_forward_transition_indices(StateIndex source) const;
    BackwardTransitionIndexIterator<AbstractTransition> get_backward_transition_indices(StateIndex target) const;
    ForwardTransitionIterator<AbstractTransition> get_forward_transitions(StateIndex source) const;
    BackwardTransitionIterator<AbstractTransition> get_backward_transitions(StateIndex target) const;
    size_t get_num_transitions() const;

    /* Distances */
    const std::vector<double>& get_goal_distances() const;

    /* Additional */
    const std::map<double, StateIndexList>& get_states_by_goal_distance() const;
};

using FaithfulAbstractionList = std::vector<FaithfulAbstraction>;

/**
 * Static assertions
 */

static_assert(IsAbstraction<FaithfulAbstraction>);

/**
 * Pretty printing
 */

extern std::ostream& operator<<(std::ostream& out, const FaithfulAbstraction& abstraction);

}

#endif
