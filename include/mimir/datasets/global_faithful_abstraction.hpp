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

#include "mimir/datasets/abstraction_interface.hpp"
#include "mimir/datasets/faithful_abstraction.hpp"
#include "mimir/datasets/state_space.hpp"
#include "mimir/datasets/transitions.hpp"
#include "mimir/graphs/object_graph.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/successor_state_generator.hpp"

#include <loki/details/utils/filesystem.hpp>
#include <memory>
#include <optional>
#include <ranges>
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
    // The index within a GlobalFaithfulAbstraction.
    StateIndex m_index;
    // The index within a GlobalFaithfulAbstractionList.
    StateId m_global_index;
    // The indices to access the corresponding FaithfulAbstractState.
    AbstractionIndex m_faithful_abstraction_index;
    StateId m_faithful_abstract_state_index;

public:
    GlobalFaithfulAbstractState(StateIndex index,
                                StateIndex global_index,
                                AbstractionIndex faithful_abstraction_index,
                                StateIndex faithful_abstract_state_index);

    bool operator==(const GlobalFaithfulAbstractState& other) const;
    size_t hash() const;

    StateIndex get_index() const;
    StateIndex get_global_index() const;
    AbstractionIndex get_faithful_abstraction_index() const;
    StateIndex get_faithful_abstract_state_index() const;
};

using GlobalFaithfulAbstractStateList = std::vector<GlobalFaithfulAbstractState>;

class GlobalFaithfulAbstraction
{
private:
    /* Meta data */
    bool m_mark_true_goal_literals;
    bool m_use_unit_cost_one;
    AbstractionIndex m_index;

    /* Memory */
    std::shared_ptr<const FaithfulAbstractionList> m_abstractions;

    /* States */
    GlobalFaithfulAbstractStateList m_states;
    size_t m_num_isomorphic_states;
    size_t m_num_non_isomorphic_states;

    /* Additional */
    std::unordered_map<StateIndex, StateIndex> m_global_state_index_to_state_index;

    GlobalFaithfulAbstraction(bool mark_true_goal_literals,
                              bool use_unit_cost_one,
                              AbstractionIndex index,
                              std::shared_ptr<const FaithfulAbstractionList> abstractions,
                              GlobalFaithfulAbstractStateList states,
                              size_t num_isomorphic_states,
                              size_t num_non_isomorphic_states);

public:
    using StateType = GlobalFaithfulAbstractState;
    using TransitionType = AbstractTransition;

    static std::vector<GlobalFaithfulAbstraction> create(const fs::path& domain_filepath,
                                                         const std::vector<fs::path>& problem_filepaths,
                                                         const FaithfulAbstractionsOptions& options = FaithfulAbstractionsOptions());

    static std::vector<GlobalFaithfulAbstraction>
    create(const std::vector<std::tuple<Problem, std::shared_ptr<PDDLFactories>, std::shared_ptr<IAAG>, std::shared_ptr<SuccessorStateGenerator>>>& memories,
           const FaithfulAbstractionsOptions& options = FaithfulAbstractionsOptions());

    /**
     * Abstraction functionality
     */

    StateIndex get_abstract_state_index(State concrete_state) const;
    StateIndex get_abstract_state_index(StateIndex global_state_index) const;

    /**
     * Extended functionality
     */

    std::vector<double> compute_shortest_distances_from_states(const StateIndexList& states, bool forward = true) const;

    std::vector<std::vector<double>> compute_pairwise_shortest_state_distances(bool forward = true) const;

    /**
     * Getters
     */

    /* Meta data */
    Problem get_problem() const;
    bool get_mark_true_goal_literals() const;
    bool get_use_unit_cost_one() const;
    AbstractionIndex get_index() const;

    /* Memory */
    const std::shared_ptr<PDDLFactories>& get_pddl_factories() const;
    const std::shared_ptr<IAAG>& get_aag() const;
    const std::shared_ptr<SuccessorStateGenerator>& get_ssg() const;
    const FaithfulAbstractionList& get_abstractions() const;

    /* States */
    const GlobalFaithfulAbstractStateList& get_states() const;
    const StateMap<StateIndex>& get_concrete_to_abstract_state() const;
    const std::unordered_map<StateIndex, StateIndex>& get_global_state_index_to_state_index() const;
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
    size_t get_num_isomorphic_states() const;
    size_t get_num_non_isomorphic_states() const;

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

/**
 * Static assertions
 */

static_assert(IsAbstraction<GlobalFaithfulAbstraction>);

/**
 * Pretty printing
 */

extern std::ostream& operator<<(std::ostream& out, const GlobalFaithfulAbstraction& abstraction);

}

#endif
