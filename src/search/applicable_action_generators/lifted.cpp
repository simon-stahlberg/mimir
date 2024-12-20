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

#include "mimir/formalism/repositories.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/applicable_action_generators/lifted/event_handlers.hpp"
#include "mimir/search/grounders/action_grounder.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/workspaces/applicable_action_generator.hpp"
#include "mimir/search/workspaces/lifted_applicable_action_generator.hpp"

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
    m_grounder(std::move(action_grounder)),
    m_event_handler(std::move(event_handler)),
    m_action_precondition_grounders()
{
    /* 2. Initialize the condition grounders for each action schema. */
    for (const auto& action : m_grounder->get_problem()->get_domain()->get_actions())
    {
        m_action_precondition_grounders.emplace(action, SatisficingBindingGenerator(m_grounder->get_literal_grounder(), action->get_precondition()));
    }
}

mimir::generator<GroundAction> LiftedApplicableActionGenerator::create_applicable_action_generator(State state, ApplicableActionGeneratorWorkspace& workspace)
{
    auto& grounded_workspace = workspace.get_or_create_lifted_workspace();
    auto& dense_state = grounded_workspace.get_or_create_dense_state();
    DenseState::translate(state, dense_state);

    return create_applicable_action_generator(dense_state, workspace);
}

mimir::generator<GroundAction> LiftedApplicableActionGenerator::create_applicable_action_generator(const DenseState& dense_state,
                                                                                                   ApplicableActionGeneratorWorkspace& workspace)
{
    auto& dense_fluent_atoms = dense_state.get_atoms<Fluent>();
    auto& dense_derived_atoms = dense_state.get_atoms<Derived>();

    auto& lifted_workspace = workspace.get_or_create_lifted_workspace();

    auto& assignment_set_workspace = lifted_workspace.get_or_create_assignment_set_workspace();
    auto& fluent_atoms = assignment_set_workspace.get_or_create_fluent_atoms(dense_fluent_atoms, *m_grounder->get_pddl_repositories());
    auto& fluent_assignment_set = assignment_set_workspace.get_or_create_fluent_assignment_set(m_grounder->get_problem());
    fluent_assignment_set.clear();
    fluent_assignment_set.insert_ground_atoms(fluent_atoms);

    auto& derived_fluents = assignment_set_workspace.get_or_create_derived_atoms(dense_derived_atoms, *m_grounder->get_pddl_repositories());
    auto& derived_assignment_set = assignment_set_workspace.get_or_create_derived_assignment_set(m_grounder->get_problem());
    derived_assignment_set.clear();
    derived_assignment_set.insert_ground_atoms(derived_fluents);

    /* Generate applicable actions */

    m_event_handler->on_start_generating_applicable_actions();

    for (auto& [action, condition_grounder] : m_action_precondition_grounders)
    {
        // We move this check here to avoid unnecessary creations of mimir::generator.
        if (!nullary_conditions_hold(action->get_precondition(), dense_state))
        {
            continue;
        }

        for (auto&& binding : condition_grounder.create_binding_generator(dense_state,
                                                                          fluent_assignment_set,
                                                                          derived_assignment_set,
                                                                          lifted_workspace.get_or_create_satisficing_binding_generator(action)))
        {
            const auto num_ground_actions = m_grounder->get_num_ground_actions();

            const auto ground_action = m_grounder->ground_action(action, std::move(binding));

            assert(ground_action->is_applicable(m_grounder->get_problem(), dense_state));

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
