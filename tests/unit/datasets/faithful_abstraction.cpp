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

#include "mimir/datasets/faithful_abstraction.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, DatasetsFaithfulAbstractionCreateTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

    const auto abstraction = FaithfulAbstraction::create(domain_file, problem_file).value();

    EXPECT_EQ(abstraction.get_num_vertices(), 12);
    EXPECT_EQ(abstraction.get_num_edges(), 36);
    EXPECT_EQ(abstraction.get_num_goal_vertices(), 2);
    EXPECT_EQ(abstraction.get_num_deadend_vertices(), 0);
}

TEST(MimirTests, DatasetsFaithfulAbstractionCreateParallelTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file_1 = fs::path(std::string(DATA_DIR) + "gripper/p-1-0.pddl");
    const auto problem_file_2 = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");
    const auto problem_files = std::vector<fs::path> { problem_file_1, problem_file_2 };

    const auto abstractions = FaithfulAbstraction::create(domain_file, problem_files);

    EXPECT_EQ(abstractions.size(), 2);

    // Reduced from 8 to 6 abstract states.
    EXPECT_EQ(abstractions.at(0).get_num_vertices(), 6);
    // Reduced from 28 to 12 abstract states.
    EXPECT_EQ(abstractions.at(1).get_num_vertices(), 12);
}

}
