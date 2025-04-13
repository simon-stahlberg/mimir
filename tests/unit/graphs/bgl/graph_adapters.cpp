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

#include "mimir/graphs/bgl/graph_adapters.hpp"

#include "mimir/datasets/state_space.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/search_context.hpp"

#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/properties.hpp>
#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, GraphsVertexListGraphTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

    const auto state_space_result = datasets::StateSpaceImpl::create(
        search::SearchContextImpl::create(domain_file, problem_file, search::SearchContextImpl::Options(search::SearchContextImpl::SearchMode::GROUNDED)));
    auto graph = graphs::DirectionTaggedType(state_space_result->first->get_graph(), graphs::ForwardTag {});

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

    const auto state_space_result = datasets::StateSpaceImpl::create(
        search::SearchContextImpl::create(domain_file, problem_file, search::SearchContextImpl::Options(search::SearchContextImpl::SearchMode::GROUNDED)));
    const auto& graph = state_space_result->first->get_graph();
    auto tagged_graph = graphs::DirectionTaggedType(graph, graphs::ForwardTag {});

    size_t transition_count = 0;
    for (auto [state_it, state_last] = vertices(tagged_graph); state_it != state_last; ++state_it)
    {
        const auto& state_index = *state_it;
        size_t state_transition_count = 0;
        for (auto [out_it, out_last] = out_edges(state_index, tagged_graph); out_it != out_last; ++out_it)
        {
            EXPECT_EQ(source(*out_it, tagged_graph), state_index);
            transition_count++;
            state_transition_count++;
        }
        // Counting the number of transitions should give us the out degree.
        EXPECT_EQ(out_degree(*state_it, tagged_graph), state_transition_count);
    }
    // Summing over the successors of each state should give the total number of transitions.
    EXPECT_EQ(transition_count, graph.get_num_edges());

    // possible actions:
    // pick(ball1, rooma, right), pick(ball1, rooma, left), pick(ball2, rooma, right), pick(ball2, rooma, left)
    // move(rooma, rooma), move(rooma, roomb)
    EXPECT_EQ(out_degree(0, tagged_graph), 6);  // 0 is the initial vertex
}

}
