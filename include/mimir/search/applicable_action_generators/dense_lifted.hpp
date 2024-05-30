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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_LIFTED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_LIFTED_HPP_

#include "mimir/formalism/action.hpp"
#include "mimir/search/actions/dense.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted/consistency_graph.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/axiom_evaluators/dense.hpp"
#include "mimir/search/axioms/dense.hpp"
#include "mimir/search/condition_grounders.hpp"
#include "mimir/search/states/dense.hpp"

#include <flatmemory/details/view_const.hpp>
#include <unordered_map>
#include <vector>

namespace mimir
{
using GroundFunctionToValue = std::unordered_map<GroundFunction, double>;

/**
 * Fully specialized implementation class.
 *
 * Implements successor generation using maximum clique enumeration by stahlberg-ecai2023
 * Source: https://mrlab.ai/papers/stahlberg-ecai2023.pdf
 */
template<>
class AAG<LiftedAAGDispatcher<DenseStateTag>> : public IStaticAAG<AAG<LiftedAAGDispatcher<DenseStateTag>>>
{
private:
    Problem m_problem;
    PDDLFactories& m_ref_pddl_factories;
    std::shared_ptr<ILiftedAAGEventHandler> m_event_handler;

    DenseAE m_axiom_evaluator;

    std::unordered_map<Action, ConditionGrounder<DenseState>> m_action_precondition_grounders;
    std::unordered_map<Action, std::vector<consistency_graph::StaticConsistencyGraph>> m_action_universal_effects;

    FlatDenseActionSet m_flat_actions;
    DenseGroundActionList m_actions_by_index;
    DenseGroundActionBuilder m_action_builder;
    std::unordered_map<Action, GroundingTable<DenseGroundAction>> m_action_groundings;

    GroundFunctionToValue m_ground_function_value_costs;

    /// @brief Ground the precondition of an action and return a view onto it.
    [[nodiscard]] DenseGroundAction ground_action_precondition(const Action& action, const ObjectList& binding);

    /* Implement IStaticAAG interface */
    friend class IStaticAAG<AAG<LiftedAAGDispatcher<DenseStateTag>>>;

    void generate_applicable_actions_impl(const DenseState state, DenseGroundActionList& out_applicable_actions);

    void generate_and_apply_axioms_impl(const FlatBitsetBuilder<Fluent>& fluent_state_atoms, FlatBitsetBuilder<Derived>& ref_derived_state_atoms);

    void on_finish_f_layer_impl() const;

    void on_end_search_impl() const;

    [[nodiscard]] Problem get_problem_impl() const;

    [[nodiscard]] PDDLFactories& get_pddl_factories_impl();

    [[nodiscard]] const PDDLFactories& get_pddl_factories_impl() const;

public:
    /// @brief Simplest construction
    AAG(Problem problem, PDDLFactories& ref_pddl_factories);

    /// @brief Complete construction
    AAG(Problem problem, PDDLFactories& ref_pddl_factories, std::shared_ptr<ILiftedAAGEventHandler> event_handler);

    /// @brief Return the axiom partitioning.
    [[nodiscard]] const std::vector<AxiomPartition>& get_axiom_partitioning() const;

    /// @brief Ground an axiom and return a view onto it.
    [[nodiscard]] GroundAxiom ground_axiom(const Axiom& axiom, ObjectList&& binding);

    /// @brief Ground an action and return a view onto it.
    [[nodiscard]] DenseGroundAction ground_action(const Action& action, ObjectList&& binding);

    /// @brief Return all axioms.
    [[nodiscard]] const DenseGroundAxiomList& get_dense_ground_axioms() const;

    /// @brief Return all actions.
    [[nodiscard]] const DenseGroundActionList& get_dense_ground_actions() const;

    /// @brief Return the action with the given id.
    [[nodiscard]] DenseGroundAction get_action(size_t action_id) const;
};

/**
 * Types
 */

using LiftedDenseAAG = AAG<LiftedAAGDispatcher<DenseStateTag>>;

}  // namespace mimir

#endif
