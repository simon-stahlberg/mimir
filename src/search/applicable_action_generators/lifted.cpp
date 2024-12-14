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
#include "mimir/search/applicable_action_generators/workspaces.hpp"
#include "mimir/search/grounders/action_grounder.hpp"
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

mimir::generator<GroundAction> LiftedApplicableActionGenerator::create_applicable_action_generator(State state, ApplicableActionGeneratorWorkspace* workspace)
{
    /* Initialize lifted workspace if necessary. */
    auto managed_workspace = std::unique_ptr<ApplicableActionGeneratorWorkspace>(nullptr);
    if (!workspace)
    {
        managed_workspace = std::make_unique<ApplicableActionGeneratorWorkspace>();
        workspace = managed_workspace.get();
    }
    auto& lifted_workspace = managed_workspace->get_or_create_lifted_workspace(m_grounder->get_problem());

    auto& fluent_assignment_set =
        lifted_workspace.get_or_create_fluent_assignment_set(m_grounder->get_problem(),
                                                             lifted_workspace.get_or_create_fluent_atoms(state, *m_grounder->get_pddl_repositories()));

    auto& derived_assignment_set =
        lifted_workspace.get_or_create_derived_assignment_set(m_grounder->get_problem(),
                                                              lifted_workspace.get_or_create_derived_atoms(state, *m_grounder->get_pddl_repositories()));

    /* Generate applicable actions */

    m_event_handler->on_start_generating_applicable_actions();

    for (auto& [action, condition_grounder] : m_action_precondition_grounders)
    {
        for (auto&& binding : condition_grounder.create_binding_generator(state, fluent_assignment_set, derived_assignment_set))
        {
            const auto num_ground_actions = m_grounder->get_num_ground_actions();

            const auto ground_action = m_grounder->ground_action(action, std::move(binding));

            assert(ground_action->is_applicable(m_grounder->get_problem(), state));

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
