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

#include "mimir/datasets/global_faithful_abstraction.hpp"

#include "mimir/datasets/state_space.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, DatasetsGlobalFaithfulAbstractionCreateGripperTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file_1 = fs::path(std::string(DATA_DIR) + "gripper/p-1-0.pddl");
    const auto problem_file_2 = fs::path(std::string(DATA_DIR) + "gripper/p-1-0.pddl");
    const auto problem_file_3 = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");
    const auto problem_files = std::vector<fs::path> { problem_file_1, problem_file_2, problem_file_3 };

    const auto abstractions = GlobalFaithfulAbstraction::create(domain_file, problem_files);

    // Problem 1 was pruned because it has 0 global non isomorphic states.
    EXPECT_EQ(abstractions.size(), 2);

    // Reduced from 8 to 6 abstract states.
    const auto& abstraction_0 = abstractions.at(0);
    EXPECT_EQ(abstraction_0.get_num_vertices(), 6);
    EXPECT_EQ(abstraction_0.get_num_isomorphic_states(), 0);
    EXPECT_EQ(abstraction_0.get_num_non_isomorphic_states(), 6);
    // Reduced from 28 to 12 abstract states.
    const auto& abstraction_1 = abstractions.at(1);
    EXPECT_EQ(abstraction_1.get_num_vertices(), 12);
    EXPECT_EQ(abstraction_1.get_num_isomorphic_states(), 0);
    EXPECT_EQ(abstraction_1.get_num_non_isomorphic_states(), 12);
}

TEST(MimirTests, DatasetsGlobalFaithfulAbstractionCreateVisitallTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "visitall/domain.pddl");
    const auto problem_file_1 = fs::path(std::string(DATA_DIR) + "visitall/instance1.pddl");
    const auto problem_file_2 = fs::path(std::string(DATA_DIR) + "visitall/instance2.pddl");
    const auto problem_files = std::vector<fs::path> { problem_file_1, problem_file_2 };

    const auto abstractions = GlobalFaithfulAbstraction::create(domain_file, problem_files);

    EXPECT_EQ(abstractions.size(), 2);

    const auto& abstraction_0 = abstractions.at(0);
    EXPECT_EQ(abstraction_0.get_num_vertices(), 5);
    EXPECT_EQ(abstraction_0.get_num_isomorphic_states(), 0);
    EXPECT_EQ(abstraction_0.get_num_non_isomorphic_states(), 5);

    const auto& abstraction_1 = abstractions.at(1);
    EXPECT_EQ(abstraction_1.get_num_vertices(), 5);
    EXPECT_EQ(abstraction_1.get_num_isomorphic_states(), 4);
    EXPECT_EQ(abstraction_1.get_num_non_isomorphic_states(), 1);
}

TEST(MimirTests, DatasetsGlobalFaithfulAbstractionCreateSpannerTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "spanner/domain.pddl");
    const auto problem_file_1 = fs::path(std::string(DATA_DIR) + "spanner/p-1-1-2-1.pddl");
    const auto problem_file_2 = fs::path(std::string(DATA_DIR) + "spanner/p-1-1-3-1.pddl");
    const auto problem_files = std::vector<fs::path> { problem_file_1, problem_file_2 };

    const auto abstractions = GlobalFaithfulAbstraction::create(domain_file, problem_files);

    EXPECT_EQ(abstractions.size(), 2);

    const auto& abstraction_0 = abstractions.at(0);
    EXPECT_EQ(abstraction_0.get_num_vertices(), 8);
    EXPECT_EQ(abstraction_0.get_num_isomorphic_states(), 0);
    EXPECT_EQ(abstraction_0.get_num_non_isomorphic_states(), 8);

    const auto& abstraction_1 = abstractions.at(1);
    EXPECT_EQ(abstraction_1.get_num_vertices(), 10);
    EXPECT_EQ(abstraction_1.get_num_isomorphic_states(), 0);
    EXPECT_EQ(abstraction_1.get_num_non_isomorphic_states(), 10);
}

TEST(MimirTests, DatasetsGlobalFaithfulAbstractionCreateSpannerSccPruningTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "spanner/domain.pddl");
    const auto problem_file_1 = fs::path(std::string(DATA_DIR) + "spanner/p-1-1-2-1.pddl");
    const auto problem_file_2 = fs::path(std::string(DATA_DIR) + "spanner/p-1-1-3-1.pddl");
    const auto problem_files = std::vector<fs::path> { problem_file_1, problem_file_2 };

    auto options = FaithfulAbstractionsOptions();
    options.fa_options.pruning_strategy = ObjectGraphPruningStrategyEnum::StaticScc;
    options.num_threads = 1;
    const auto abstractions = GlobalFaithfulAbstraction::create(domain_file, problem_files, options);

    EXPECT_EQ(abstractions.size(), 2);

    const auto& abstraction_0 = abstractions.at(0);
    EXPECT_EQ(abstraction_0.get_num_vertices(), 8);
    EXPECT_EQ(abstraction_0.get_num_isomorphic_states(), 0);
    EXPECT_EQ(abstraction_0.get_num_non_isomorphic_states(), 8);

    // Now 6 isomorphic states across instances.
    const auto& abstraction_1 = abstractions.at(1);
    EXPECT_EQ(abstraction_1.get_num_vertices(), 10);
    EXPECT_EQ(abstraction_1.get_num_isomorphic_states(), 6);
    EXPECT_EQ(abstraction_1.get_num_non_isomorphic_states(), 4);
}

TEST(MimirTests, DatasetsGlobalFaithfulAbstractionCreateMiconicSccPruningTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "miconic/domain.pddl");
    const auto problem_file_1 = fs::path(std::string(DATA_DIR) + "miconic/test_problem.pddl");
    const auto problem_file_2 = fs::path(std::string(DATA_DIR) + "miconic/test_problem_2.pddl");
    const auto problem_files = std::vector<fs::path> { problem_file_1, problem_file_2 };

    auto options = FaithfulAbstractionsOptions();
    options.fa_options.pruning_strategy = ObjectGraphPruningStrategyEnum::StaticScc;
    options.num_threads = 1;
    const auto abstractions = GlobalFaithfulAbstraction::create(domain_file, problem_files, options);

    EXPECT_EQ(abstractions.size(), 2);

    const auto& abstraction_0 = abstractions.at(0);
    EXPECT_EQ(abstraction_0.get_num_vertices(), 6);
    EXPECT_EQ(abstraction_0.get_num_isomorphic_states(), 0);
    EXPECT_EQ(abstraction_0.get_num_non_isomorphic_states(), 6);

    std::cout << abstraction_0 << std::endl;

    // Now 6 isomorphic states across instances.
    const auto& abstraction_1 = abstractions.at(1);
    EXPECT_EQ(abstraction_1.get_num_vertices(), 12);
    EXPECT_EQ(abstraction_1.get_num_isomorphic_states(), 6);
    EXPECT_EQ(abstraction_1.get_num_non_isomorphic_states(), 6);

    std::cout << abstraction_1 << std::endl;
}

}
