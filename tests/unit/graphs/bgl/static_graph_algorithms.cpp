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

#include "mimir/graphs/bgl/static_graph_algorithms.hpp"

#include "mimir/datasets/state_space.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/search_context.hpp"

#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/properties.hpp>
#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, GraphsDijkstraShortestPathTest)
{
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
        const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

        const auto state_space_result = datasets::StateSpaceImpl::create(search::SearchContextImpl::create(domain_file, problem_file));
        const auto& graph = state_space_result->first->get_graph();
        auto tagged_graph = graphs::DirectionTaggedType(graph, graphs::ForwardTag {});

        const auto edge_costs = ContinuousCostList(graph.get_num_edges(), 1);
        auto states = IndexList { 0 };
        const auto [predecessor_map, distance_map] = graphs::bgl::dijkstra_shortest_paths(tagged_graph, edge_costs, states.begin(), states.end());

        EXPECT_EQ(distance_map.at(0), 0);
        for (const auto& goal_state : state_space_result->first->get_goal_vertices())
        {
            EXPECT_GT(distance_map.at(goal_state), 0);
        }
        // There are zero deadend state.
        EXPECT_EQ(std::count(distance_map.begin(), distance_map.end(), std::numeric_limits<ContinuousCost>::infinity()), 0);
    }
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "spanner/domain.pddl");
        const auto problem_file = fs::path(std::string(DATA_DIR) + "spanner/test_problem.pddl");

        const auto state_space_result = datasets::StateSpaceImpl::create(search::SearchContextImpl::create(domain_file, problem_file));
        const auto& graph = state_space_result->first->get_graph();
        auto tagged_graph = graphs::DirectionTaggedType(graph, graphs::BackwardTag {});

        const auto edge_costs = ContinuousCostList(graph.get_num_edges(), 1);
        const auto [predecessor_map, distance_map] = graphs::bgl::dijkstra_shortest_paths(tagged_graph,
                                                                                          edge_costs,
                                                                                          state_space_result->first->get_goal_vertices().begin(),
                                                                                          state_space_result->first->get_goal_vertices().end());

        EXPECT_EQ(distance_map.at(0), 4);
        // There is one deadend state.
        EXPECT_EQ(std::count(distance_map.begin(), distance_map.end(), std::numeric_limits<ContinuousCost>::infinity()), 1);
    }
}

TEST(MimirTests, GraphsFloydWarshallAllPairsShortestPathTest)
{
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
        const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

        const auto state_space_result = datasets::StateSpaceImpl::create(search::SearchContextImpl::create(domain_file, problem_file));
        const auto& graph = state_space_result->first->get_graph();
        auto tagged_graph = graphs::DirectionTaggedType(graph, graphs::ForwardTag {});

        const auto edge_costs = ContinuousCostList(graph.get_num_edges(), 1);
        const auto distance_matrix = graphs::bgl::floyd_warshall_all_pairs_shortest_paths(tagged_graph, edge_costs);

        auto min_goal_distance = std::numeric_limits<ContinuousCost>::infinity();
        for (const auto& goal_state : state_space_result->first->get_goal_vertices())
        {
            min_goal_distance = std::min(min_goal_distance, distance_matrix[goal_state][0]);
        }
        EXPECT_GT(min_goal_distance, 0);
        EXPECT_NE(min_goal_distance, std::numeric_limits<ContinuousCost>::infinity());
        auto max_pairwise_distance = ContinuousCost();
        for (auto v1 = Index(0); v1 < tagged_graph.get().get_num_vertices(); ++v1)
        {
            for (auto v2 = Index(0); v2 < tagged_graph.get().get_num_vertices(); ++v2)
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

        const auto state_space_result = datasets::StateSpaceImpl::create(search::SearchContextImpl::create(domain_file, problem_file));
        const auto& graph = state_space_result->first->get_graph();
        auto tagged_graph = graphs::DirectionTaggedType(graph, graphs::BackwardTag {});

        const auto edge_costs = ContinuousCostList(graph.get_num_edges(), 1);
        const auto distance_matrix = graphs::bgl::floyd_warshall_all_pairs_shortest_paths(tagged_graph, edge_costs);

        auto min_goal_distance = std::numeric_limits<ContinuousCost>::infinity();
        for (const auto& goal_state : state_space_result->first->get_goal_vertices())
        {
            min_goal_distance = std::min(min_goal_distance, distance_matrix[goal_state][0]);
        }
        EXPECT_GT(min_goal_distance, 0);
        EXPECT_NE(min_goal_distance, std::numeric_limits<ContinuousCost>::infinity());
        auto max_pairwise_distance = ContinuousCost();
        for (auto v1 = Index(0); v1 < tagged_graph.get().get_num_vertices(); ++v1)
        {
            for (auto v2 = Index(0); v2 < tagged_graph.get().get_num_vertices(); ++v2)
            {
                max_pairwise_distance = std::max(max_pairwise_distance, distance_matrix[v1][v2]);
            }
        }
        // There exist deadend states
        // Boost seems to initialize the distances with max instead of infinity
        EXPECT_EQ(max_pairwise_distance, std::numeric_limits<ContinuousCost>::max());
    }
}

