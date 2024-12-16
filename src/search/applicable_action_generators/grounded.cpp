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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "mimir/search/applicable_action_generators/grounded.hpp"

#include "mimir/search/applicable_action_generators/grounded/event_handlers.hpp"
#include "mimir/search/applicable_action_generators/lifted.hpp"
#include "mimir/search/grounders/action_grounder.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/state_repository.hpp"
#include "mimir/search/workspaces/applicable_action_generator.hpp"
#include "mimir/search/workspaces/grounded_applicable_action_generator.hpp"

namespace mimir
{

GroundedApplicableActionGenerator::GroundedApplicableActionGenerator(std::shared_ptr<ActionGrounder> action_grounder, MatchTree<GroundAction> match_tree) :
    GroundedApplicableActionGenerator(std::move(action_grounder),
                                      std::move(match_tree),
                                      std::make_shared<DefaultGroundedApplicableActionGeneratorEventHandler>())
{
}

GroundedApplicableActionGenerator::GroundedApplicableActionGenerator(std::shared_ptr<ActionGrounder> action_grounder,
                                                                     MatchTree<GroundAction> match_tree,
                                                                     std::shared_ptr<IGroundedApplicableActionGeneratorEventHandler> event_handler) :
    m_grounder(std::move(action_grounder)),
    m_match_tree(std::move(match_tree)),
    m_event_handler(std::move(event_handler))
{
}

mimir::generator<GroundAction> GroundedApplicableActionGenerator::create_applicable_action_generator(State state, ApplicableActionGeneratorWorkspace& workspace)
{
    auto& grounded_workspace = workspace.get_or_create_grounded_workspace();
    auto& [fluent_bitset, derived_bitset] = grounded_workspace.get_or_create_bitsets();
    fluent_bitset.unset_all();
    derived_bitset.unset_all();
    for (const auto& fluent_atom_index : state->get_atoms<Fluent>())
    {
        fluent_bitset.set(fluent_atom_index);
    }
    for (const auto& derived_atom_index : state->get_atoms<Derived>())
    {
        derived_bitset.set(derived_atom_index);
    }

    auto ground_actions = GroundActionList {};
    m_match_tree.get_applicable_elements(fluent_bitset, derived_bitset, ground_actions);

    for (const auto& ground_action : ground_actions)
    {
        assert(ground_action->is_applicable(m_grounder->get_problem(), state));

        co_yield ground_action;
    }
}

Problem GroundedApplicableActionGenerator::get_problem() const { return m_grounder->get_problem(); }

const std::shared_ptr<PDDLRepositories>& GroundedApplicableActionGenerator::get_pddl_repositories() const { return m_grounder->get_pddl_repositories(); }

void GroundedApplicableActionGenerator::on_finish_search_layer() { m_event_handler->on_finish_search_layer(); }

void GroundedApplicableActionGenerator::on_end_search() { m_event_handler->on_end_search(); }

const std::shared_ptr<ActionGrounder>& GroundedApplicableActionGenerator::get_action_grounder() const { return m_grounder; }

}
