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

#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/applicability.hpp"
#include "mimir/search/applicable_action_generators/lifted/event_handlers.hpp"
#include "mimir/search/state.hpp"

#include <boost/dynamic_bitset.hpp>
#include <stdexcept>
#include <vector>

using namespace std::string_literals;

namespace mimir
{

LiftedApplicableActionGenerator::LiftedApplicableActionGenerator(Problem problem, std::shared_ptr<PDDLRepositories> pddl_repositories) :
    LiftedApplicableActionGenerator(problem, std::move(pddl_repositories), std::make_shared<DefaultLiftedApplicableActionGeneratorEventHandler>())
{
}

LiftedApplicableActionGenerator::LiftedApplicableActionGenerator(Problem problem,
                                                                 std::shared_ptr<PDDLRepositories> pddl_repositories,
                                                                 std::shared_ptr<ILiftedApplicableActionGeneratorEventHandler> event_handler) :
    m_problem(problem),
    m_pddl_repositories(pddl_repositories),
    m_event_handler(event_handler),
    m_action_grounding_data(),
    m_dense_state(),
    m_fluent_atoms(),
    m_derived_atoms(),
    m_fluent_functions(),
    m_fluent_assignment_set(m_problem->get_objects().size(), m_problem->get_domain()->get_predicates<Fluent>()),
    m_derived_assignment_set(m_problem->get_objects().size(), m_problem->get_problem_and_domain_derived_predicates()),
    m_numeric_assignment_set(m_problem->get_objects().size(), m_problem->get_domain()->get_functions<Fluent>())
{
    /* 2. Initialize the condition grounders for each action schema. */
    for (const auto& action : m_problem->get_domain()->get_actions())
    {
        auto cond_effect_candidate_objects = std::vector<std::vector<IndexList>> {};
        cond_effect_candidate_objects.reserve(action->get_conditional_effects().size());

        for (const auto& conditional_effect : action->get_conditional_effects())
        {
            auto [vertices_, vertices_by_parameter_index_, objects_by_parameter_index_] =
                consistency_graph::StaticConsistencyGraph::compute_vertices(problem,
                                                                            action->get_arity(),
                                                                            action->get_arity() + conditional_effect->get_arity(),
                                                                            conditional_effect->get_conjunctive_condition()->get_literals<Static>());

            cond_effect_candidate_objects.push_back(std::move(objects_by_parameter_index_));
        }

        m_action_grounding_data.emplace(
            action,
            std::make_pair(ActionSatisficingBindingGenerator(action, m_problem, m_pddl_repositories), std::move(cond_effect_candidate_objects)));
    }
}

mimir::generator<GroundAction> LiftedApplicableActionGenerator::create_applicable_action_generator(State state)
{
    DenseState::translate(state, m_dense_state);

    return create_applicable_action_generator(m_dense_state);
}

mimir::generator<GroundAction> LiftedApplicableActionGenerator::create_applicable_action_generator(const DenseState& dense_state)
{
    auto& dense_fluent_atoms = dense_state.get_atoms<Fluent>();
    auto& dense_derived_atoms = dense_state.get_atoms<Derived>();
    auto& dense_numeric_variables = dense_state.get_numeric_variables();

    m_pddl_repositories->get_ground_atoms_from_indices(dense_fluent_atoms, m_fluent_atoms);
    m_fluent_assignment_set.reset();
    m_fluent_assignment_set.insert_ground_atoms(m_fluent_atoms);

    m_pddl_repositories->get_ground_atoms_from_indices(dense_derived_atoms, m_derived_atoms);
    m_derived_assignment_set.reset();
    m_derived_assignment_set.insert_ground_atoms(m_derived_atoms);

    m_numeric_assignment_set.reset();
    m_pddl_repositories->get_ground_functions(dense_numeric_variables.size(), m_fluent_functions);
    m_numeric_assignment_set.insert_ground_function_values(m_fluent_functions, dense_numeric_variables);

    const auto& static_numeric_assignment_set = m_problem->get_static_initial_numeric_assignment_set();

    /* Generate applicable actions */

    m_event_handler->on_start_generating_applicable_actions();

    for (auto& [action, grounding_data] : m_action_grounding_data)
    {
        auto& [condition_grounder, conditional_effects_candidate_objects] = grounding_data;

        // We move this check here to avoid unnecessary creations of mimir::generator.
        if (!nullary_conditions_hold(action->get_conjunctive_condition(), dense_state))
        {
            continue;
        }

        for (auto&& binding : condition_grounder.create_binding_generator(dense_state,
                                                                          m_fluent_assignment_set,
                                                                          m_derived_assignment_set,
                                                                          static_numeric_assignment_set,
                                                                          m_numeric_assignment_set))
        {
            const auto num_ground_actions = m_pddl_repositories->get_num_ground_actions();

            const auto ground_action = m_pddl_repositories->ground(action, m_problem, std::move(binding), conditional_effects_candidate_objects);

            assert(is_applicable(ground_action, m_problem, dense_state));

            m_event_handler->on_ground_action(ground_action);

            (m_pddl_repositories->get_num_ground_actions() > num_ground_actions) ? m_event_handler->on_ground_action_cache_miss(ground_action) :
                                                                                   m_event_handler->on_ground_action_cache_hit(ground_action);

            co_yield ground_action;
        }
    }

    m_event_handler->on_end_generating_applicable_actions();
}

Problem LiftedApplicableActionGenerator::get_problem() const { return m_problem; }

const std::shared_ptr<PDDLRepositories>& LiftedApplicableActionGenerator::get_pddl_repositories() const { return m_pddl_repositories; }

void LiftedApplicableActionGenerator::on_finish_search_layer() { m_event_handler->on_finish_search_layer(); }

void LiftedApplicableActionGenerator::on_end_search() { m_event_handler->on_end_search(); }
}
