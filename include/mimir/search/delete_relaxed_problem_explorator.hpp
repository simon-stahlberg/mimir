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
#include "mimir/formalism/translator/delete_relax.hpp"
#include "mimir/search/applicable_action_generators/grounded.hpp"
#include "mimir/search/axiom_evaluators/grounded.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/options.hpp"
#include "mimir/search/state_repository.hpp"

#include <memory>

namespace mimir::search
{

class DeleteRelaxedProblemExplorator
{
private:
    formalism::Problem m_problem;

    /* Delete free info to instantiate the grounded generators. */
    formalism::DeleteRelaxTranslator m_delete_relax_transformer;
    formalism::Problem m_delete_free_problem;
    formalism::ToObjectMap<formalism::Object> m_delete_free_object_to_unrelaxed_object;

public:
    explicit DeleteRelaxedProblemExplorator(formalism::Problem problem);
    DeleteRelaxedProblemExplorator(const DeleteRelaxedProblemExplorator& other) = delete;
    DeleteRelaxedProblemExplorator& operator=(const DeleteRelaxedProblemExplorator& other) = delete;
    DeleteRelaxedProblemExplorator(DeleteRelaxedProblemExplorator&& other) = delete;
    DeleteRelaxedProblemExplorator& operator=(DeleteRelaxedProblemExplorator&& other) = delete;

    GroundedAxiomEvaluator create_grounded_axiom_evaluator(const match_tree::Options& options = match_tree::Options(),
                                                           GroundedAxiomEvaluatorImpl::EventHandler event_handler = nullptr) const;

    GroundedApplicableActionGenerator
    create_grounded_applicable_action_generator(const match_tree::Options& options = match_tree::Options(),
                                                GroundedApplicableActionGeneratorImpl::EventHandler event_handler = nullptr) const;

    const formalism::Problem& get_problem() const;
};

}  // namespace mimir

#endif
