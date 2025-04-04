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

#include "mimir/search/applicability.hpp"
#include "mimir/search/applicable_action_generators/grounded/event_handlers/default.hpp"
#include "mimir/search/applicable_action_generators/grounded/event_handlers/interface.hpp"
#include "mimir/search/applicable_action_generators/lifted.hpp"
#include "mimir/search/match_tree/match_tree.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/state_repository.hpp"

using namespace mimir::formalism;

namespace mimir::search
{

/**
 * GroundedApplicableActionGenerator
 */

GroundedApplicableActionGeneratorImpl::GroundedApplicableActionGeneratorImpl(Problem problem,
                                                                             match_tree::MatchTree<GroundActionImpl>&& match_tree,
                                                                             EventHandler event_handler) :
    m_problem(std::move(problem)),
    m_match_tree(std::move(match_tree)),
    m_event_handler(std::move(event_handler)),
    m_dense_state()
{
}

std::shared_ptr<GroundedApplicableActionGeneratorImpl> GroundedApplicableActionGeneratorImpl::create(Problem problem,
                                                                                                     match_tree::MatchTree<GroundActionImpl>&& match_tree)
{
    return create(problem, std::move(match_tree), DefaultEventHandlerImpl::create());
}

std::shared_ptr<GroundedApplicableActionGeneratorImpl>
GroundedApplicableActionGeneratorImpl::create(Problem problem, match_tree::MatchTree<GroundActionImpl>&& match_tree, EventHandler event_handler)
{
    return std::shared_ptr<GroundedApplicableActionGeneratorImpl>(
        new GroundedApplicableActionGeneratorImpl(std::move(problem), std::move(match_tree), std::move(event_handler)));
}

mimir::generator<GroundAction> GroundedApplicableActionGeneratorImpl::create_applicable_action_generator(State state)
{
    DenseState::translate(state, m_dense_state);

    return create_applicable_action_generator(m_dense_state);
}

mimir::generator<GroundAction> GroundedApplicableActionGeneratorImpl::create_applicable_action_generator(const DenseState& dense_state)
{
    auto ground_actions = GroundActionList {};
    m_match_tree->generate_applicable_elements_iteratively(dense_state, *m_problem, ground_actions);

    for (const auto& ground_action : ground_actions)
    {
        assert(is_applicable(ground_action, *m_problem, dense_state));
        co_yield ground_action;
    }
}

const Problem& GroundedApplicableActionGeneratorImpl::get_problem() const { return m_problem; }

void GroundedApplicableActionGeneratorImpl::on_finish_search_layer() { m_event_handler->on_finish_search_layer(); }

void GroundedApplicableActionGeneratorImpl::on_end_search() { m_event_handler->on_end_search(); }

}
