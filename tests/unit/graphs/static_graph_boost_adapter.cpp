/*
 * Copyright (C) 2023 Dominik Drexler and Till Hofmann
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

#include "mimir/graphs/static_graph_boost_adapter.hpp"

#include "mimir/datasets/state_space.hpp"

#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/properties.hpp>
#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, GraphsVertexListGraphTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");
    const auto state_space = StateSpace::create(domain_file, problem_file).value();
    auto graph = TraversalDirectionTaggedType(state_space.get_graph(), ForwardTraversal());

    EXPECT_EQ(num_vertices(graph), 28);

    auto [it, last] = vertices(graph);
    for (int i = 0; i < 28; ++i, ++it)
    {
        EXPECT_EQ(*it, i);
    }
    EXPECT_EQ(it, last);
}

TEST(MimirTests, GraphsIncidenceGraphTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");
    const auto state_space = StateSpace::create(domain_file, problem_file).value();
    auto graph = TraversalDirectionTaggedType(state_space.get_graph(), ForwardTraversal());

    size_t transition_count = 0;
    for (auto [state_it, state_last] = vertices(graph); state_it != state_last; ++state_it)
    {
        const auto& state_index = *state_it;
        size_t state_transition_count = 0;
        for (auto [out_it, out_last] = out_edges(state_index, graph); out_it != out_last; ++out_it)
        {
            EXPECT_EQ(source(*out_it, graph), state_index);
            transition_count++;
            state_transition_count++;
        }
        // Counting the number of transitions should give us the out degree.
        EXPECT_EQ(out_degree(*state_it, graph), state_transition_count);
    }
    // Summing over the successors of each state should give the total number of transitions.
    EXPECT_EQ(transition_count, state_space.get_num_edges());

    // possible actions:
    // pick(ball1, rooma, right), pick(ball1, rooma, left), pick(ball2, rooma, right), pick(ball2, rooma, left)
    // move(rooma, rooma), move(rooma, roomb)
    EXPECT_EQ(out_degree(state_space.get_initial_vertex_index(), graph), 6);
}

TEST(MimirTests, GraphsStrongComponentsTest)
{
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
        const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");
        const auto state_space = StateSpace::create(domain_file, problem_file).value();
        auto graph = TraversalDirectionTaggedType(state_space.get_graph(), ForwardTraversal());

        const auto [num_components, component_map] = strong_components(graph);
        EXPECT_EQ(num_components, 1);
        for (auto [it, last] = vertices(graph); it != last; ++it)
        {
            EXPECT_EQ(component_map.at(*it), 0);
        }
    }
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "spanner/domain.pddl");
        const auto problem_file = fs::path(std::string(DATA_DIR) + "spanner/test_problem.pddl");
        const auto state_space = StateSpace::create(domain_file, problem_file).value();
        auto graph = TraversalDirectionTaggedType(state_space.get_graph(), ForwardTraversal());

        const auto [num_components, component_map] = strong_components(graph);

        // Each state should have its own component.
        EXPECT_EQ(num_components, num_vertices(graph));

        // Each component should only have one state.
        std::map<size_t, size_t> num_states_per_component;
        for (auto [it, last] = vertices(graph); it != last; ++it)
        {
            num_states_per_component[component_map.at(*it)]++;
        }
        for (const auto& [key, val] : num_states_per_component)
        {
            EXPECT_EQ(val, 1);
        }
    }
}

TEST(MimirTests, GraphsDijkstraShortestPathTest)
{
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
        const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");
        const auto state_space = StateSpace::create(domain_file, problem_file).value();
        auto graph = TraversalDirectionTaggedType(state_space.get_graph(), ForwardTraversal());

        const auto edge_costs = std::vector<double>(state_space.get_num_edges(), 1);
        auto states = IndexList { state_space.get_initial_vertex_index() };
        const auto [predecessor_map, distance_map] = dijkstra_shortest_paths(graph, edge_costs, states.begin(), states.end());

        EXPECT_EQ(distance_map.at(state_space.get_initial_vertex_index()), 0);
        for (const auto& goal_state : state_space.get_goal_vertex_indices())
        {
            EXPECT_GT(distance_map.at(goal_state), 0);
        }
        // There are zero deadend state.
        EXPECT_EQ(std::count(distance_map.begin(), distance_map.end(), std::numeric_limits<ContinuousCost>::infinity()), 0);
    }
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "spanner/domain.pddl");
        const auto problem_file = fs::path(std::string(DATA_DIR) + "spanner/test_problem.pddl");
        const auto state_space = StateSpace::create(domain_file, problem_file).value();
        auto graph = TraversalDirectionTaggedType(state_space.get_graph(), BackwardTraversal());

        const auto edge_costs = std::vector<double>(state_space.get_num_edges(), 1);
        const auto [predecessor_map, distance_map] =
            dijkstra_shortest_paths(graph, edge_costs, state_space.get_goal_vertex_indices().begin(), state_space.get_goal_vertex_indices().end());

        EXPECT_EQ(distance_map.at(state_space.get_initial_vertex_index()), 4);
        // There is one deadend state.
        EXPECT_EQ(std::count(distance_map.begin(), distance_map.end(), std::numeric_limits<ContinuousCost>::infinity()), 1);
    }
}

TEST(MimirTests, GraphsBreadthFirstSearchTest)
{
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
        const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");
        const auto state_space = StateSpace::create(domain_file, problem_file).value();
        auto graph = TraversalDirectionTaggedType(state_space.get_graph(), ForwardTraversal());

        auto states = IndexList { state_space.get_initial_vertex_index() };
        const auto [predecessor_map, distance_map] = breadth_first_search(graph, states.begin(), states.end());

        EXPECT_EQ(distance_map.at(state_space.get_initial_vertex_index()), 0);
        for (const auto& goal_state : state_space.get_goal_vertex_indices())
        {
            EXPECT_GT(distance_map.at(goal_state), 0);
        }
        // There are zero deadend state.
        EXPECT_EQ(std::count(distance_map.begin(), distance_map.end(), std::numeric_limits<ContinuousCost>::infinity()), 0);
    }
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "spanner/domain.pddl");
        const auto problem_file = fs::path(std::string(DATA_DIR) + "spanner/test_problem.pddl");
        const auto state_space = StateSpace::create(domain_file, problem_file).value();
        auto graph = TraversalDirectionTaggedType(state_space.get_graph(), BackwardTraversal());

        const auto [predecessor_map, distance_map] =
            breadth_first_search(graph, state_space.get_goal_vertex_indices().begin(), state_space.get_goal_vertex_indices().end());

        EXPECT_EQ(distance_map.at(state_space.get_initial_vertex_index()), 4);
        // There is one deadend state.
        EXPECT_EQ(std::count(distance_map.begin(), distance_map.end(), std::numeric_limits<ContinuousCost>::infinity()), 1);
    }
}

TEST(MimirTests, GraphsFloydWarshallAllPairsShortestPathTest)
{
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
        const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");
        const auto state_space = StateSpace::create(domain_file, problem_file).value();
        auto graph = TraversalDirectionTaggedType(state_space.get_graph(), ForwardTraversal());

        const auto edge_costs = std::vector<double>(state_space.get_num_edges(), 1);
        const auto distance_matrix = floyd_warshall_all_pairs_shortest_paths(graph, edge_costs);

        auto min_goal_distance = std::numeric_limits<ContinuousCost>::infinity();
        for (const auto& goal_state : state_space.get_goal_vertex_indices())
        {
            min_goal_distance = std::min(min_goal_distance, distance_matrix[goal_state][state_space.get_initial_vertex_index()]);
        }
        EXPECT_GT(min_goal_distance, 0);
        EXPECT_NE(min_goal_distance, std::numeric_limits<ContinuousCost>::infinity());
        auto max_pairwise_distance = ContinuousCost();
        for (auto v1 = Index(0); v1 < graph.get().get_num_vertices(); ++v1)
        {
            for (auto v2 = Index(0); v2 < graph.get().get_num_vertices(); ++v2)
            {
                max_pairwise_distance = std::max(max_pairwise_distance, distance_matrix[v1][v2]);
            }
        }
        // All states are pairwise reachable
        EXPECT_NE(max_pairwise_distance, std::numeric_limits<ContinuousCost>::infinity());
    }
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "spanner/domain.pddl");
        const auto problem_file = fs::path(std::string(DATA_DIR) + "spanner/test_problem.pddl");
        const auto state_space = StateSpace::create(domain_file, problem_file).value();
        auto graph = TraversalDirectionTaggedType(state_space.get_graph(), BackwardTraversal());

        const auto edge_costs = ContinuousCostList(state_space.get_num_edges(), 1);
        const auto distance_matrix = floyd_warshall_all_pairs_shortest_paths(graph, edge_costs);

        auto min_goal_distance = std::numeric_limits<ContinuousCost>::infinity();
        for (const auto& goal_state : state_space.get_goal_vertex_indices())
        {
            min_goal_distance = std::min(min_goal_distance, distance_matrix[goal_state][state_space.get_initial_vertex_index()]);
        }
        EXPECT_GT(min_goal_distance, 0);
        EXPECT_NE(min_goal_distance, std::numeric_limits<ContinuousCost>::infinity());
        auto max_pairwise_distance = ContinuousCost();
        for (auto v1 = Index(0); v1 < graph.get().get_num_vertices(); ++v1)
        {
            for (auto v2 = Index(0); v2 < graph.get().get_num_vertices(); ++v2)
            {
                max_pairwise_distance = std::max(max_pairwise_distance, distance_matrix[v1][v2]);
            }
        }
        // There exist deadend states
        // Boost seems to initialize the distances with max instead of infinity
        EXPECT_EQ(max_pairwise_distance, std::numeric_limits<ContinuousCost>::max());
    }
}

}
