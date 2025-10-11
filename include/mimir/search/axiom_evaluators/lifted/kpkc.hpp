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

#ifndef MIMIR_SEARCH_AXIOM_EVALUATOR_LIFTED_KPKC_HPP_
#define MIMIR_SEARCH_AXIOM_EVALUATOR_LIFTED_KPKC_HPP_

#include "mimir/formalism/assignment_set.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/axiom_evaluators/interface.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/satisficing_binding_generators/axiom.hpp"

namespace mimir::search
{

class KPKCLiftedAxiomEvaluatorImpl : public IAxiomEvaluator
{
public:
    using Statistics = axiom_evaluator::lifted::kpkc::Statistics;

    using IEventHandler = axiom_evaluator::lifted::kpkc::IEventHandler;
    using EventHandler = axiom_evaluator::lifted::kpkc::EventHandler;

    using DebugEventHandlerImpl = axiom_evaluator::lifted::kpkc::DebugEventHandlerImpl;
    using DebugEventHandler = axiom_evaluator::lifted::kpkc::DebugEventHandler;

    using DefaultEventHandlerImpl = axiom_evaluator::lifted::kpkc::DefaultEventHandlerImpl;
    using DefaultEventHandler = axiom_evaluator::lifted::kpkc::DefaultEventHandler;

    KPKCLiftedAxiomEvaluatorImpl(formalism::Problem problem,
                                 EventHandler event_handler = nullptr,
                                 satisficing_binding_generator::EventHandler binding_event_handler = nullptr);

    static KPKCLiftedAxiomEvaluator
    create(formalism::Problem problem, EventHandler event_handler = nullptr, satisficing_binding_generator::EventHandler binding_event_handler = nullptr);

    // Uncopyable
    KPKCLiftedAxiomEvaluatorImpl(const KPKCLiftedAxiomEvaluatorImpl& other) = delete;
    KPKCLiftedAxiomEvaluatorImpl& operator=(const KPKCLiftedAxiomEvaluatorImpl& other) = delete;
    // Unmovable
    KPKCLiftedAxiomEvaluatorImpl(KPKCLiftedAxiomEvaluatorImpl&& other) = delete;
    KPKCLiftedAxiomEvaluatorImpl& operator=(KPKCLiftedAxiomEvaluatorImpl&& other) = delete;

    void generate_and_apply_axioms(UnpackedStateImpl& unpacked_state) override;

    void on_finish_search_layer() override;
    void on_end_search() override;

    /**
     * Getters.
     */

    const formalism::Problem& get_problem() const override;
    const EventHandler& get_event_handler() const;

private:
    formalism::Problem m_problem;
    EventHandler m_event_handler;
    satisficing_binding_generator::EventHandler m_binding_event_handler;

    AxiomSatisficingBindingGeneratorList m_condition_grounders;

    formalism::DynamicAssignmentSets m_dynamic_assignment_sets;
};

}  // namespace mimir

#endif
