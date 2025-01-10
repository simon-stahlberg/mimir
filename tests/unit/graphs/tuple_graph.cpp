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

#include "mimir/graphs/tuple_graph.hpp"

#include "mimir/formalism/utils.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, GraphsTupleGraphTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-1-0.pddl");
    PDDLParser parser(domain_file, problem_file);

    const auto state_space = std::make_shared<StateSpace>(std::move(StateSpace::create(domain_file, problem_file).value()));

    EXPECT_EQ(state_space->get_num_vertices(), 8);

    auto tuple_graph_factory_0 = TupleGraphFactory(state_space, 0, false);
    auto tuple_graph_factory_1 = TupleGraphFactory(state_space, 1, false);
    auto tuple_graph_factory_2 = TupleGraphFactory(state_space, 2, false);
    auto tuple_graph_factory_0_pruned = TupleGraphFactory(state_space, 0, true);
    auto tuple_graph_factory_1_pruned = TupleGraphFactory(state_space, 1, true);
    auto tuple_graph_factory_2_pruned = TupleGraphFactory(state_space, 2, true);

    auto tuple_graphs_0 = TupleGraphList {};
    auto tuple_graphs_1 = TupleGraphList {};
    auto tuple_graphs_2 = TupleGraphList {};
    auto tuple_graphs_0_pruned = TupleGraphList {};
    auto tuple_graphs_1_pruned = TupleGraphList {};
    auto tuple_graphs_2_pruned = TupleGraphList {};

    for (const auto& vertex : state_space->get_graph().get_vertices())
    {
        const auto state = get_state(vertex);
        // std::cout << std::make_tuple(state_space->get_applicable_action_generator()->get_problem(), state,
        // std::cref(*state_space->get_applicable_action_generator()->get_pddl_repositories())) << std::endl;

        tuple_graphs_0.push_back(tuple_graph_factory_0.create(state));
        tuple_graphs_1.push_back(tuple_graph_factory_1.create(state));
        tuple_graphs_2.push_back(tuple_graph_factory_2.create(state));

        tuple_graphs_0_pruned.push_back(tuple_graph_factory_0_pruned.create(state));
        tuple_graphs_1_pruned.push_back(tuple_graph_factory_1_pruned.create(state));
        tuple_graphs_2_pruned.push_back(tuple_graph_factory_2_pruned.create(state));

        // std::cout << tuple_graph << std::endl;
    }

    // std::cout << tuple_graphs_0.at(0) << std::endl;
    // std::cout << tuple_graphs_1.at(0) << std::endl;
    // std::cout << tuple_graphs_2.at(0) << std::endl;

    // std::cout << tuple_graphs_0.at(7) << std::endl;
    // std::cout << tuple_graphs_1.at(7) << std::endl;
    // std::cout << tuple_graphs_2.at(7) << std::endl;

    EXPECT_EQ(tuple_graphs_0.at(0).get_digraph().get_num_vertices(), 4);
    EXPECT_EQ(tuple_graphs_0.at(0).get_digraph().get_num_edges(), 3);
    EXPECT_EQ(tuple_graphs_0.at(0).get_vertices_grouped_by_distance().size(), 2);
    EXPECT_EQ(tuple_graphs_0.at(0).get_states_grouped_by_distance().size(), 2);
    EXPECT_EQ(tuple_graphs_1.at(0).get_digraph().get_num_vertices(), 8);
    EXPECT_EQ(tuple_graphs_1.at(0).get_digraph().get_num_edges(), 15);
    EXPECT_EQ(tuple_graphs_1.at(0).get_vertices_grouped_by_distance().size(), 2);
    EXPECT_EQ(tuple_graphs_1.at(0).get_states_grouped_by_distance().size(), 2);
    EXPECT_EQ(tuple_graphs_2.at(0).get_digraph().get_num_vertices(), 28);
    EXPECT_EQ(tuple_graphs_2.at(0).get_digraph().get_num_edges(), 128);
    EXPECT_EQ(tuple_graphs_2.at(0).get_vertices_grouped_by_distance().size(), 5);
    EXPECT_EQ(tuple_graphs_2.at(0).get_states_grouped_by_distance().size(), 5);

    EXPECT_EQ(tuple_graphs_0_pruned.at(0).get_digraph().get_num_vertices(), 4);
    EXPECT_EQ(tuple_graphs_0_pruned.at(0).get_digraph().get_num_edges(), 3);
    EXPECT_EQ(tuple_graphs_0_pruned.at(0).get_vertices_grouped_by_distance().size(), 2);
    EXPECT_EQ(tuple_graphs_0_pruned.at(0).get_states_grouped_by_distance().size(), 2);
    EXPECT_EQ(tuple_graphs_1_pruned.at(0).get_digraph().get_num_vertices(), 4);
    EXPECT_EQ(tuple_graphs_1_pruned.at(0).get_digraph().get_num_edges(), 3);
    EXPECT_EQ(tuple_graphs_1_pruned.at(0).get_vertices_grouped_by_distance().size(), 2);
    EXPECT_EQ(tuple_graphs_1_pruned.at(0).get_states_grouped_by_distance().size(), 2);
    EXPECT_EQ(tuple_graphs_2_pruned.at(0).get_digraph().get_num_vertices(), 8);
    EXPECT_EQ(tuple_graphs_2_pruned.at(0).get_digraph().get_num_edges(), 8);
    EXPECT_EQ(tuple_graphs_2_pruned.at(0).get_vertices_grouped_by_distance().size(), 5);
    EXPECT_EQ(tuple_graphs_2_pruned.at(0).get_states_grouped_by_distance().size(), 5);

    EXPECT_EQ(tuple_graphs_0.at(7).get_digraph().get_num_vertices(), 3);
    EXPECT_EQ(tuple_graphs_0.at(7).get_digraph().get_num_edges(), 2);
    EXPECT_EQ(tuple_graphs_0.at(7).get_vertices_grouped_by_distance().size(), 2);
    EXPECT_EQ(tuple_graphs_0.at(7).get_states_grouped_by_distance().size(), 2);
    EXPECT_EQ(tuple_graphs_1.at(7).get_digraph().get_num_vertices(), 9);
    EXPECT_EQ(tuple_graphs_1.at(7).get_digraph().get_num_edges(), 15);
    EXPECT_EQ(tuple_graphs_1.at(7).get_vertices_grouped_by_distance().size(), 3);
    EXPECT_EQ(tuple_graphs_1.at(7).get_states_grouped_by_distance().size(), 3);
    EXPECT_EQ(tuple_graphs_2.at(7).get_digraph().get_num_vertices(), 28);
    EXPECT_EQ(tuple_graphs_2.at(7).get_digraph().get_num_edges(), 131);
    EXPECT_EQ(tuple_graphs_2.at(7).get_vertices_grouped_by_distance().size(), 4);
    EXPECT_EQ(tuple_graphs_2.at(7).get_states_grouped_by_distance().size(), 4);

    EXPECT_EQ(tuple_graphs_0_pruned.at(7).get_digraph().get_num_vertices(), 3);
    EXPECT_EQ(tuple_graphs_0_pruned.at(7).get_digraph().get_num_edges(), 2);
    EXPECT_EQ(tuple_graphs_0_pruned.at(7).get_vertices_grouped_by_distance().size(), 2);
    EXPECT_EQ(tuple_graphs_0_pruned.at(7).get_states_grouped_by_distance().size(), 2);
    EXPECT_EQ(tuple_graphs_1_pruned.at(7).get_digraph().get_num_vertices(), 5);
    EXPECT_EQ(tuple_graphs_1_pruned.at(7).get_digraph().get_num_edges(), 4);
    EXPECT_EQ(tuple_graphs_1_pruned.at(7).get_vertices_grouped_by_distance().size(), 3);
    EXPECT_EQ(tuple_graphs_1_pruned.at(7).get_states_grouped_by_distance().size(), 3);
    EXPECT_EQ(tuple_graphs_2_pruned.at(7).get_digraph().get_num_vertices(), 8);
    EXPECT_EQ(tuple_graphs_2_pruned.at(7).get_digraph().get_num_edges(), 8);
    EXPECT_EQ(tuple_graphs_2_pruned.at(7).get_vertices_grouped_by_distance().size(), 4);
    EXPECT_EQ(tuple_graphs_2_pruned.at(7).get_states_grouped_by_distance().size(), 4);
}

