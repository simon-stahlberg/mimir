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
#include "mimir/formalism/grounders/action_grounder.hpp"
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

LiftedApplicableActionGenerator::LiftedApplicableActionGenerator(std::shared_ptr<ActionGrounder> action_grounder) :
    LiftedApplicableActionGenerator(std::move(action_grounder), std::make_shared<DefaultLiftedApplicableActionGeneratorEventHandler>())
{
}

LiftedApplicableActionGenerator::LiftedApplicableActionGenerator(std::shared_ptr<ActionGrounder> action_grounder,
                                                                 std::shared_ptr<ILiftedApplicableActionGeneratorEventHandler> event_handler) :
    m_grounder(action_grounder),
    m_event_handler(event_handler),
    m_action_precondition_grounders(),
    m_dense_state(),
    m_fluent_atoms(),
    m_derived_atoms(),
    m_fluent_assignment_set(m_grounder->get_problem()->get_objects().size(), m_grounder->get_problem()->get_domain()->get_predicates<Fluent>()),
    m_derived_assignment_set(m_grounder->get_problem()->get_objects().size(), m_grounder->get_problem()->get_problem_and_domain_derived_predicates())

{
    /* 2. Initialize the condition grounders for each action schema. */
    for (const auto& action : m_grounder->get_problem()->get_domain()->get_actions())
    {
        m_action_precondition_grounders.emplace(action,
                                                SatisficingBindingGenerator(m_grounder->get_literal_grounder(),
                                                                            m_grounder->get_numeric_constraint_grounder(),
                                                                            action->get_conjunctive_condition()));
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

    m_grounder->get_pddl_repositories()->get_ground_atoms_from_indices(dense_fluent_atoms, m_fluent_atoms);
    m_fluent_assignment_set.clear();
    m_fluent_assignment_set.insert_ground_atoms(m_fluent_atoms);

    m_grounder->get_pddl_repositories()->get_ground_atoms_from_indices(dense_derived_atoms, m_derived_atoms);
    m_derived_assignment_set.clear();
    m_derived_assignment_set.insert_ground_atoms(m_derived_atoms);

    /* Generate applicable actions */

    m_event_handler->on_start_generating_applicable_actions();

    for (auto& [action, condition_grounder] : m_action_precondition_grounders)
    {
        // We move this check here to avoid unnecessary creations of mimir::generator.
        if (!nullary_conditions_hold(action->get_conjunctive_condition(), dense_state))
        {
            continue;
        }

        for (auto&& binding : condition_grounder.create_binding_generator(dense_state, m_fluent_assignment_set, m_derived_assignment_set))
        {
            const auto num_ground_actions = m_grounder->get_num_ground_actions();

            const auto ground_action = m_grounder->ground(action, std::move(binding));

            assert(is_applicable(ground_action, m_grounder->get_problem(), dense_state));

            m_event_handler->on_ground_action(ground_action);

            (m_grounder->get_num_ground_actions() > num_ground_actions) ? m_event_handler->on_ground_action_cache_miss(ground_action) :
                                                                          m_event_handler->on_ground_action_cache_hit(ground_action);

            co_yield ground_action;
        }
    }

    m_event_handler->on_end_generating_applicable_actions();
}

Problem LiftedApplicableActionGenerator::get_problem() const { return m_grounder->get_problem(); }

const std::shared_ptr<PDDLRepositories>& LiftedApplicableActionGenerator::get_pddl_repositories() const { return m_grounder->get_pddl_repositories(); }

void LiftedApplicableActionGenerator::on_finish_search_layer() { m_event_handler->on_finish_search_layer(); }

void LiftedApplicableActionGenerator::on_end_search() { m_event_handler->on_end_search(); }

const std::shared_ptr<ActionGrounder>& LiftedApplicableActionGenerator::get_action_grounder() const { return m_grounder; }
}
