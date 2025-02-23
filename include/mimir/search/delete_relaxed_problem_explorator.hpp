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
#include "mimir/search/declarations.hpp"
#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/options.hpp"
#include "mimir/search/state_repository.hpp"

#include <memory>

namespace mimir
{

class DeleteRelaxedProblemExplorator
{
private:
    Problem m_problem;

    /* We store all delete-free info separately, allowing us to discard it after instantiating the generator and evaluator. */
    std::shared_ptr<PDDLRepositories> m_delete_free_pddl_repositories;
    DeleteRelaxTransformer m_delete_relax_transformer;
    Problem m_delete_free_problem;
    ProblemContext m_delete_free_problem_context;
    std::shared_ptr<LiftedApplicableActionGenerator> m_delete_free_applicable_action_generator;
    std::shared_ptr<LiftedAxiomEvaluator> m_delete_free_axiom_evalator;
    StateRepository m_delete_free_state_repository;

public:
    explicit DeleteRelaxedProblemExplorator(ProblemContext problem_context);

    // Uncopyable
    DeleteRelaxedProblemExplorator(const DeleteRelaxedProblemExplorator& other) = delete;
    DeleteRelaxedProblemExplorator& operator=(const DeleteRelaxedProblemExplorator& other) = delete;
    // Unmovable
    DeleteRelaxedProblemExplorator(DeleteRelaxedProblemExplorator&& other) = delete;
    DeleteRelaxedProblemExplorator& operator=(DeleteRelaxedProblemExplorator&& other) = delete;

    std::shared_ptr<GroundedAxiomEvaluator> create_grounded_axiom_evaluator(const match_tree::Options& options = match_tree::Options(),
                                                                            std::shared_ptr<IGroundedAxiomEvaluatorEventHandler> event_handler = nullptr) const;

    std::shared_ptr<GroundedApplicableActionGenerator>
    create_grounded_applicable_action_generator(const match_tree::Options& options = match_tree::Options(),
                                                std::shared_ptr<IGroundedApplicableActionGeneratorEventHandler> event_handler = nullptr) const;

    const ProblemContext& get_problem_context() const;
};

}  // namespace mimir

#endif
