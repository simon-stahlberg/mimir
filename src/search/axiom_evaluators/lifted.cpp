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

#include "mimir/search/axiom_evaluators/lifted.hpp"

#include "mimir/formalism/axiom.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/axiom_evaluators/lifted/event_handlers.hpp"
#include "mimir/search/state.hpp"

namespace mimir
{
LiftedAxiomEvaluator::LiftedAxiomEvaluator(Problem problem, std::shared_ptr<PDDLRepositories> pddl_repositories) :
    LiftedAxiomEvaluator(problem, std::move(pddl_repositories), std::make_shared<DefaultLiftedAxiomEvaluatorEventHandler>())
{
}

LiftedAxiomEvaluator::LiftedAxiomEvaluator(Problem problem,
                                           std::shared_ptr<PDDLRepositories> pddl_repositories,
                                           std::shared_ptr<ILiftedAxiomEvaluatorEventHandler> event_handler) :
    m_grounder(problem, pddl_repositories),
    m_event_handler(std::move(event_handler)),
    m_partitioning(compute_axiom_partitioning(problem->get_problem_and_domain_axioms(), problem->get_problem_and_domain_derived_predicates()))
{
}

void LiftedAxiomEvaluator::generate_and_apply_axioms(StateImpl& unextended_state)
{
    /* 1. Initialize assignment set */

    m_event_handler->on_start_generating_applicable_axioms();

    const auto problem = m_grounder.get_problem();
    auto pddl_repositories = m_grounder.get_pddl_repositories();

    // TODO: In principle, we could reuse the resulting assignment set from the lifted AAG but it is difficult to access here.
    const auto fluent_assignment_set = AssignmentSet<Fluent>(problem,
                                                             problem->get_domain()->get_predicates<Fluent>(),
                                                             pddl_repositories->get_ground_atoms_from_indices<Fluent>(unextended_state.get_atoms<Fluent>()));

    auto derived_assignment_set = AssignmentSet<Derived>(problem,
                                                         problem->get_problem_and_domain_derived_predicates(),
                                                         pddl_repositories->get_ground_atoms_from_indices<Derived>(unextended_state.get_atoms<Derived>()));

    /* 2. Fixed point computation */

    auto applicable_axioms = GroundAxiomList {};

    for (const auto& partition : m_partitioning)
    {
        bool reached_partition_fixed_point;

        // Optimization 1: Track new ground atoms to simplify the clique enumeration graph in the next iteration.
        auto new_ground_atoms = GroundAtomList<Derived> {};

        // TODO: Optimization 4: Inductively compile away axioms with static bodies. (grounded in match tree?)

        // Optimization 2: Track axioms that might trigger in next iteration.
        // Optimization 3: Use precomputed set of axioms that might be applicable initially
        auto relevant_axioms = partition.get_initially_relevant_axioms();

        do
        {
            reached_partition_fixed_point = true;

            /* Compute applicable axioms */

            applicable_axioms.clear();
            for (const auto& axiom : relevant_axioms)
            {
                auto& condition_grounder = m_grounder.get_axiom_precondition_grounders().at(axiom);

                for (auto&& binding : condition_grounder.create_binding_generator(&unextended_state, fluent_assignment_set, derived_assignment_set))
                {
                    const auto num_ground_axioms = m_grounder.get_num_ground_axioms();

                    const auto ground_axiom = m_grounder.ground_axiom(axiom, std::move(binding));

                    assert(ground_axiom->is_applicable(problem, &unextended_state));

                    m_event_handler->on_ground_axiom(ground_axiom);

                    (m_grounder.get_num_ground_axioms() > num_ground_axioms) ? m_event_handler->on_ground_axiom_cache_miss(ground_axiom) :
                                                                               m_event_handler->on_ground_axiom_cache_hit(ground_axiom);

                    applicable_axioms.emplace_back(ground_axiom);
                }
            }

            /* Apply applicable axioms */

            new_ground_atoms.clear();
            relevant_axioms.clear();

            for (const auto& grounded_axiom : applicable_axioms)
            {
                assert(!grounded_axiom->get_derived_effect().is_negated);

                const auto grounded_atom_index = grounded_axiom->get_derived_effect().atom_index;

                if (!unextended_state.get_atoms<Derived>().get(grounded_atom_index))
                {
                    // GENERATED NEW DERIVED ATOM!
                    const auto new_ground_atom = pddl_repositories->get_ground_atom<Derived>(grounded_atom_index);
                    reached_partition_fixed_point = false;

                    // TODO: Optimization 5: Update new ground atoms to speed up successive iterations, i.e.,
                    // only cliques that takes these new atoms into account must be computed.
                    new_ground_atoms.push_back(new_ground_atom);

                    // Update the assignment set
                    derived_assignment_set.insert_ground_atom(new_ground_atom);

                    // Retrieve relevant axioms
                    partition.retrieve_axioms_with_same_body_predicate(new_ground_atom, relevant_axioms);
                }

                unextended_state.get_atoms<Derived>().set(grounded_atom_index);
            }
        } while (!reached_partition_fixed_point);
    }

    m_event_handler->on_end_generating_applicable_axioms();
}

void LiftedAxiomEvaluator::on_finish_search_layer() { m_event_handler->on_finish_search_layer(); }

void LiftedAxiomEvaluator::on_end_search() { m_event_handler->on_end_search(); }

AxiomGrounder& LiftedAxiomEvaluator::get_axiom_grounder() { return m_grounder; }

const AxiomGrounder& LiftedAxiomEvaluator::get_axiom_grounder() const { return m_grounder; }

const std::shared_ptr<ILiftedAxiomEvaluatorEventHandler>& LiftedAxiomEvaluator::get_event_handler() const { return m_event_handler; }

const std::vector<AxiomPartition>& LiftedAxiomEvaluator::get_axiom_partitioning() const { return m_partitioning; }
}
