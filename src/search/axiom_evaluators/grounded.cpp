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

#include "mimir/search/axiom_evaluators/grounded.hpp"

#include "mimir/formalism/axiom.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/applicability.hpp"
#include "mimir/search/axiom_evaluators/grounded/event_handlers.hpp"
#include "mimir/search/dense_state.hpp"

namespace mimir
{
GroundedAxiomEvaluator::GroundedAxiomEvaluator(ProblemContext problem_context,
                                               std::vector<std::unique_ptr<match_tree::MatchTree<GroundAxiomImpl>>>&& match_tree_partitioning) :
    GroundedAxiomEvaluator(std::move(problem_context), std::move(match_tree_partitioning), std::make_shared<DefaultGroundedAxiomEvaluatorEventHandler>())
{
}

GroundedAxiomEvaluator::GroundedAxiomEvaluator(ProblemContext problem_context,
                                               std::vector<std::unique_ptr<match_tree::MatchTree<GroundAxiomImpl>>>&& match_tree_partitioning,
                                               std::shared_ptr<IGroundedAxiomEvaluatorEventHandler> event_handler) :
    m_problem_context(std::move(problem_context)),
    m_match_tree_partitioning(std::move(match_tree_partitioning)),
    m_event_handler(std::move(event_handler))
{
}

void GroundedAxiomEvaluator::generate_and_apply_axioms(DenseState& dense_state)
{
    auto& dense_derived_atoms = dense_state.get_atoms<Derived>();

    auto applicable_axioms = GroundAxiomList {};

    for (const auto& match_tree : m_match_tree_partitioning)
    {
        bool reached_partition_fixed_point;

        do
        {
            reached_partition_fixed_point = true;

            /* Compute applicable axioms. */

            applicable_axioms.clear();

            match_tree->generate_applicable_elements_iteratively(dense_state, m_problem_context.get_problem(), applicable_axioms);

            /* Apply applicable axioms */

            for (const auto& grounded_axiom : applicable_axioms)
            {
                assert(is_applicable(grounded_axiom, m_problem_context.get_problem(), dense_state));

                assert(!grounded_axiom->get_derived_effect().is_negated);

                const auto grounded_atom_index = grounded_axiom->get_derived_effect().atom_index;

                if (!dense_derived_atoms.get(grounded_atom_index))
                {
                    // GENERATED NEW DERIVED ATOM!
                    reached_partition_fixed_point = false;
                }

                dense_derived_atoms.set(grounded_atom_index);
            }

        } while (!reached_partition_fixed_point);
    }
}

void GroundedAxiomEvaluator::on_finish_search_layer() { m_event_handler->on_finish_search_layer(); }

void GroundedAxiomEvaluator::on_end_search() { m_event_handler->on_end_search(); }

const ProblemContext& GroundedAxiomEvaluator::get_problem_context() const { return m_problem_context; }

const std::shared_ptr<IGroundedAxiomEvaluatorEventHandler>& GroundedAxiomEvaluator::get_event_handler() const { return m_event_handler; }
}
