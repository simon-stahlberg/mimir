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

    /// @brief Create all delete-relaxed-reachable ground atoms.
    /// Note: this function is deprecated since it does not deliver the atoms in the unrelaxed task
    /// because relaxed actions miss some...
    /// @tparam P is the predicate type.
    /// @return a vector containing all delete-relaxed reachable ground atoms in the input problem.
    template<formalism::IsFluentOrDerivedTag P>
    formalism::GroundAtomList<P> create_ground_atoms() const;

    /// @brief Create all delete-relaxed-reachable unrelaxed ground actions.
    /// @return a vector containing all delete-relaxed-reachable unrelaxed ground actions.
    formalism::GroundActionList create_ground_actions() const;

    /// @brief Create all delete-relaxed-reachable unrelaxed ground axioms.
    /// @return a vector containing all delete-relaxed-reachable unrelaxed ground axioms.
    formalism::GroundAxiomList create_ground_axioms() const;

    /// @brief Create a grounded axiom evaluator.
    /// @param options the match tree options
    /// @param event_handler the grounded axiom evaluator event handler.
    /// @return a grounded axiom evaluator.
    GroundedAxiomEvaluator create_grounded_axiom_evaluator(const match_tree::Options& options = match_tree::Options(),
                                                           GroundedAxiomEvaluatorImpl::EventHandler event_handler = nullptr) const;

    /// @brief Create a grounded applicable action generator.
    /// @param options the match tree options
    /// @param event_handler the grounded applicable action generator event handler.
    /// @return a grounded applicable action generator.
    GroundedApplicableActionGenerator
    create_grounded_applicable_action_generator(const match_tree::Options& options = match_tree::Options(),
                                                GroundedApplicableActionGeneratorImpl::EventHandler event_handler = nullptr) const;

    /// @brief Get the input problem.
    /// @return the input problem.
    const formalism::Problem& get_problem() const;
};

}  // namespace mimir

#endif