TEST(MimirTests, GraphsTupleGraphAdmissibleChainTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "visitall/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "visitall/test_problem2.pddl");
    PDDLParser parser(domain_file, problem_file);

    const auto state_space = std::make_shared<StateSpace>(std::move(StateSpace::create(domain_file, problem_file).value()));

    auto tuple_graph_factory_0 = TupleGraphFactory(state_space, 0, false);
    auto tuple_graph_factory_1 = TupleGraphFactory(state_space, 1, false);
    auto tuple_graph_factory_2 = TupleGraphFactory(state_space, 2, false);
    auto tuple_graph_factory_3 = TupleGraphFactory(state_space, 3, false);

    auto tuple_graph_0 = tuple_graph_factory_0.create(state_space->get_state_repository()->get_or_create_initial_state());
    auto tuple_graph_1 = tuple_graph_factory_1.create(state_space->get_state_repository()->get_or_create_initial_state());
    auto tuple_graph_2 = tuple_graph_factory_2.create(state_space->get_state_repository()->get_or_create_initial_state());
    auto tuple_graph_3 = tuple_graph_factory_3.create(state_space->get_state_repository()->get_or_create_initial_state());

    auto fluent_goal_atoms = to_ground_atoms(parser.get_problem()->get_goal_condition<Fluent>());
    // We skip constructing derived goal atoms because there are none in visitall.

    EXPECT_EQ(tuple_graph_0.compute_admissible_chain(fluent_goal_atoms), std::nullopt);
    EXPECT_EQ(tuple_graph_1.compute_admissible_chain(fluent_goal_atoms), std::nullopt);
    EXPECT_EQ(tuple_graph_2.compute_admissible_chain(fluent_goal_atoms), std::nullopt);
    EXPECT_NE(tuple_graph_3.compute_admissible_chain(fluent_goal_atoms), std::nullopt);
}

}
