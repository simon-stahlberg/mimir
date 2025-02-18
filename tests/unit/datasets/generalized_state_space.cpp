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

#include "mimir/datasets/generalized_state_space.hpp"

#include "mimir/search/search_context.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, DatasetsGeneralizedStateSpaceConstructorTest)
{
    /* Test two spanner problems with two locations and a single spanner each. */
    const auto domain_file = fs::path(std::string(DATA_DIR) + "spanner/domain.pddl");
    // The spanner is at location 1.
    const auto problem1_file = fs::path(std::string(DATA_DIR) + "spanner/p-1-1-2-1.pddl");
    // The spanner is at location 2.
    const auto problem2_file = fs::path(std::string(DATA_DIR) + "spanner/p-1-1-2-1(2).pddl");

    auto search_contexts =
        SearchContextList { SearchContext(ProblemContext(domain_file, problem1_file)), SearchContext(ProblemContext(domain_file, problem2_file)) };

    {
        /* Without symmetry reduction */
        auto options = GeneralizedStateSpace::Options();
        options.problem_options.symmetry_pruning = false;
        const auto problem_class_state_space = GeneralizedStateSpace(search_contexts, options);
        const auto& class_state_space = problem_class_state_space.get_class_state_space();
        const auto& class_graph = class_state_space.get_graph();

        EXPECT_EQ(class_graph.get_num_vertices(), 15);
        EXPECT_EQ(class_graph.get_num_edges(), 13);
        EXPECT_EQ(class_state_space.get_initial_vertices().size(), 2);
        EXPECT_EQ(class_state_space.get_goal_vertices().size(), 2);
        EXPECT_EQ(class_state_space.get_unsolvable_vertices().size(), 3);
        EXPECT_EQ(class_state_space.get_alive_vertices().size(), 10);
    }

    {
        /* With symmetry reduction */
        auto options = GeneralizedStateSpace::Options();
        options.problem_options.symmetry_pruning = true;
        const auto problem_class_state_space = GeneralizedStateSpace(search_contexts, options);
        const auto& class_state_space = problem_class_state_space.get_class_state_space();
        const auto& class_graph = class_state_space.get_graph();

        EXPECT_EQ(class_graph.get_num_vertices(), 12);
        EXPECT_EQ(class_graph.get_num_edges(), 11);
        EXPECT_EQ(class_state_space.get_initial_vertices().size(), 2);
        EXPECT_EQ(class_state_space.get_goal_vertices().size(), 1);
        EXPECT_EQ(class_state_space.get_unsolvable_vertices().size(), 3);
        EXPECT_EQ(class_state_space.get_alive_vertices().size(), 8);
    }
}

}
