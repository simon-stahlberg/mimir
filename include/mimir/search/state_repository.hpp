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

#ifndef MIMIR_SEARCH_STATE_REPOSITORY_HPP_
#define MIMIR_SEARCH_STATE_REPOSITORY_HPP_

#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/workspaces/state_repository.hpp"  ///< We put this in the header to reduce includes

namespace mimir
{

class StateRepository
{
private:
    std::shared_ptr<IAxiomEvaluator> m_axiom_evaluator;  ///< The axiom evaluator.
    bool m_problem_or_domain_has_axioms;                 ///< flag that indicates whether axiom evaluation must trigger.

    StateImplSet m_states;                   ///< Stores all created extended states.
    AxiomEvaluationSet m_axiom_evaluations;  ///< Stores all axiom evaluations.

    FlatBitset m_reached_fluent_atoms;   ///< Stores all encountered fluent atoms.
    FlatBitset m_reached_derived_atoms;  ///< Stores all encountered derived atoms.

public:
    explicit StateRepository(std::shared_ptr<IAxiomEvaluator> axiom_evaluator);

    StateRepository(const StateRepository& other) = delete;
    StateRepository& operator=(const StateRepository& other) = delete;
    StateRepository(StateRepository&& other) = delete;
    StateRepository& operator=(StateRepository&& other) = delete;

    /// @brief Get or create the extended initial state of the underlying problem.
    /// @return the extended initial state.
    State get_or_create_initial_state(StateRepositoryWorkspace& workspace);

    /// @brief Get or create the extended state for a given set of ground `atoms`.
    /// @param atoms the ground atoms.
    /// @param workspace is the workspace containing preallocated memory.
    /// @return the extended state.
    State get_or_create_state(const GroundAtomList<Fluent>& atoms, StateRepositoryWorkspace& workspace);

    /// @brief Get or create the extended successor state when applying the given ground `action` in the given `state`.
    /// @param state is the state.
    /// @param action is the ground action.
    /// @param workspace is the workspace containing preallocated memory.
    /// @return the extended successor state and the action cost.
    std::pair<State, ContinuousCost> get_or_create_successor_state(State state, GroundAction action, StateRepositoryWorkspace& workspace);

    /// @brief Get or create the extended successor state when applying the given ground `action` in the given state identifed by the `state_fluent_atoms` and
    /// `derived_atoms`. The input parameters `state_fluent_atoms` and `state_derived_atoms` are modified, meaning that side effects have to be taken into
    /// account.
    /// @param state_fluent_atoms are the fluent atoms of the state
    /// @param state_derived_atoms are the derived atoms of the state
    /// @param action is the ground action.
    /// @param workspace is the workspace containing preallocated memory.
    /// @return the extended successor state and the action cost.
    std::pair<State, ContinuousCost> get_or_create_successor_state(DenseState& dense_state, GroundAction action, StateRepositoryWorkspace& workspace);

    /**
     * Getters
     */

    Problem get_problem() const;
    const std::shared_ptr<PDDLRepositories>& get_pddl_repositories() const;

    /// @brief Return the number of created states.
    /// @return the number of created states.
    size_t get_state_count() const;

    /// @brief Return the reached fluent ground atoms.
    /// @return a bitset that stores the fluent ground atom indices.
    const FlatBitset& get_reached_fluent_ground_atoms_bitset() const;

    /// @brief Return the reached derived ground atoms.
    /// @return a bitset that stores the derived ground atom indices.
    const FlatBitset& get_reached_derived_ground_atoms_bitset() const;

    /// @brief Get the underlying axiom evaluator.
    /// @return the axiom evaluator.
    const std::shared_ptr<IAxiomEvaluator>& get_axiom_evaluator() const;

    /// @brief Get the total number of bytes used for storing the unextended state portions.
    /// @return the number of bytes for storing storing the unextended state portions.
    size_t get_estimated_memory_usage_in_bytes_for_unextended_state_portion() const;

    /// @brief Get the total number of bytes used for storing the extended state portions.
    /// @return the number of bytes for storing storing the extended state portions.
    size_t get_estimated_memory_usage_in_bytes_for_extended_state_portion() const;
};

}

#endif