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

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/axiom_evaluators/axiom_stratification.hpp"
#include "mimir/search/axiom_evaluators/interface.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/satisficing_binding_generator/satisficing_binding_generator.hpp"

namespace mimir
{

class LiftedAxiomEvaluator : public IAxiomEvaluator
{
private:
    std::shared_ptr<AxiomGrounder> m_grounder;
    std::shared_ptr<ILiftedAxiomEvaluatorEventHandler> m_event_handler;

    std::unordered_map<Axiom, SatisficingBindingGenerator> m_condition_grounders;

    std::vector<AxiomPartition> m_partitioning;

public:
    explicit LiftedAxiomEvaluator(std::shared_ptr<AxiomGrounder> axiom_grounder);

    LiftedAxiomEvaluator(std::shared_ptr<AxiomGrounder> axiom_grounder, std::shared_ptr<ILiftedAxiomEvaluatorEventHandler> event_handler);

    // Uncopyable
    LiftedAxiomEvaluator(const LiftedAxiomEvaluator& other) = delete;
    LiftedAxiomEvaluator& operator=(const LiftedAxiomEvaluator& other) = delete;
    // Unmovable
    LiftedAxiomEvaluator(LiftedAxiomEvaluator&& other) = delete;
    LiftedAxiomEvaluator& operator=(LiftedAxiomEvaluator&& other) = delete;

    void generate_and_apply_axioms(DenseState& dense_state, AxiomEvaluatorWorkspace& workspace) override;

    void on_finish_search_layer() override;
    void on_end_search() override;

    /**
     * Getters.
     */

    Problem get_problem() const override;
    const std::shared_ptr<PDDLRepositories>& get_pddl_repositories() const override;
    const std::shared_ptr<AxiomGrounder>& get_axiom_grounder() const override;
    const std::shared_ptr<ILiftedAxiomEvaluatorEventHandler>& get_event_handler() const;
    const std::vector<AxiomPartition>& get_axiom_partitioning() const;
};

}  // namespace mimir

#endif
