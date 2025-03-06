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

#include "mimir/datasets/knowledge_base.hpp"

#include "mimir/formalism/problem.hpp"
#include "mimir/search/search_context.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, DatasetsKnowledgeBaseConstructorTest)
{
    /* Test two spanner problems with two locations and a single spanner each. */
    const auto domain_file = fs::path(std::string(DATA_DIR) + "spanner/domain.pddl");
    // The spanner is at location 1.
    const auto problem1_file = fs::path(std::string(DATA_DIR) + "spanner/p-1-1-2-1.pddl");
    // The spanner is at location 2.
    const auto problem2_file = fs::path(std::string(DATA_DIR) + "spanner/p-1-1-2-1(2).pddl");

    auto context = GeneralizedSearchContext(domain_file, std::vector<fs::path> { problem1_file, problem2_file });

    {
        /* Without symmetry reduction */
        auto state_space_options = GeneralizedStateSpace::Options();
        state_space_options.problem_options.symmetry_pruning = false;

        auto tuple_graph_options = TupleGraphCollection::Options();
        tuple_graph_options.width = 2;

        auto kb = KnowledgeBase::create(context, KnowledgeBase::Options(state_space_options, tuple_graph_options));

        // Test generalized state space.
        const auto& generalized_state_space = kb->get_generalized_state_space();
        const auto& class_graph = generalized_state_space.get_graph();

        EXPECT_EQ(class_graph.get_num_vertices(), 15);
        EXPECT_EQ(class_graph.get_num_edges(), 13);
        EXPECT_EQ(generalized_state_space.get_initial_vertices().size(), 2);
        EXPECT_EQ(generalized_state_space.get_goal_vertices().size(), 2);
        EXPECT_EQ(generalized_state_space.get_unsolvable_vertices().size(), 3);

        // Test tuple graph collection.
        EXPECT_EQ(kb->get_tuple_graphs().value().get_per_class_vertex_tuple_graph().size(), 15);

        auto num_tuple_graph_vertices = size_t(0);
        auto num_tuple_graph_edges = size_t(0);
        for (const auto& tuple_graph : kb->get_tuple_graphs().value().get_per_class_vertex_tuple_graph())
        {
            num_tuple_graph_vertices += tuple_graph.get_graph().get_num_vertices();
            num_tuple_graph_edges += tuple_graph.get_graph().get_num_edges();
        }

        EXPECT_EQ(num_tuple_graph_vertices, 53);
        EXPECT_EQ(num_tuple_graph_edges, 38);
    }

    {
        /* With symmetry reduction */
        auto state_space_options = GeneralizedStateSpace::Options();
        state_space_options.problem_options.symmetry_pruning = true;

        auto tuple_graph_options = TupleGraphCollection::Options();
        tuple_graph_options.width = 2;

        auto kb = KnowledgeBase::create(context, KnowledgeBase::Options(state_space_options, tuple_graph_options));

        // Test generalized state space.
        const auto& generalized_state_space = kb->get_generalized_state_space();
        const auto& class_graph = generalized_state_space.get_graph();

        EXPECT_EQ(class_graph.get_num_vertices(), 12);
        EXPECT_EQ(class_graph.get_num_edges(), 11);
        EXPECT_EQ(generalized_state_space.get_initial_vertices().size(), 2);
        EXPECT_EQ(generalized_state_space.get_goal_vertices().size(), 1);
        EXPECT_EQ(generalized_state_space.get_unsolvable_vertices().size(), 3);

        // Test tuple graph collection.
        EXPECT_EQ(kb->get_tuple_graphs().value().get_per_class_vertex_tuple_graph().size(), 12);

        auto num_tuple_graph_vertices = size_t(0);
        auto num_tuple_graph_edges = size_t(0);
        for (const auto& tuple_graph : kb->get_tuple_graphs().value().get_per_class_vertex_tuple_graph())
        {
            num_tuple_graph_vertices += tuple_graph.get_graph().get_num_vertices();
            num_tuple_graph_edges += tuple_graph.get_graph().get_num_edges();
        }

        EXPECT_EQ(num_tuple_graph_vertices, 47);  ///< Nice! symmetry reduction decreases the size of tuple graphs :)
        EXPECT_EQ(num_tuple_graph_edges, 35);
    }
}

}
