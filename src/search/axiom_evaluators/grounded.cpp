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
#include "mimir/search/axiom_evaluators/grounded/event_handlers/default.hpp"
#include "mimir/search/axiom_evaluators/grounded/event_handlers/interface.hpp"
#include "mimir/search/dense_state.hpp"

using namespace mimir::formalism;

namespace mimir::search
{

/**
 * GroundedAxiomEvaluator
 */

GroundedAxiomEvaluatorImpl::GroundedAxiomEvaluatorImpl(Problem problem,
                                                       match_tree::MatchTreeList<GroundAxiomImpl>&& match_tree_partitioning,
                                                       EventHandler event_handler) :
    m_problem(std::move(problem)),
    m_match_tree_partitioning(std::move(match_tree_partitioning)),
    m_event_handler(std::move(event_handler))
{
}

GroundedAxiomEvaluator GroundedAxiomEvaluatorImpl::create(Problem problem, match_tree::MatchTreeList<GroundAxiomImpl>&& match_tree_partitioning)
{
    return create(std::move(problem), std::move(match_tree_partitioning), DefaultEventHandlerImpl::create());
}

GroundedAxiomEvaluator
GroundedAxiomEvaluatorImpl::create(Problem problem, match_tree::MatchTreeList<GroundAxiomImpl>&& match_tree_partitioning, EventHandler event_handler)
{
    return std::shared_ptr<GroundedAxiomEvaluatorImpl>(
        new GroundedAxiomEvaluatorImpl(std::move(problem), std::move(match_tree_partitioning), std::move(event_handler)));
}

void GroundedAxiomEvaluatorImpl::generate_and_apply_axioms(DenseState& dense_state)
{
    auto& dense_derived_atoms = dense_state.get_atoms<DerivedTag>();

    auto applicable_axioms = GroundAxiomList {};

    for (const auto& match_tree : m_match_tree_partitioning)
    {
        bool reached_partition_fixed_point;

        do
        {
            reached_partition_fixed_point = true;

            /* Compute applicable axioms. */

            applicable_axioms.clear();

            match_tree->generate_applicable_elements_iteratively(dense_state, *m_problem, applicable_axioms);

            /* Apply applicable axioms */

            for (const auto& grounded_axiom : applicable_axioms)
            {
                assert(is_applicable(grounded_axiom, *m_problem, dense_state));

                assert(grounded_axiom->get_literal()->get_polarity());

                const auto grounded_atom_index = grounded_axiom->get_literal()->get_atom()->get_index();

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

void GroundedAxiomEvaluatorImpl::on_finish_search_layer() { m_event_handler->on_finish_search_layer(); }

void GroundedAxiomEvaluatorImpl::on_end_search() { m_event_handler->on_end_search(); }

const Problem& GroundedAxiomEvaluatorImpl::get_problem() const { return m_problem; }

const GroundedAxiomEvaluatorImpl::EventHandler& GroundedAxiomEvaluatorImpl::get_event_handler() const { return m_event_handler; }
}
