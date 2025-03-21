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

using namespace mimir::datasets;

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

    auto context = search::GeneralizedSearchContext(domain_file, std::vector<fs::path> { problem1_file, problem2_file });

    {
        /* Without symmetry reduction */

        auto kb_options = KnowledgeBaseImpl::Options();

        auto& state_space_options = kb_options.state_space_options;
        state_space_options.symmetry_pruning = false;

        auto& generalized_state_space_options = kb_options.generalized_state_space_options;
        generalized_state_space_options = GeneralizedStateSpaceImpl::Options();
        generalized_state_space_options->symmetry_pruning = false;

        auto& tuple_graph_options = kb_options.tuple_graph_options;
        tuple_graph_options = TupleGraphImpl::Options();
        tuple_graph_options->width = 2;

        auto kb = KnowledgeBaseImpl::create(context, kb_options);

        // Test generalized state space.
        const auto& generalized_state_space = kb->get_generalized_state_space().value();
        const auto& class_graph = generalized_state_space->get_graph();

        EXPECT_EQ(class_graph.get_num_vertices(), 15);
        EXPECT_EQ(class_graph.get_num_edges(), 13);
        EXPECT_EQ(generalized_state_space->get_initial_vertices().size(), 2);
        EXPECT_EQ(generalized_state_space->get_goal_vertices().size(), 2);
        EXPECT_EQ(generalized_state_space->get_unsolvable_vertices().size(), 3);

        // Test tuple graph collection.
        EXPECT_EQ(kb->get_tuple_graphs().value().at(0).size(), 7);
        EXPECT_EQ(kb->get_tuple_graphs().value().at(1).size(), 8);

        auto num_tuple_graph_vertices = size_t(0);
        auto num_tuple_graph_edges = size_t(0);
        for (const auto& tuple_graph_list : kb->get_tuple_graphs().value())
        {
            for (const auto& tuple_graph : tuple_graph_list)
            {
                num_tuple_graph_vertices += tuple_graph->get_graph().get_num_vertices();
                num_tuple_graph_edges += tuple_graph->get_graph().get_num_edges();
            }
        }

        EXPECT_EQ(num_tuple_graph_vertices, 53);
        EXPECT_EQ(num_tuple_graph_edges, 38);
    }

    {
        /* With symmetry reduction */
        auto kb_options = KnowledgeBaseImpl::Options();

        auto& state_space_options = kb_options.state_space_options;
        state_space_options.symmetry_pruning = true;

        auto& generalized_state_space_options = kb_options.generalized_state_space_options;
        generalized_state_space_options = GeneralizedStateSpaceImpl::Options();
        generalized_state_space_options->symmetry_pruning = true;

        auto& tuple_graph_options = kb_options.tuple_graph_options;
        tuple_graph_options = TupleGraphImpl::Options();
        tuple_graph_options->width = 2;

        auto kb = KnowledgeBaseImpl::create(context, kb_options);

        // Test generalized state space.
        const auto& generalized_state_space = kb->get_generalized_state_space().value();
        const auto& class_graph = generalized_state_space->get_graph();

        EXPECT_EQ(class_graph.get_num_vertices(), 12);
        EXPECT_EQ(class_graph.get_num_edges(), 11);
        EXPECT_EQ(generalized_state_space->get_initial_vertices().size(), 2);
        EXPECT_EQ(generalized_state_space->get_goal_vertices().size(), 1);
        EXPECT_EQ(generalized_state_space->get_unsolvable_vertices().size(), 3);

        // Test tuple graph collection.
        EXPECT_EQ(kb->get_tuple_graphs().value().at(0).size(), 7);

        auto num_tuple_graph_vertices = size_t(0);
        auto num_tuple_graph_edges = size_t(0);
        for (const auto& tuple_graph_list : kb->get_tuple_graphs().value())
        {
            for (const auto& tuple_graph : tuple_graph_list)
            {
                num_tuple_graph_vertices += tuple_graph->get_graph().get_num_vertices();
                num_tuple_graph_edges += tuple_graph->get_graph().get_num_edges();
            }
        }

        EXPECT_EQ(num_tuple_graph_vertices, 53);
        EXPECT_EQ(num_tuple_graph_edges, 38);

        // Test subspace from problem index 0
        const auto& class_state_space_0 = generalized_state_space->create_induced_subgraph_from_problem_indices(IndexList { 0 });
        const auto& class_graph_0 = class_state_space_0.get_graph();

        EXPECT_EQ(class_graph_0.get_num_vertices(), 7);
        EXPECT_EQ(class_graph_0.get_num_edges(), 6);

        // Test subspace from problem index 1
        const auto& class_state_space_1 = generalized_state_space->create_induced_subgraph_from_problem_indices(IndexList { 1 });
        const auto& class_graph_1 = class_state_space_1.get_graph();

        EXPECT_EQ(class_graph_1.get_num_vertices(), 8);
        EXPECT_EQ(class_graph_1.get_num_edges(), 7);

        // Test subspace from even vertex indices
        auto class_vertex_indices = IndexList {};
        for (Index i = 0; i < class_graph.get_num_vertices(); i += 2)
        {
            class_vertex_indices.push_back(i);
        }

        const auto& class_state_space_even = generalized_state_space->create_induced_subgraph_from_class_vertex_indices(class_vertex_indices);
        const auto& class_graph_even = class_state_space_even.get_graph();

        EXPECT_EQ(class_graph_even.get_num_vertices(), 6);
        EXPECT_EQ(class_graph_even.get_num_edges(), 2);
    }
}

}
