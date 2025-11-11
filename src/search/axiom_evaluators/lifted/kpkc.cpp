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

#include "mimir/search/axiom_evaluators/lifted/kpkc.hpp"

#include "mimir/formalism/conjunctive_condition.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/formatter.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/applicability.hpp"
#include "mimir/search/assignment_set_utils.hpp"
#include "mimir/search/axiom_evaluators/lifted/kpkc/event_handlers/default.hpp"
#include "mimir/search/axiom_evaluators/lifted/kpkc/event_handlers/interface.hpp"
#include "mimir/search/satisficing_binding_generators/event_handlers/default.hpp"
#include "mimir/search/state_unpacked.hpp"

using namespace mimir::formalism;

namespace mimir::search
{
/**
 * LiftedAxiomEvaluator
 */

KPKCLiftedAxiomEvaluatorImpl::KPKCLiftedAxiomEvaluatorImpl(Problem problem,
                                                           EventHandler event_handler,
                                                           satisficing_binding_generator::EventHandler binding_event_handler) :
    m_problem(problem),
    m_event_handler(event_handler ? event_handler : DefaultEventHandlerImpl::create()),
    m_binding_event_handler(binding_event_handler ? binding_event_handler : satisficing_binding_generator::DefaultEventHandlerImpl::create()),
    m_condition_grounders(),
    m_dynamic_assignment_sets(*m_problem)
{
    /* 3. Initialize condition grounders */
    const auto& axioms = m_problem->get_problem_and_domain_axioms();
    for (size_t i = 0; i < axioms.size(); ++i)
    {
        const auto& axiom = axioms[i];
        assert(axiom->get_index() == i);
        m_condition_grounders.emplace_back(AxiomSatisficingBindingGenerator(axiom, m_problem, m_binding_event_handler));
    }
}

KPKCLiftedAxiomEvaluator
KPKCLiftedAxiomEvaluatorImpl::create(Problem problem, EventHandler event_handler, satisficing_binding_generator::EventHandler binding_event_handler)
{
    return std::shared_ptr<KPKCLiftedAxiomEvaluatorImpl>(new KPKCLiftedAxiomEvaluatorImpl(problem, event_handler, binding_event_handler));
}

void KPKCLiftedAxiomEvaluatorImpl::generate_and_apply_axioms(UnpackedStateImpl& unpacked_state)
{
    initialize(unpacked_state, m_dynamic_assignment_sets);

    /* 2. Fixed point computation */

    const auto& ground_axiom_repository =
        boost::hana::at_key(unpacked_state.get_problem().get_repositories().get_hana_repositories(), boost::hana::type<GroundAxiomImpl> {});

    auto applicable_axioms = GroundAxiomList {};

    for (const auto& partition : unpacked_state.get_problem().get_problem_and_domain_axiom_partitioning())
    {
        bool reached_partition_fixed_point;

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
                // We move this check here to avoid unnecessary creations of mimir::generator.
                if (!nullary_conditions_hold(axiom->get_conjunctive_condition(), unpacked_state))
                {
                    continue;
                }

                auto& condition_grounder = m_condition_grounders.at(axiom->get_index());

                auto vertex_mask = std::optional<boost::dynamic_bitset<>> { std::nullopt };

                for (auto&& binding : condition_grounder.create_binding_generator(unpacked_state, m_dynamic_assignment_sets, vertex_mask))
                {
                    const auto num_ground_axioms = ground_axiom_repository.size();

                    const auto ground_axiom = m_problem->ground(axiom, std::move(binding));

                    assert(is_applicable(ground_axiom, unpacked_state));

                    m_event_handler->on_ground_axiom(ground_axiom);

                    (ground_axiom_repository.size() > num_ground_axioms) ? m_event_handler->on_ground_axiom_cache_miss(ground_axiom) :
                                                                           m_event_handler->on_ground_axiom_cache_hit(ground_axiom);

                    applicable_axioms.emplace_back(ground_axiom);
                }
            }

            /* Apply applicable axioms */

            relevant_axioms.clear();

            for (const auto& grounded_axiom : applicable_axioms)
            {
                assert(grounded_axiom->get_literal()->get_polarity());

                const auto grounded_atom_index = grounded_axiom->get_literal()->get_atom()->get_index();

                if (!unpacked_state.get_atoms<DerivedTag>().get(grounded_atom_index))
                {
                    // GENERATED NEW DERIVED ATOM!
                    const auto new_ground_atom = m_problem->get_repositories().get_ground_atom<DerivedTag>(grounded_atom_index);
                    reached_partition_fixed_point = false;

                    // Update the assignment set
                    m_dynamic_assignment_sets.derived_predicate_assignment_sets.insert_ground_atom(new_ground_atom);
                    // Update the state
                    unpacked_state.get_atoms<DerivedTag>().set(grounded_atom_index);

                    // Retrieve relevant axioms
                    partition.retrieve_axioms_with_same_body_predicate(new_ground_atom, relevant_axioms);
                }
            }
        } while (!reached_partition_fixed_point);
    }

    m_event_handler->on_end_generating_applicable_axioms();
}

void KPKCLiftedAxiomEvaluatorImpl::on_finish_search_layer()
{
    m_event_handler->on_finish_search_layer();
    m_binding_event_handler->on_finish_search_layer();
}

void KPKCLiftedAxiomEvaluatorImpl::on_end_search()
{
    m_event_handler->on_end_search();
    m_binding_event_handler->on_end_search();
}

const Problem& KPKCLiftedAxiomEvaluatorImpl::get_problem() const { return m_problem; }

const KPKCLiftedAxiomEvaluatorImpl::EventHandler& KPKCLiftedAxiomEvaluatorImpl::get_event_handler() const { return m_event_handler; }
}