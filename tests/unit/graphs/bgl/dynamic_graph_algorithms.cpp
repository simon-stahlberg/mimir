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

#include "mimir/graphs/bgl/dynamic_graph_algorithms.hpp"

#include "mimir/graphs/dynamic_graph.hpp"

#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/properties.hpp>
#include <gtest/gtest.h>

namespace mimir::tests
{
TEST(MimirTests, GraphsDynamicGraphStrongComponentsTest)
{
    auto graph = graphs::DynamicGraph<graphs::EmptyVertex, graphs::EmptyEdge> {};
    const auto vx = graph.add_vertex();
    const auto v1 = graph.add_vertex();
    const auto v2 = graph.add_vertex();
    const auto v3 = graph.add_vertex();
    const auto v4 = graph.add_vertex();
    graph.add_undirected_edge(v1, v2);
    graph.add_directed_edge(v2, v3);
    graph.remove_vertex(vx);

    {
        auto tagged_graph = graphs::DirectionTaggedType(graph, graphs::ForwardTag {});

        const auto [num_components, component_map] = graphs::bgl::strong_components(tagged_graph);
        EXPECT_EQ(num_components, 3);
        // First component
        EXPECT_EQ(component_map.at(v1), component_map.at(v2));
        // Second component
        EXPECT_NE(component_map.at(v1), component_map.at(v3));
        // Third component
        EXPECT_NE(component_map.at(v1), component_map.at(v4));
        EXPECT_NE(component_map.at(v3), component_map.at(v4));
    }
}

}
