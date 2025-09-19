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

#include "mimir/search/applicable_action_generators/lifted/kpkc.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/applicability.hpp"
#include "mimir/search/applicable_action_generators/lifted/kpkc/event_handlers/default.hpp"
#include "mimir/search/applicable_action_generators/lifted/kpkc/event_handlers/interface.hpp"
#include "mimir/search/satisficing_binding_generators/event_handlers/default.hpp"
#include "mimir/search/satisficing_binding_generators/event_handlers/interface.hpp"
#include "mimir/search/state.hpp"

#include <boost/dynamic_bitset.hpp>
#include <stdexcept>
#include <vector>

using namespace mimir::formalism;

using namespace std::string_literals;

namespace mimir::search
{

/**
 * LiftedApplicableActionGenerator
 */

KPKCLiftedApplicableActionGeneratorImpl::KPKCLiftedApplicableActionGeneratorImpl(Problem problem,
                                                                                 EventHandler event_handler,
                                                                                 satisficing_binding_generator::EventHandler binding_event_handler) :
    m_problem(problem),
    m_event_handler(event_handler ? event_handler : DefaultEventHandlerImpl::create()),
    m_binding_event_handler(binding_event_handler ? binding_event_handler : satisficing_binding_generator::DefaultEventHandlerImpl::create()),
    m_action_grounding_data(),
    m_fluent_atoms(),
    m_derived_atoms(),
    m_fluent_functions(),
    m_fluent_predicate_assignment_sets(m_problem->get_problem_and_domain_objects(), m_problem->get_domain()->get_predicates<formalism::FluentTag>()),
    m_derived_predicate_assignment_sets(m_problem->get_problem_and_domain_objects(), m_problem->get_problem_and_domain_derived_predicates()),
    m_fluent_function_skeleton_assignment_sets(m_problem->get_problem_and_domain_objects(),
                                               m_problem->get_domain()->get_function_skeletons<formalism::FluentTag>())
{
    /* 2. Initialize the condition grounders for each action schema. */
    const auto& actions = problem->get_domain()->get_actions();
    for (size_t i = 0; i < actions.size(); ++i)
    {
        const auto& action = actions[i];
        assert(action->get_index() == i);
        m_action_grounding_data.push_back(ActionSatisficingBindingGenerator(action, m_problem, m_binding_event_handler));
    }
}

KPKCLiftedApplicableActionGenerator
KPKCLiftedApplicableActionGeneratorImpl::create(Problem problem, EventHandler event_handler, satisficing_binding_generator::EventHandler binding_event_handler)
{
    return std::shared_ptr<KPKCLiftedApplicableActionGeneratorImpl>(new KPKCLiftedApplicableActionGeneratorImpl(problem, event_handler, binding_event_handler));
}

mimir::generator<GroundAction> KPKCLiftedApplicableActionGeneratorImpl::create_applicable_action_generator(const State& state)
{
    const auto& dense_fluent_atoms = state.get_atoms<FluentTag>();
    const auto& dense_derived_atoms = state.get_atoms<DerivedTag>();
    const auto& dense_numeric_variables = state.get_numeric_variables();

    const auto& problem = *m_problem;
    const auto& pddl_repositories = problem.get_repositories();

    pddl_repositories.get_ground_atoms_from_indices(dense_fluent_atoms, m_fluent_atoms);
    m_fluent_predicate_assignment_sets.reset();
    m_fluent_predicate_assignment_sets.insert_ground_atoms(m_fluent_atoms);

    pddl_repositories.get_ground_atoms_from_indices(dense_derived_atoms, m_derived_atoms);
    m_derived_predicate_assignment_sets.reset();
    m_derived_predicate_assignment_sets.insert_ground_atoms(m_derived_atoms);

    pddl_repositories.get_ground_functions(dense_numeric_variables.size(), m_fluent_functions);
    m_fluent_function_skeleton_assignment_sets.reset();
    m_fluent_function_skeleton_assignment_sets.insert_ground_function_values(m_fluent_functions, dense_numeric_variables);

    const auto& static_function_skeleton_assignment_sets = problem.get_static_initial_function_skeleton_assignment_sets();

    /* Generate applicable actions */

    m_event_handler->on_start_generating_applicable_actions();

    const auto& ground_action_repository = boost::hana::at_key(problem.get_repositories().get_hana_repositories(), boost::hana::type<GroundActionImpl> {});

    for (auto& condition_grounder : m_action_grounding_data)
    {
        // We move this check here to avoid unnecessary creations of mimir::generator.
        if (!nullary_conditions_hold(condition_grounder.get_conjunctive_condition(), state.get_unpacked_state()))
        {
            continue;
        }

        for (auto&& binding : condition_grounder.create_binding_generator(state,
                                                                          m_fluent_predicate_assignment_sets,
                                                                          m_derived_predicate_assignment_sets,
                                                                          static_function_skeleton_assignment_sets,
                                                                          m_fluent_function_skeleton_assignment_sets))
        {
            const auto num_ground_actions = ground_action_repository.size();

            const auto ground_action = m_problem->ground(condition_grounder.get_action(), std::move(binding));

            assert(is_applicable(ground_action, state));

            m_event_handler->on_ground_action(ground_action);

            (ground_action_repository.size() > num_ground_actions) ? m_event_handler->on_ground_action_cache_miss(ground_action) :
                                                                     m_event_handler->on_ground_action_cache_hit(ground_action);

            co_yield ground_action;
        }
    }

    m_event_handler->on_end_generating_applicable_actions();
}

const Problem& KPKCLiftedApplicableActionGeneratorImpl::get_problem() const { return m_problem; }

void KPKCLiftedApplicableActionGeneratorImpl::on_finish_search_layer()
{
    m_event_handler->on_finish_search_layer();
    m_binding_event_handler->on_finish_search_layer();
}

void KPKCLiftedApplicableActionGeneratorImpl::on_end_search()
{
    m_event_handler->on_end_search();
    m_binding_event_handler->on_end_search();
}
}