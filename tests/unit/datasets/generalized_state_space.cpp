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

#include "mimir/formalism/problem.hpp"
#include "mimir/search/search_context.hpp"

#include <gtest/gtest.h>

using namespace mimir::datasets;

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

    auto context = search::GeneralizedSearchContext(domain_file, std::vector<fs::path> { problem1_file, problem2_file });

    {
        /* Without symmetry reduction */
        auto options = GeneralizedStateSpace::Options();
        options.problem_options.symmetry_pruning = false;
        const auto generalized_state_space = GeneralizedStateSpace(context, options);
        const auto& class_graph = generalized_state_space.get_graph();

        EXPECT_EQ(class_graph.get_num_vertices(), 15);
        EXPECT_EQ(class_graph.get_num_edges(), 13);
        EXPECT_EQ(generalized_state_space.get_initial_vertices().size(), 2);
        EXPECT_EQ(generalized_state_space.get_goal_vertices().size(), 2);
        EXPECT_EQ(generalized_state_space.get_unsolvable_vertices().size(), 3);
    }

    {
        /* With symmetry reduction */
        auto options = GeneralizedStateSpace::Options();
        options.problem_options.symmetry_pruning = true;
        const auto generalized_state_space = GeneralizedStateSpace(context, options);
        const auto& class_graph = generalized_state_space.get_graph();

        EXPECT_EQ(class_graph.get_num_vertices(), 12);
        EXPECT_EQ(class_graph.get_num_edges(), 11);
        EXPECT_EQ(generalized_state_space.get_initial_vertices().size(), 2);
        EXPECT_EQ(generalized_state_space.get_goal_vertices().size(), 1);
        EXPECT_EQ(generalized_state_space.get_unsolvable_vertices().size(), 3);

        // Test subspace from problem index 0
        const auto& class_state_space_0 = generalized_state_space.create_induced_subgraph_from_problem_indices(IndexList { 0 });
        const auto& class_graph_0 = class_state_space_0.get_graph();

        EXPECT_EQ(class_graph_0.get_num_vertices(), 7);
        EXPECT_EQ(class_graph_0.get_num_edges(), 6);

        // Test subspace from problem index 1
        const auto& class_state_space_1 = generalized_state_space.create_induced_subgraph_from_problem_indices(IndexList { 1 });
        const auto& class_graph_1 = class_state_space_1.get_graph();

        EXPECT_EQ(class_graph_1.get_num_vertices(), 8);
        EXPECT_EQ(class_graph_1.get_num_edges(), 7);

        // Test subspace from even vertex indices
        auto class_vertex_indices = IndexList {};
        for (Index i = 0; i < class_graph.get_num_vertices(); i += 2)
        {
            class_vertex_indices.push_back(i);
        }

        const auto& class_state_space_even = generalized_state_space.create_induced_subgraph_from_class_vertex_indices(class_vertex_indices);
        const auto& class_graph_even = class_state_space_even.get_graph();

        EXPECT_EQ(class_graph_even.get_num_vertices(), 6);
        EXPECT_EQ(class_graph_even.get_num_edges(), 2);
        EXPECT_EQ(generalized_state_space.get_initial_vertices().size(), 2);
        EXPECT_EQ(generalized_state_space.get_goal_vertices().size(), 1);
        EXPECT_EQ(generalized_state_space.get_unsolvable_vertices().size(), 3);
    }
}

}
