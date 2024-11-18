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

#include "mimir/search/algorithms/astar.hpp"

#include "mimir/formalism/parser.hpp"
#include "mimir/search/algorithms/astar/event_handlers.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/applicable_action_generators/grounded/event_handlers.hpp"
#include "mimir/search/applicable_action_generators/lifted/event_handlers.hpp"
#include "mimir/search/heuristics.hpp"
#include "mimir/search/plan.hpp"
#include "mimir/search/planners/single.hpp"
#include "mimir/search/state_repository.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

/**
 * Gripper
 */

TEST(MimirTests, SearchAlgorithmsAStarGroundedHStarGripperTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl");
    auto parser = PDDLParser(domain_file, problem_file);
    auto applicable_action_generator_event_handler = std::make_shared<DefaultGroundedApplicableActionGeneratorEventHandler>();
    auto applicable_action_generator =
        std::make_shared<GroundedApplicableActionGenerator>(parser.get_problem(), parser.get_pddl_repositories(), applicable_action_generator_event_handler);
    auto state_repository = std::make_shared<StateRepository>(applicable_action_generator);
    auto astar_event_handler = std::make_shared<DefaultAStarAlgorithmEventHandler>();
    auto hstar = std::make_shared<HStarHeuristic>(parser.get_problem(), parser.get_pddl_repositories(), applicable_action_generator, state_repository);
    auto astar = std::make_shared<AStarAlgorithm>(applicable_action_generator, state_repository, hstar, astar_event_handler);
    auto planner = SinglePlanner(astar);
    auto [search_status, plan] = planner.find_solution();

    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 3);

    const auto& astar_statistics = astar_event_handler->get_statistics();

    // Zero because AStar never enters a new f-layer.
    EXPECT_EQ(astar_statistics.get_num_generated_until_f_value().back(), 0);
    EXPECT_EQ(astar_statistics.get_num_expanded_until_f_value().back(), 0);
}

TEST(MimirTests, SearchAlgorithmsAStarLiftedHStarGripperTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl");
    auto parser = PDDLParser(domain_file, problem_file);
    auto applicable_action_generator_event_handler = std::make_shared<DefaultLiftedApplicableActionGeneratorEventHandler>();
    auto applicable_action_generator =
        std::make_shared<LiftedApplicableActionGenerator>(parser.get_problem(), parser.get_pddl_repositories(), applicable_action_generator_event_handler);
    auto state_repository = std::make_shared<StateRepository>(applicable_action_generator);
    auto astar_event_handler = std::make_shared<DefaultAStarAlgorithmEventHandler>();
    auto hstar = std::make_shared<HStarHeuristic>(parser.get_problem(), parser.get_pddl_repositories(), applicable_action_generator, state_repository);
    auto astar = std::make_shared<AStarAlgorithm>(applicable_action_generator, state_repository, hstar, astar_event_handler);
    auto planner = SinglePlanner(astar);

    auto [search_status, plan] = planner.find_solution();

    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 3);

    const auto& astar_statistics = astar_event_handler->get_statistics();

    // Zero because AStar never enters a new f-layer.
    EXPECT_EQ(astar_statistics.get_num_generated_until_f_value().back(), 0);
    EXPECT_EQ(astar_statistics.get_num_expanded_until_f_value().back(), 0);
}

TEST(MimirTests, SearchAlgorithmsAStarGroundedBlindGripperTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl");
    auto parser = PDDLParser(domain_file, problem_file);
    auto applicable_action_generator_event_handler = std::make_shared<DefaultGroundedApplicableActionGeneratorEventHandler>();
    auto applicable_action_generator =
        std::make_shared<GroundedApplicableActionGenerator>(parser.get_problem(), parser.get_pddl_repositories(), applicable_action_generator_event_handler);
    auto state_repository = std::make_shared<StateRepository>(applicable_action_generator);
    auto astar_event_handler = std::make_shared<DefaultAStarAlgorithmEventHandler>();
    auto blind = std::make_shared<BlindHeuristic>();
    auto astar = std::make_shared<AStarAlgorithm>(applicable_action_generator, state_repository, blind, astar_event_handler);
    auto planner = SinglePlanner(astar);
    auto [search_status, plan] = planner.find_solution();

    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 3);

    const auto& astar_statistics = astar_event_handler->get_statistics();

    // Identical to the BrFs result because domain has unit cost 1.
    EXPECT_EQ(astar_statistics.get_num_generated_until_f_value().back(), 44);
    EXPECT_EQ(astar_statistics.get_num_expanded_until_f_value().back(), 12);
}

TEST(MimirTests, SearchAlgorithmsAStarLiftedBlindGripperTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl");
    auto parser = PDDLParser(domain_file, problem_file);
    auto applicable_action_generator_event_handler = std::make_shared<DefaultLiftedApplicableActionGeneratorEventHandler>();
    auto applicable_action_generator =
        std::make_shared<LiftedApplicableActionGenerator>(parser.get_problem(), parser.get_pddl_repositories(), applicable_action_generator_event_handler);
    auto state_repository = std::make_shared<StateRepository>(applicable_action_generator);
    auto astar_event_handler = std::make_shared<DefaultAStarAlgorithmEventHandler>();
    auto blind = std::make_shared<BlindHeuristic>();
    auto astar = std::make_shared<AStarAlgorithm>(applicable_action_generator, state_repository, blind, astar_event_handler);
    auto planner = SinglePlanner(astar);

    auto [search_status, plan] = planner.find_solution();

    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 3);

    const auto& astar_statistics = astar_event_handler->get_statistics();

    // Identical to the BrFs result because domain has unit cost 1.
    EXPECT_EQ(astar_statistics.get_num_generated_until_f_value().back(), 44);
    EXPECT_EQ(astar_statistics.get_num_expanded_until_f_value().back(), 12);
}

}
