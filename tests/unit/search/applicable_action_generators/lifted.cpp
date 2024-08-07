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

#include "mimir/formalism/parser.hpp"
#include "mimir/search/algorithms.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/state_repository.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, SearchAAGsDenseLiftedTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "miconic-fulladl/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "miconic-fulladl/test_problem.pddl");
    PDDLParser parser(domain_file, problem_file);
    auto aag_event_handler = std::make_shared<DefaultLiftedApplicableActionGeneratorEventHandler>();
    auto aag = std::make_shared<LiftedApplicableActionGenerator>(parser.get_problem(), parser.get_pddl_factories(), aag_event_handler);
    auto ssg = std::make_shared<StateRepository>(aag);
    auto brfs_event_handler = std::make_shared<DefaultBrFSAlgorithmEventHandler>();
    auto brfs = BrFSAlgorithm(aag, ssg, brfs_event_handler);
    auto ground_actions = GroundActionList {};
    const auto status = brfs.find_solution(ground_actions);
    EXPECT_EQ(status, SearchStatus::SOLVED);

    const auto& aag_statistics = aag_event_handler->get_statistics();

    EXPECT_EQ(aag_statistics.get_num_ground_action_cache_hits_until_f_value().back(), 84);
    EXPECT_EQ(aag_statistics.get_num_ground_action_cache_misses_until_f_value().back(), 10);

    EXPECT_EQ(aag_statistics.get_num_ground_axiom_cache_hits_until_f_value().back(), 377);
    EXPECT_EQ(aag_statistics.get_num_ground_axiom_cache_misses_until_f_value().back(), 16);

    const auto& brfs_statistics = brfs_event_handler->get_statistics();

    EXPECT_EQ(brfs_statistics.get_num_generated_until_f_value().back(), 94);
    EXPECT_EQ(brfs_statistics.get_num_expanded_until_f_value().back(), 36);
}

}
