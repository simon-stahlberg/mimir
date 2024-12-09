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

#include "mimir/common/collections.hpp"
#include "mimir/common/itertools.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/transformers/delete_relax.hpp"
#include "mimir/formalism/utils.hpp"
#include "mimir/search/algorithms/brfs.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/applicable_action_generators/lifted.hpp"
#include "mimir/search/delete_relaxed_problem_explorator.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/state_repository.hpp"

#include <deque>
#include <memory>

namespace mimir
{

GroundedApplicableActionGenerator::GroundedApplicableActionGenerator(ActionGrounder grounder, MatchTree<GroundAction> match_tree) :
    GroundedApplicableActionGenerator(std::move(grounder), std::move(match_tree), std::make_shared<DefaultGroundedApplicableActionGeneratorEventHandler>())
{
}

GroundedApplicableActionGenerator::GroundedApplicableActionGenerator(ActionGrounder grounder,
                                                                     MatchTree<GroundAction> match_tree,
                                                                     std::shared_ptr<IGroundedApplicableActionGeneratorEventHandler> event_handler) :
    m_grounder(std::move(grounder)),
    m_match_tree(std::move(match_tree)),
    m_event_handler(std::move(event_handler))
{
}

std::generator<GroundAction> GroundedApplicableActionGenerator::create_applicable_action_generator(State state)
{
    auto ground_actions = GroundActionList {};
    m_match_tree.get_applicable_elements(state->get_atoms<Fluent>(), state->get_atoms<Derived>(), ground_actions);

    for (const auto& ground_action : ground_actions)
    {
        assert(ground_action->is_applicable(m_grounder.get_problem(), state));

        co_yield ground_action;
    }
}

void GroundedApplicableActionGenerator::on_finish_search_layer() { m_event_handler->on_finish_search_layer(); }

void GroundedApplicableActionGenerator::on_end_search() { m_event_handler->on_end_search(); }

ActionGrounder& GroundedApplicableActionGenerator::get_action_grounder() { return m_grounder; }

const ActionGrounder& GroundedApplicableActionGenerator::get_action_grounder() const { return m_grounder; }

}