TEST(MimirTests, GraphsStrongComponentsTest)
{
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
        const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

        const auto state_space_result = datasets::StateSpaceImpl::create(search::SearchContextImpl::create(domain_file, problem_file));
        const auto& graph = state_space_result->first->get_graph();
        auto tagged_graph = graphs::DirectionTaggedType(graph, graphs::ForwardTag {});

        const auto [num_components, component_map] = graphs::bgl::strong_components(tagged_graph);
        EXPECT_EQ(num_components, 1);
        for (auto [it, last] = vertices(tagged_graph); it != last; ++it)
        {
            EXPECT_EQ(component_map.at(*it), 0);
        }
    }
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "spanner/domain.pddl");
        const auto problem_file = fs::path(std::string(DATA_DIR) + "spanner/test_problem.pddl");

        const auto state_space_result = datasets::StateSpaceImpl::create(search::SearchContextImpl::create(domain_file, problem_file));
        const auto& graph = state_space_result->first->get_graph();
        auto tagged_graph = graphs::DirectionTaggedType(graph, graphs::ForwardTag {});

        const auto [num_components, component_map] = graphs::bgl::strong_components(tagged_graph);

        // Each state should have its own component.
        EXPECT_EQ(num_components, num_vertices(tagged_graph));

        // Each component should only have one state.
        std::map<size_t, size_t> num_states_per_component;
        for (auto [it, last] = vertices(tagged_graph); it != last; ++it)
        {
            num_states_per_component[component_map.at(*it)]++;
        }
        for (const auto& [key, val] : num_states_per_component)
        {
            EXPECT_EQ(val, 1);
        }
    }
}

TEST(MimirTests, GraphsBreadthFirstSearchTest)
{
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
        const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

        const auto state_space_result = datasets::StateSpaceImpl::create(search::SearchContextImpl::create(domain_file, problem_file));
        const auto& graph = state_space_result->first->get_graph();
        auto tagged_graph = graphs::DirectionTaggedType(graph, graphs::ForwardTag {});

        auto states = IndexList { 0 };
        const auto [predecessor_map, distance_map] = graphs::bgl::breadth_first_search(tagged_graph, states.begin(), states.end());

        EXPECT_EQ(distance_map.at(0), 0);
        for (const auto& goal_state : state_space_result->first->get_goal_vertices())
        {
            EXPECT_GT(distance_map.at(goal_state), 0);
        }
        // There are zero deadend state.
        EXPECT_EQ(std::count(distance_map.begin(), distance_map.end(), UNDEFINED_DISCRETE_COST), 0);
    }
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "spanner/domain.pddl");
        const auto problem_file = fs::path(std::string(DATA_DIR) + "spanner/test_problem.pddl");

        const auto state_space_result = datasets::StateSpaceImpl::create(search::SearchContextImpl::create(domain_file, problem_file));
        const auto& graph = state_space_result->first->get_graph();
        auto tagged_graph = graphs::DirectionTaggedType(graph, graphs::BackwardTag {});

        const auto [predecessor_map, distance_map] = graphs::bgl::breadth_first_search(tagged_graph,
                                                                                       state_space_result->first->get_goal_vertices().begin(),
                                                                                       state_space_result->first->get_goal_vertices().end());

        EXPECT_EQ(distance_map.at(0), 4);
        // There is one deadend state.
        EXPECT_EQ(std::count(distance_map.begin(), distance_map.end(), UNDEFINED_DISCRETE_COST), 1);
    }
}

}
