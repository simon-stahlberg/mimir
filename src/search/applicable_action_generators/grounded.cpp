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
#include "mimir/search/applicable_action_generators/lifted.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/state_repository.hpp"

using namespace mimir::formalism;

namespace mimir::search
{

/**
 * DebugEventHandler
 */

void GroundedApplicableActionGenerator::DebugEventHandler::on_start_ground_action_instantiation_impl() const
{
    std::cout << "[GroundedApplicableActionGenerator] Started instantiating ground actions." << std::endl;
}

void GroundedApplicableActionGenerator::DebugEventHandler::on_finish_ground_action_instantiation_impl(std::chrono::milliseconds total_time) const
{
    std::cout << "[GroundedApplicableActionGenerator] Total time for ground action instantiation: " << total_time.count() << " ms" << std::endl;
}

void GroundedApplicableActionGenerator::DebugEventHandler::on_start_build_action_match_tree_impl() const
{
    std::cout << "[GroundedApplicableActionGenerator] Started building action match tree." << std::endl;
}

void GroundedApplicableActionGenerator::DebugEventHandler::on_finish_build_action_match_tree_impl(const match_tree::MatchTree<GroundActionImpl>& match_tree)
{
    std::cout << match_tree.get_statistics() << "\n"
              << "[GroundedApplicableActionGenerator] Finished building action match tree" << std::endl;
}

void GroundedApplicableActionGenerator::DebugEventHandler::on_finish_search_layer_impl() const
{  //
}

void GroundedApplicableActionGenerator::DebugEventHandler::on_end_search_impl() const {}

/**
 * DefaultEventHandler
 */

void GroundedApplicableActionGenerator::DefaultEventHandler::on_start_ground_action_instantiation_impl() const
{
    std::cout << "[GroundedApplicableActionGenerator] Started instantiating ground actions." << std::endl;
}

void GroundedApplicableActionGenerator::DefaultEventHandler::on_finish_ground_action_instantiation_impl(std::chrono::milliseconds total_time) const
{
    std::cout << "[GroundedApplicableActionGenerator] Total time for ground action instantiation: " << total_time.count() << " ms" << std::endl;
}

void GroundedApplicableActionGenerator::DefaultEventHandler::on_start_build_action_match_tree_impl() const
{
    std::cout << "[GroundedApplicableActionGenerator] Started building action match tree." << std::endl;
}

void GroundedApplicableActionGenerator::DefaultEventHandler::on_finish_build_action_match_tree_impl(const match_tree::MatchTree<GroundActionImpl>& match_tree)
{
    std::cout << match_tree.get_statistics() << "\n"
              << "[GroundedApplicableActionGenerator] Finished building action match tree" << std::endl;
}

void GroundedApplicableActionGenerator::DefaultEventHandler::on_finish_search_layer_impl() const
{  //
}

void GroundedApplicableActionGenerator::DefaultEventHandler::on_end_search_impl() const {}

/**
 * GroundedApplicableActionGenerator
 */

GroundedApplicableActionGenerator::GroundedApplicableActionGenerator(Problem problem, std::unique_ptr<match_tree::MatchTree<GroundActionImpl>>&& match_tree) :
    GroundedApplicableActionGenerator(std::move(problem), std::move(match_tree), std::make_shared<DefaultEventHandler>())
{
}

GroundedApplicableActionGenerator::GroundedApplicableActionGenerator(Problem problem,
                                                                     std::unique_ptr<match_tree::MatchTree<GroundActionImpl>>&& match_tree,
                                                                     std::shared_ptr<IEventHandler> event_handler) :
    m_problem(std::move(problem)),
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
    m_match_tree->generate_applicable_elements_iteratively(dense_state, *m_problem, ground_actions);

    for (const auto& ground_action : ground_actions)
    {
        assert(is_applicable(ground_action, *m_problem, dense_state));
        co_yield ground_action;
    }
}

const Problem& GroundedApplicableActionGenerator::get_problem() const { return m_problem; }

void GroundedApplicableActionGenerator::on_finish_search_layer() { m_event_handler->on_finish_search_layer(); }

void GroundedApplicableActionGenerator::on_end_search() { m_event_handler->on_end_search(); }

}
