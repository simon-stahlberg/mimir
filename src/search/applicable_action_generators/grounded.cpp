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

#include "mimir/formalism/grounders/action_grounder.hpp"
#include "mimir/search/applicability.hpp"
#include "mimir/search/applicable_action_generators/grounded/event_handlers.hpp"
#include "mimir/search/applicable_action_generators/lifted.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/state_repository.hpp"

namespace mimir
{

GroundedApplicableActionGenerator::GroundedApplicableActionGenerator(std::shared_ptr<ActionGrounder> action_grounder,
                                                                     std::unique_ptr<match_tree::MatchTree<GroundActionImpl>>&& match_tree) :
    GroundedApplicableActionGenerator(std::move(action_grounder),
                                      std::move(match_tree),
                                      std::make_shared<DefaultGroundedApplicableActionGeneratorEventHandler>())
{
}

GroundedApplicableActionGenerator::GroundedApplicableActionGenerator(std::shared_ptr<ActionGrounder> action_grounder,
                                                                     std::unique_ptr<match_tree::MatchTree<GroundActionImpl>>&& match_tree,
                                                                     std::shared_ptr<IGroundedApplicableActionGeneratorEventHandler> event_handler) :
    m_grounder(std::move(action_grounder)),
    m_match_tree(std::move(match_tree)),
    m_event_handler(std::move(event_handler)),
    m_dense_state()
{
}

mimir::generator<GroundAction> GroundedApplicableActionGenerator::create_applicable_action_generator(State state)
{
    DenseState::translate(state, m_dense_state);

    return create_applicable_action_generator(m_dense_state);
}

mimir::generator<GroundAction> GroundedApplicableActionGenerator::create_applicable_action_generator(const DenseState& dense_state)
{
    auto ground_actions = GroundActionList {};
    m_match_tree->generate_applicable_elements_iteratively(dense_state, ground_actions);

    const auto requires_numeric_effect_applicability_check = m_dense_state.get_numeric_variables().size() > 0;
    const auto requires_match_tree_imperfection_check = m_match_tree->is_imperfect();

    if (requires_numeric_effect_applicability_check || requires_match_tree_imperfection_check)
    {
        for (const auto& ground_action : ground_actions)
        {
            if (is_applicable(ground_action, m_grounder->get_problem(), dense_state))
            {
                co_yield ground_action;
            }
        }
    }
    else
    {
        for (const auto& ground_action : ground_actions)
        {
            assert(is_applicable(ground_action, m_grounder->get_problem(), dense_state));

            co_yield ground_action;
        }
    }
}

Problem GroundedApplicableActionGenerator::get_problem() const { return m_grounder->get_problem(); }

const std::shared_ptr<PDDLRepositories>& GroundedApplicableActionGenerator::get_pddl_repositories() const { return m_grounder->get_pddl_repositories(); }

void GroundedApplicableActionGenerator::on_finish_search_layer() { m_event_handler->on_finish_search_layer(); }

void GroundedApplicableActionGenerator::on_end_search() { m_event_handler->on_end_search(); }

const std::shared_ptr<ActionGrounder>& GroundedApplicableActionGenerator::get_action_grounder() const { return m_grounder; }

}
