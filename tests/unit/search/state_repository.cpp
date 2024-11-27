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

#include "mimir/search/state_repository.hpp"

#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/parser.hpp"
#include "mimir/search/applicable_action_generators/lifted.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, SearchStateRepositoryTest)
{
    // Instantiate lifted version
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl");
    PDDLParser parser(domain_file, problem_file);
    const auto problem = parser.get_problem();
    auto lifted_applicable_action_generator = std::make_shared<LiftedApplicableActionGenerator>(problem, parser.get_pddl_repositories());
    auto state_repository = StateRepository(lifted_applicable_action_generator);
    auto initial_state = state_repository.get_or_create_initial_state();
    auto applicable_actions = GroundActionList {};
    lifted_applicable_action_generator->generate_applicable_actions(initial_state, applicable_actions);

    for (const auto& action : applicable_actions)
    {
        const auto successor_state = state_repository.get_or_create_successor_state(initial_state, action);

        auto applicable_actions2 = GroundActionList {};
        lifted_applicable_action_generator->generate_applicable_actions(successor_state, applicable_actions2);
        for (const auto& action2 : applicable_actions2)
        {
            [[maybe_unused]] const auto successor_state2 = state_repository.get_or_create_successor_state(successor_state, action2);
        }
    }
}

}
