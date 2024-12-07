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
#include "mimir/search/axiom_evaluators/axiom_stratification.hpp"
#include "mimir/search/axiom_evaluators/grounded/event_handlers.hpp"
#include "mimir/search/axiom_evaluators/interface.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/grounding/axiom_grounder.hpp"
#include "mimir/search/grounding/match_tree.hpp"

namespace mimir
{

class GroundedAxiomEvaluator : public IAxiomEvaluator
{
private:
    AxiomGrounder m_grounder;
    MatchTree<GroundAxiom> m_match_tree;
    std::shared_ptr<IGroundedAxiomEvaluatorEventHandler> m_event_handler;

    std::vector<AxiomPartition> m_partitioning;

public:
    GroundedAxiomEvaluator(AxiomGrounder grounder, MatchTree<GroundAxiom> match_tree);

    GroundedAxiomEvaluator(AxiomGrounder grounder, MatchTree<GroundAxiom> match_tree, std::shared_ptr<IGroundedAxiomEvaluatorEventHandler> event_handler);

    // Uncopyable
    GroundedAxiomEvaluator(const GroundedAxiomEvaluator& other) = delete;
    GroundedAxiomEvaluator& operator=(const GroundedAxiomEvaluator& other) = delete;
    // Unmovable
    GroundedAxiomEvaluator(GroundedAxiomEvaluator&& other) = delete;
    GroundedAxiomEvaluator& operator=(GroundedAxiomEvaluator&& other) = delete;

    void generate_and_apply_axioms(StateImpl& unextended_state) override;

    void on_finish_search_layer() override;
    void on_end_search() override;

    AxiomGrounder& get_axiom_grounder() override;
    const AxiomGrounder& get_axiom_grounder() const override;

    const std::shared_ptr<IGroundedAxiomEvaluatorEventHandler>& get_event_handler() const;

    const std::vector<AxiomPartition>& get_axiom_partitioning() const;
};

}

#endif