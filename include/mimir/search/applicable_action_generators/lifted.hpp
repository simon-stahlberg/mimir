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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_HPP_

#include "mimir/formalism/action.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/applicable_action_generators/lifted/consistency_graph.hpp"
#include "mimir/search/applicable_action_generators/lifted/event_handlers.hpp"
#include "mimir/search/axiom.hpp"
#include "mimir/search/axiom_evaluators/axiom_evaluator.hpp"
#include "mimir/search/condition_grounders.hpp"
#include "mimir/search/state.hpp"

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
class LiftedApplicableActionGenerator : public IApplicableActionGenerator
{
private:
    Problem m_problem;

    // Memory
    std::shared_ptr<PDDLFactories> m_pddl_factories;
    std::shared_ptr<ILiftedAAGEventHandler> m_event_handler;

    AxiomEvaluator m_axiom_evaluator;

    std::unordered_map<Action, ConditionGrounder<State>> m_action_precondition_grounders;
    std::unordered_map<Action, std::vector<consistency_graph::StaticConsistencyGraph>> m_action_universal_effects;

    FlatActionSet m_flat_actions;
    GroundActionList m_actions_by_index;
    GroundActionBuilder m_action_builder;
    std::unordered_map<Action, GroundingTable<GroundAction>> m_action_groundings;

    GroundFunctionToValue m_ground_function_value_costs;

    /// @brief Ground the precondition of an action and return a view onto it.
    [[nodiscard]] GroundAction ground_action_precondition(Action action, const ObjectList& binding);

public:
    /// @brief Simplest construction
    LiftedApplicableActionGenerator(Problem problem, std::shared_ptr<PDDLFactories> ref_pddl_factories);

    /// @brief Complete construction
    LiftedApplicableActionGenerator(Problem problem, std::shared_ptr<PDDLFactories> ref_pddl_factories, std::shared_ptr<ILiftedAAGEventHandler> event_handler);

    // Uncopyable
    LiftedApplicableActionGenerator(const LiftedApplicableActionGenerator& other) = delete;
    LiftedApplicableActionGenerator& operator=(const LiftedApplicableActionGenerator& other) = delete;
    // Unmovable
    LiftedApplicableActionGenerator(LiftedApplicableActionGenerator&& other) = delete;
    LiftedApplicableActionGenerator& operator=(LiftedApplicableActionGenerator&& other) = delete;

    void generate_applicable_actions(State state, GroundActionList& out_applicable_actions) override;

    void generate_and_apply_axioms(const FlatBitsetBuilder<Fluent>& fluent_state_atoms, FlatBitsetBuilder<Derived>& ref_derived_state_atoms) override;

    void on_finish_f_layer() const override;

    void on_end_search() const override;

    [[nodiscard]] Problem get_problem() const override;

    [[nodiscard]] const std::shared_ptr<PDDLFactories>& get_pddl_factories() const override;

    /// @brief Return the axiom partitioning.
    [[nodiscard]] const std::vector<AxiomPartition>& get_axiom_partitioning() const;

    /// @brief Ground an axiom and return a view onto it.
    [[nodiscard]] GroundAxiom ground_axiom(Axiom axiom, ObjectList&& binding);

    /// @brief Ground an action and return a view onto it.
    [[nodiscard]] GroundAction ground_action(Action action, ObjectList&& binding);

    /// @brief Return all axioms.
    [[nodiscard]] const GroundAxiomList& get_ground_axioms() const;

    /// @brief Return all actions.
    [[nodiscard]] const GroundActionList& get_ground_actions() const;

    /// @brief Return the action with the given id.
    [[nodiscard]] GroundAction get_action(size_t action_id) const override;

    friend std::ostream& operator<<(std::ostream& out, const LiftedApplicableActionGenerator& lifted_aag);
};

std::ostream& operator<<(std::ostream& out, const LiftedApplicableActionGenerator& lifted_aag);

/**
 * Types
 */

using LiftedAAG = LiftedApplicableActionGenerator;

}  // namespace mimir

#endif
