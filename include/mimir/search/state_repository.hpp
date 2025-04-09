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
#include "mimir/search/dense_state.hpp"
#include "mimir/search/state.hpp"

namespace mimir::search
{

class StateRepositoryImpl
{
private:
    AxiomEvaluator m_axiom_evaluator;     ///< The axiom evaluator.
    bool m_problem_or_domain_has_axioms;  ///< flag that indicates whether axiom evaluation must trigger.

    StateImplSet m_states;  ///< Stores all created extended states.
    // TODO: merge both members below?
    FlatIndexListSet m_fluent_atoms_set;               ///< Stores all created fluent atom lists.
    FlatIndexListSet m_derived_atoms_set;              ///< Stores all created derived atom lists.
    FlatDoubleListSet m_fluent_numeric_variables_set;  ///< Stores all created fluent numeric variable lists.

    FlatBitset m_reached_fluent_atoms;   ///< Stores all encountered fluent atoms.
    FlatBitset m_reached_derived_atoms;  ///< Stores all encountered derived atoms.

    /* Memory for reuse */
    StateImpl m_state_builder;
    DenseState m_dense_state_builder;

    FlatBitset m_applied_positive_effect_atoms;
    FlatBitset m_applied_negative_effect_atoms;

    FlatIndexList m_state_fluent_atoms;
    FlatIndexList m_state_derived_atoms;

public:
    explicit StateRepositoryImpl(AxiomEvaluator axiom_evaluator);

    static StateRepository create(AxiomEvaluator axiom_evaluator);

    StateRepositoryImpl(const StateRepositoryImpl& other) = delete;
    StateRepositoryImpl& operator=(const StateRepositoryImpl& other) = delete;
    StateRepositoryImpl(StateRepositoryImpl&& other) = delete;
    StateRepositoryImpl& operator=(StateRepositoryImpl&& other) = delete;

    /// @brief Get or create the extended initial state of the underlying problem.
    /// @return the extended initial state.
    std::pair<State, ContinuousCost> get_or_create_initial_state();

    /// @brief Get or create the extended state for a given set of ground `atoms`.
    /// @param atoms the ground atoms.
    /// @param workspace is the workspace containing preallocated memory.
    /// @return the extended state.
    std::pair<State, ContinuousCost> get_or_create_state(const formalism::GroundAtomList<formalism::FluentTag>& atoms,
                                                         const FlatDoubleList& fluent_numeric_variables);

    /// @brief Get or create the extended successor state when applying the given ground `action` in the given `state`.
    /// @param state is the state.
    /// @param action is the ground action.
    /// @param state_metric_value is the metric value of the state.
    /// @param workspace is the workspace containing preallocated memory.
    /// @return the extended successor state and its metric value.
    std::pair<State, ContinuousCost> get_or_create_successor_state(State state, formalism::GroundAction action, ContinuousCost state_metric_value);

    /// @brief Get or create the extended successor state when applying the given ground `action` in the given state identifed by the `state_fluent_atoms` and
    /// `derived_atoms`. The input parameters `dense_state` are modified, meaning that side effects have to be taken into account.
    /// @param dense_state is the dense state.
    /// @param action is the ground action.
    /// @param state_metric_value is the metric value of the dense state.
    /// @param workspace is the workspace containing preallocated memory.
    /// @return the extended successor state and its metric value.
    std::pair<State, ContinuousCost> get_or_create_successor_state(State state,  ///< for parallel application of numeric effects
                                                                   DenseState& dense_state,
                                                                   formalism::GroundAction action,
                                                                   ContinuousCost state_metric_value);

    /**
     * Getters
     */

    const formalism::Problem& get_problem() const;

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
    const AxiomEvaluator& get_axiom_evaluator() const;

    /// @brief Get the total number of bytes used for storing the unextended state portions.
    /// @return the number of bytes for storing storing the unextended state portions.
    size_t get_estimated_memory_usage_in_bytes_for_unextended_state_portion() const;

    /// @brief Get the total number of bytes used for storing the extended state portions.
    /// @return the number of bytes for storing storing the extended state portions.
    size_t get_estimated_memory_usage_in_bytes_for_extended_state_portion() const;
};

/**
 * Utils
 */

extern ContinuousCost compute_state_metric_value(State state, const formalism::ProblemImpl& problem);

}

#endif