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

#ifndef MIMIR_SEARCH_AXIOM_EVALUATOR_GROUNDED_HPP_
#define MIMIR_SEARCH_AXIOM_EVALUATOR_GROUNDED_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/axiom_evaluators/interface.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/match_tree.hpp"

namespace mimir::search
{

class GroundedAxiomEvaluatorImpl : public IAxiomEvaluator
{
public:
    using Statistics = axiom_evaluator::grounded::Statistics;

    using IEventHandler = axiom_evaluator::grounded::IEventHandler;
    using EventHandler = axiom_evaluator::grounded::EventHandler;

    using DebugEventHandlerImpl = axiom_evaluator::grounded::DebugEventHandlerImpl;
    using DebugEventHandler = axiom_evaluator::grounded::DebugEventHandler;

    using DefaultEventHandlerImpl = axiom_evaluator::grounded::DefaultEventHandlerImpl;
    using DefaultEventHandler = axiom_evaluator::grounded::DefaultEventHandler;

    GroundedAxiomEvaluatorImpl(formalism::Problem problem,
                               match_tree::MatchTreeList<formalism::GroundAxiomImpl>&& match_tree_partitioning,
                               EventHandler event_handler);

    static GroundedAxiomEvaluator create(formalism::Problem problem, match_tree::MatchTreeList<formalism::GroundAxiomImpl>&& match_tree_partitioning);

    static GroundedAxiomEvaluator
    create(formalism::Problem problem, match_tree::MatchTreeList<formalism::GroundAxiomImpl>&& match_tree_partitioning, EventHandler event_handler);

    // Uncopyable
    GroundedAxiomEvaluatorImpl(const GroundedAxiomEvaluatorImpl& other) = delete;
    GroundedAxiomEvaluatorImpl& operator=(const GroundedAxiomEvaluatorImpl& other) = delete;
    // Unmovable
    GroundedAxiomEvaluatorImpl(GroundedAxiomEvaluatorImpl&& other) = delete;
    GroundedAxiomEvaluatorImpl& operator=(GroundedAxiomEvaluatorImpl&& other) = delete;

    void generate_and_apply_axioms(DenseState& dense_state) override;

    void on_finish_search_layer() override;
    void on_end_search() override;

    /**
     * Getters
     */

    const formalism::Problem& get_problem() const override;
    const EventHandler& get_event_handler() const;

private:
    formalism::Problem m_problem;
    match_tree::MatchTreeList<formalism::GroundAxiomImpl> m_match_tree_partitioning;
    EventHandler m_event_handler;
};

}

#endif