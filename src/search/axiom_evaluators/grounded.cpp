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
#include "mimir/search/axiom_evaluators/grounded/event_handlers.hpp"

namespace mimir
{
GroundedAxiomEvaluator::GroundedAxiomEvaluator(AxiomGrounder grounder, MatchTree<GroundAxiom> match_tree) :
    GroundedAxiomEvaluator(std::move(grounder), std::move(match_tree), std::make_shared<DefaultGroundedAxiomEvaluatorEventHandler>())
{
}

GroundedAxiomEvaluator::GroundedAxiomEvaluator(AxiomGrounder grounder,
                                               MatchTree<GroundAxiom> match_tree,
                                               std::shared_ptr<IGroundedAxiomEvaluatorEventHandler> event_handler) :
    m_grounder(std::move(grounder)),
    m_match_tree(std::move(match_tree)),
    m_event_handler(std::move(event_handler)),
    m_partitioning(compute_axiom_partitioning(m_grounder.get_problem()->get_problem_and_domain_axioms(),
                                              m_grounder.get_problem()->get_problem_and_domain_derived_predicates()))
{
}

void GroundedAxiomEvaluator::generate_and_apply_axioms(StateImpl& unextended_state)
{
    for (const auto& lifted_partition : m_partitioning)
    {
        bool reached_partition_fixed_point;

        do
        {
            reached_partition_fixed_point = true;

            /* Compute applicable axioms. */

            auto applicable_axioms = GroundAxiomList {};

            // TODO: For axioms, the same fluent branch is taken all the time.
            // Exploit this!
            m_match_tree.get_applicable_elements(unextended_state.get_atoms<Fluent>(), unextended_state.get_atoms<Derived>(), applicable_axioms);

            /* Apply applicable axioms */

            for (const auto& grounded_axiom : applicable_axioms)
            {
                if (!lifted_partition.get_axioms().count(m_grounder.get_pddl_repositories()->get_axiom(grounded_axiom->get_axiom_index())))
                {
                    // axiom not part of same partition
                    continue;
                }

                assert(!grounded_axiom->get_derived_effect().is_negated);

                const auto grounded_atom_index = grounded_axiom->get_derived_effect().atom_index;

                if (!unextended_state.get_atoms<Derived>().get(grounded_atom_index))
                {
                    // GENERATED NEW DERIVED ATOM!
                    reached_partition_fixed_point = false;
                }

                unextended_state.get_atoms<Derived>().set(grounded_atom_index);
            }

        } while (!reached_partition_fixed_point);
    }
}

AxiomGrounder& GroundedAxiomEvaluator::get_axiom_grounder() { return m_grounder; }

const AxiomGrounder& GroundedAxiomEvaluator::get_axiom_grounder() const { return m_grounder; }
}
