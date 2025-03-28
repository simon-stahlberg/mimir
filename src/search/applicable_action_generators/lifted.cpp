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

#include "mimir/search/applicable_action_generators/lifted.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/applicability.hpp"
#include "mimir/search/state.hpp"

#include <boost/dynamic_bitset.hpp>
#include <stdexcept>
#include <vector>

using namespace mimir::formalism;

using namespace std::string_literals;

namespace mimir::search
{

/**
 * DebugEventHandler
 */

void LiftedApplicableActionGenerator::DebugEventHandler::on_start_generating_applicable_actions_impl() const {}

void LiftedApplicableActionGenerator::DebugEventHandler::on_ground_action_impl(GroundAction action) const {}

void LiftedApplicableActionGenerator::DebugEventHandler::on_ground_action_cache_hit_impl(GroundAction action) const {}

void LiftedApplicableActionGenerator::DebugEventHandler::on_ground_action_cache_miss_impl(GroundAction action) const {}

void LiftedApplicableActionGenerator::DebugEventHandler::on_end_generating_applicable_actions_impl() const {}

void LiftedApplicableActionGenerator::DebugEventHandler::on_finish_search_layer_impl() const {}

void LiftedApplicableActionGenerator::DebugEventHandler::on_end_search_impl() const { std::cout << get_statistics() << std::endl; }

/**
 * DefaultEventHandler
 */

void LiftedApplicableActionGenerator::DefaultEventHandler::on_start_generating_applicable_actions_impl() const {}

void LiftedApplicableActionGenerator::DefaultEventHandler::on_ground_action_impl(GroundAction action) const {}

void LiftedApplicableActionGenerator::DefaultEventHandler::on_ground_action_cache_hit_impl(GroundAction action) const {}

void LiftedApplicableActionGenerator::DefaultEventHandler::on_ground_action_cache_miss_impl(GroundAction action) const {}

void LiftedApplicableActionGenerator::DefaultEventHandler::on_end_generating_applicable_actions_impl() const {}

void LiftedApplicableActionGenerator::DefaultEventHandler::on_finish_search_layer_impl() const {}

void LiftedApplicableActionGenerator::DefaultEventHandler::on_end_search_impl() const { std::cout << get_statistics() << std::endl; }

/**
 * LiftedApplicableActionGenerator
 */

LiftedApplicableActionGenerator::LiftedApplicableActionGenerator(Problem problem) :
    LiftedApplicableActionGenerator(std::move(problem), std::make_shared<DefaultEventHandler>())
{
}

LiftedApplicableActionGenerator::LiftedApplicableActionGenerator(Problem problem, std::shared_ptr<IEventHandler> event_handler) :
    m_problem(problem),
    m_event_handler(event_handler),
    m_action_grounding_data(),
    m_dense_state(),
    m_fluent_atoms(),
    m_derived_atoms(),
    m_fluent_functions(),
    m_fluent_assignment_set(m_problem->get_problem_and_domain_objects().size(), m_problem->get_domain()->get_predicates<FluentTag>()),
    m_derived_assignment_set(m_problem->get_problem_and_domain_objects().size(), m_problem->get_problem_and_domain_derived_predicates()),
    m_numeric_assignment_set(m_problem->get_problem_and_domain_objects().size(), m_problem->get_domain()->get_function_skeletons<FluentTag>())
{
    /* 2. Initialize the condition grounders for each action schema. */
    const auto& actions = problem->get_domain()->get_actions();
    for (size_t i = 0; i < actions.size(); ++i)
    {
        const auto& action = actions[i];
        assert(action->get_index() == i);
        m_action_grounding_data.push_back(ActionSatisficingBindingGenerator(action, m_problem));
    }
}

mimir::generator<GroundAction> LiftedApplicableActionGenerator::create_applicable_action_generator(State state)
{
    DenseState::translate(state, m_dense_state);

    return create_applicable_action_generator(m_dense_state);
}

mimir::generator<GroundAction> LiftedApplicableActionGenerator::create_applicable_action_generator(const DenseState& dense_state)
{
    auto& dense_fluent_atoms = dense_state.get_atoms<FluentTag>();
    auto& dense_derived_atoms = dense_state.get_atoms<DerivedTag>();
    auto& dense_numeric_variables = dense_state.get_numeric_variables();

    const auto& problem = *m_problem;
    const auto& pddl_repositories = problem.get_repositories();

    pddl_repositories.get_ground_atoms_from_indices(dense_fluent_atoms, m_fluent_atoms);
    m_fluent_assignment_set.reset();
    m_fluent_assignment_set.insert_ground_atoms(m_fluent_atoms);

    pddl_repositories.get_ground_atoms_from_indices(dense_derived_atoms, m_derived_atoms);
    m_derived_assignment_set.reset();
    m_derived_assignment_set.insert_ground_atoms(m_derived_atoms);

    m_numeric_assignment_set.reset();
    pddl_repositories.get_ground_functions(dense_numeric_variables.size(), m_fluent_functions);
    m_numeric_assignment_set.insert_ground_function_values(m_fluent_functions, dense_numeric_variables);

    const auto& static_numeric_assignment_set = problem.get_static_initial_numeric_assignment_set();

    /* Generate applicable actions */

    m_event_handler->on_start_generating_applicable_actions();

    for (auto& condition_grounder : m_action_grounding_data)
    {
        // We move this check here to avoid unnecessary creations of mimir::generator.
        if (!nullary_conditions_hold(condition_grounder.get_conjunctive_condition(), problem, dense_state))
        {
            continue;
        }

        for (auto&& binding : condition_grounder.create_binding_generator(dense_state,
                                                                          m_fluent_assignment_set,
                                                                          m_derived_assignment_set,
                                                                          static_numeric_assignment_set,
                                                                          m_numeric_assignment_set))
        {
            const auto num_ground_actions = problem.get_num_ground_actions();

            const auto ground_action = m_problem->ground(condition_grounder.get_action(), std::move(binding));

            assert(is_applicable(ground_action, problem, dense_state));

            m_event_handler->on_ground_action(ground_action);

            (problem.get_num_ground_actions() > num_ground_actions) ? m_event_handler->on_ground_action_cache_miss(ground_action) :
                                                                      m_event_handler->on_ground_action_cache_hit(ground_action);

            co_yield ground_action;
        }
    }

    m_event_handler->on_end_generating_applicable_actions();
}

const Problem& LiftedApplicableActionGenerator::get_problem() const { return m_problem; }

void LiftedApplicableActionGenerator::on_finish_search_layer() { m_event_handler->on_finish_search_layer(); }

void LiftedApplicableActionGenerator::on_end_search() { m_event_handler->on_end_search(); }
}
