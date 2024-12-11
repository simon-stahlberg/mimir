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

#ifndef MIMIR_SEARCH_DELETE_RELAXED_PROBLEM_EXPLORATOR_HPP_
#define MIMIR_SEARCH_DELETE_RELAXED_PROBLEM_EXPLORATOR_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/transformers/delete_relax.hpp"
#include "mimir/search/applicable_action_generators/grounded/event_handlers.hpp"
#include "mimir/search/axiom_evaluators/grounded/event_handlers.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/grounders/grounder.hpp"
#include "mimir/search/state_repository.hpp"

#include <vector>

namespace mimir
{

class DeleteRelaxedProblemExplorator
{
private:
    std::shared_ptr<Grounder> m_grounder;

    DeleteRelaxTransformer m_delete_relax_transformer;
    Problem m_delete_free_problem;
    std::shared_ptr<Grounder> m_delete_free_grounder;
    std::shared_ptr<LiftedApplicableActionGenerator> m_delete_free_applicable_action_generator;
    std::shared_ptr<LiftedAxiomEvaluator> m_delete_free_axiom_evalator;
    StateRepository m_delete_free_state_repository;

public:
    explicit DeleteRelaxedProblemExplorator(std::shared_ptr<Grounder> grounder);

    // Uncopyable
    DeleteRelaxedProblemExplorator(const DeleteRelaxedProblemExplorator& other) = delete;
    DeleteRelaxedProblemExplorator& operator=(const DeleteRelaxedProblemExplorator& other) = delete;
    // Unmovable
    DeleteRelaxedProblemExplorator(DeleteRelaxedProblemExplorator&& other) = delete;
    DeleteRelaxedProblemExplorator& operator=(DeleteRelaxedProblemExplorator&& other) = delete;

    std::shared_ptr<GroundedAxiomEvaluator> create_grounded_axiom_evaluator(
        std::shared_ptr<IGroundedAxiomEvaluatorEventHandler> event_handler = std::make_shared<DefaultGroundedAxiomEvaluatorEventHandler>()) const;

    std::shared_ptr<GroundedApplicableActionGenerator>
    create_grounded_applicable_action_generator(std::shared_ptr<IGroundedApplicableActionGeneratorEventHandler> event_handler =
                                                    std::make_shared<DefaultGroundedApplicableActionGeneratorEventHandler>()) const;

    const std::shared_ptr<Grounder>& get_grounder() const;
};

}  // namespace mimir

#endif
