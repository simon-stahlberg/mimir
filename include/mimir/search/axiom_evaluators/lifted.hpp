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

namespace mimir::search
{

class LiftedAxiomEvaluator : public IAxiomEvaluator
{
private:
    formalism::Problem m_problem;
    LiftedAxiomEvaluatorEventHandler m_event_handler;

    AxiomSatisficingBindingGeneratorList m_condition_grounders;

    /* Memory for reuse */
    formalism::GroundAtomList<formalism::FluentTag> m_fluent_atoms;
    formalism::GroundAtomList<formalism::DerivedTag> m_derived_atoms;
    formalism::GroundFunctionList<formalism::FluentTag> m_fluent_functions;
    formalism::AssignmentSet<formalism::FluentTag> m_fluent_assignment_set;
    formalism::AssignmentSet<formalism::DerivedTag> m_derived_assignment_set;
    formalism::NumericAssignmentSet<formalism::FluentTag> m_numeric_assignment_set;

public:
    explicit LiftedAxiomEvaluator(formalism::Problem problem);

    LiftedAxiomEvaluator(formalism::Problem problem, LiftedAxiomEvaluatorEventHandler event_handler);

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

    const formalism::Problem& get_problem() const override;
    const LiftedAxiomEvaluatorEventHandler& get_event_handler() const;
};

}  // namespace mimir

#endif
