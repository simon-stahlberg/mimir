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

#ifndef MIMIR_SEARCH_AXIOM_EVALUATOR_LIFTED_HPP_
#define MIMIR_SEARCH_AXIOM_EVALUATOR_LIFTED_HPP_

#include "mimir/formalism/assignment_set.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/axiom_evaluators/interface.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/satisficing_binding_generators/axiom.hpp"

namespace mimir
{

class LiftedAxiomEvaluator : public IAxiomEvaluator
{
private:
    ProblemContext m_problem_context;
    std::shared_ptr<ILiftedAxiomEvaluatorEventHandler> m_event_handler;

    std::unordered_map<Axiom, AxiomSatisficingBindingGenerator> m_condition_grounders;

    /* Memory for reuse */
    GroundAtomList<Fluent> m_fluent_atoms;
    GroundAtomList<Derived> m_derived_atoms;
    GroundFunctionList<Fluent> m_fluent_functions;
    AssignmentSet<Fluent> m_fluent_assignment_set;
    AssignmentSet<Derived> m_derived_assignment_set;
    NumericAssignmentSet<Fluent> m_numeric_assignment_set;

public:
    explicit LiftedAxiomEvaluator(ProblemContext problem_context);

    LiftedAxiomEvaluator(ProblemContext problem_context, std::shared_ptr<ILiftedAxiomEvaluatorEventHandler> event_handler);

    // Uncopyable
    LiftedAxiomEvaluator(const LiftedAxiomEvaluator& other) = delete;
    LiftedAxiomEvaluator& operator=(const LiftedAxiomEvaluator& other) = delete;
    // Unmovable
    LiftedAxiomEvaluator(LiftedAxiomEvaluator&& other) = delete;
    LiftedAxiomEvaluator& operator=(LiftedAxiomEvaluator&& other) = delete;

    void generate_and_apply_axioms(DenseState& dense_state) override;

    void on_finish_search_layer() override;
    void on_end_search() override;

    /**
     * Getters.
     */

    const ProblemContext& get_problem_context() const override;
    const std::shared_ptr<ILiftedAxiomEvaluatorEventHandler>& get_event_handler() const;
};

}  // namespace mimir

#endif
