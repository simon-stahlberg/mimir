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

#include "mimir/graphs/concrete/digraph.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, GraphsStaticDigraphTest)
{
    /**
     * StaticDigraph
     */

    auto graph = graphs::StaticDigraph();

    /* Add edge with non existing source or target */
    EXPECT_ANY_THROW(graph.add_directed_edge(0, 0));
    EXPECT_ANY_THROW(graph.add_undirected_edge(1, 2));

    /* Add vertices and edges to the graph. */
    auto v0 = graph.add_vertex();
    auto v1 = graph.add_vertex();
    auto v2 = graph.add_vertex();
    auto v3 = graph.add_vertex();
    auto e0 = graph.add_directed_edge(v0, v1);
    auto [e1, e2] = graph.add_undirected_edge(v1, v2);
    auto e3 = graph.add_directed_edge(v2, v3);
    auto [e4, e5] = graph.add_undirected_edge(v3, v0);

    {
        EXPECT_EQ(graph.get_num_vertices(), 4);
        EXPECT_EQ(graph.get_num_edges(), 6);
        EXPECT_EQ(v0, 0);
        EXPECT_EQ(v1, 1);
        EXPECT_EQ(v2, 2);
        EXPECT_EQ(v3, 3);
        EXPECT_EQ(e0, 0);
        EXPECT_EQ(e1, 1);
        EXPECT_EQ(e2, 2);
        EXPECT_EQ(e3, 3);
        EXPECT_EQ(e4, 4);
        EXPECT_EQ(e5, 5);
        // Non exhaustive test because it should work for others as well.
        EXPECT_EQ(graph.get_source<graphs::ForwardTag>(e0), v0);
        EXPECT_EQ(graph.get_source<graphs::BackwardTag>(e0), v1);
        EXPECT_EQ(graph.get_target<graphs::ForwardTag>(e0), v1);
        EXPECT_EQ(graph.get_target<graphs::BackwardTag>(e0), v0);
        // Ensure correct out- (Forward) and in- (Backward) degrees.
        EXPECT_EQ(graph.get_degree<graphs::ForwardTag>(v0), 2);
        EXPECT_EQ(graph.get_degree<graphs::ForwardTag>(v1), 1);
        EXPECT_EQ(graph.get_degree<graphs::ForwardTag>(v2), 2);
        EXPECT_EQ(graph.get_degree<graphs::ForwardTag>(v3), 1);
        EXPECT_EQ(graph.get_degree<graphs::BackwardTag>(v0), 1);
        EXPECT_EQ(graph.get_degree<graphs::BackwardTag>(v1), 2);
        EXPECT_EQ(graph.get_degree<graphs::BackwardTag>(v2), 1);
        EXPECT_EQ(graph.get_degree<graphs::BackwardTag>(v3), 2);

        /* Test that iterators yield correct values.
           Non exhaustive test because it should work for other vertices as well. */

        // VertexIndexIterator
        auto vertex_indices = graphs::VertexIndexSet(graph.get_vertex_indices().begin(), graph.get_vertex_indices().end());
        EXPECT_EQ(vertex_indices.size(), 4);
        EXPECT_TRUE(vertex_indices.contains(v0));
        EXPECT_TRUE(vertex_indices.contains(v1));
        EXPECT_TRUE(vertex_indices.contains(v2));
        EXPECT_TRUE(vertex_indices.contains(v3));

        // EdgeIndexIterator
        auto edge_indices = graphs::EdgeIndexSet(graph.get_edge_indices().begin(), graph.get_edge_indices().end());
        EXPECT_EQ(edge_indices.size(), 6);
        EXPECT_TRUE(edge_indices.contains(e0));
        EXPECT_TRUE(edge_indices.contains(e1));
        EXPECT_TRUE(edge_indices.contains(e2));
        EXPECT_TRUE(edge_indices.contains(e3));
        EXPECT_TRUE(edge_indices.contains(e4));
        EXPECT_TRUE(edge_indices.contains(e5));

        // AdjacentVertexIndexIterator
        auto v0_forward_adjacent_vertex_indices = graphs::VertexIndexSet(graph.get_adjacent_vertex_indices<graphs::ForwardTag>(v0).begin(),
                                                                         graph.get_adjacent_vertex_indices<graphs::ForwardTag>(v0).end());
        auto v0_backward_adjacent_vertex_indices = graphs::VertexIndexSet(graph.get_adjacent_vertex_indices<graphs::BackwardTag>(v0).begin(),
                                                                          graph.get_adjacent_vertex_indices<graphs::BackwardTag>(v0).end());

        EXPECT_EQ(v0_forward_adjacent_vertex_indices.size(), 2);
        EXPECT_TRUE(v0_forward_adjacent_vertex_indices.contains(v1));
        EXPECT_TRUE(v0_forward_adjacent_vertex_indices.contains(v3));

        EXPECT_EQ(v0_backward_adjacent_vertex_indices.size(), 1);
        EXPECT_TRUE(v0_backward_adjacent_vertex_indices.contains(v3));

        // AdjacentVertexIterator
        using VertexType = typename graphs::DynamicDigraph::VertexType;
        using VertexSetType = std::unordered_set<VertexType, loki::Hash<VertexType>, loki::EqualTo<VertexType>>;
        auto v0_foward_adjacent_vertices =
            VertexSetType(graph.get_adjacent_vertices<graphs::ForwardTag>(v0).begin(), graph.get_adjacent_vertices<graphs::ForwardTag>(v0).end());
        auto v0_backward_adjacent_vertices =
            VertexSetType(graph.get_adjacent_vertices<graphs::BackwardTag>(v0).begin(), graph.get_adjacent_vertices<graphs::BackwardTag>(v0).end());

        EXPECT_EQ(v0_foward_adjacent_vertices.size(), 2);
        EXPECT_TRUE(v0_foward_adjacent_vertices.contains(graph.get_vertices().at(v1)));
        EXPECT_TRUE(v0_foward_adjacent_vertices.contains(graph.get_vertices().at(v3)));

        EXPECT_EQ(v0_backward_adjacent_vertices.size(), 1);
        EXPECT_TRUE(v0_backward_adjacent_vertices.contains(graph.get_vertices().at(v3)));

        // AdjacentEdgeIndexIterator
        auto v0_forward_adjacent_edge_indices = graphs::EdgeIndexSet(graph.get_adjacent_edge_indices<graphs::ForwardTag>(v0).begin(),
                                                                     graph.get_adjacent_edge_indices<graphs::ForwardTag>(v0).end());
        auto v0_backward_adjacent_edge_indices = graphs::EdgeIndexSet(graph.get_adjacent_edge_indices<graphs::BackwardTag>(v0).begin(),
                                                                      graph.get_adjacent_edge_indices<graphs::BackwardTag>(v0).end());

        EXPECT_EQ(v0_forward_adjacent_edge_indices.size(), 2);
        EXPECT_TRUE(v0_forward_adjacent_edge_indices.contains(e0));
        EXPECT_TRUE(v0_forward_adjacent_edge_indices.contains(e5));

        EXPECT_EQ(v0_backward_adjacent_edge_indices.size(), 1);
        EXPECT_TRUE(v0_backward_adjacent_edge_indices.contains(e4));

        // AdjacentEdgeIterator
        using EdgeType = typename graphs::DynamicDigraph::EdgeType;
        using EdgeSetType = std::unordered_set<EdgeType, loki::Hash<EdgeType>, loki::EqualTo<EdgeType>>;
        auto v0_forward_adjacent_edge =
            EdgeSetType(graph.get_adjacent_edges<graphs::ForwardTag>(v0).begin(), graph.get_adjacent_edges<graphs::ForwardTag>(v0).end());
        auto v0_backward_adjacent_edge =
            EdgeSetType(graph.get_adjacent_edges<graphs::BackwardTag>(v0).begin(), graph.get_adjacent_edges<graphs::BackwardTag>(v0).end());

        EXPECT_EQ(v0_forward_adjacent_edge.size(), 2);
        EXPECT_TRUE(v0_forward_adjacent_edge.contains(graph.get_edge(e0)));
        EXPECT_TRUE(v0_forward_adjacent_edge.contains(graph.get_edge(e5)));

        EXPECT_EQ(v0_backward_adjacent_edge.size(), 1);
        EXPECT_TRUE(v0_backward_adjacent_edge.contains(graph.get_edge(e4)));
    }

    /**
     * StaticForwardDigraph
     */

    auto forward_graph = graphs::StaticForwardDigraph(graph);

    {
        EXPECT_EQ(forward_graph.get_num_vertices(), 4);
        EXPECT_EQ(forward_graph.get_num_edges(), 6);
        // Non exhaustive test because it should work for others as well.
        EXPECT_EQ(forward_graph.get_source<graphs::ForwardTag>(e0), v0);
        EXPECT_EQ(forward_graph.get_source<graphs::BackwardTag>(e0), v1);
        EXPECT_EQ(forward_graph.get_target<graphs::ForwardTag>(e0), v1);
        EXPECT_EQ(forward_graph.get_target<graphs::BackwardTag>(e0), v0);
        // Ensure correct out- (Forward) and in- (Backward) degrees.
        EXPECT_EQ(forward_graph.get_degree<graphs::ForwardTag>(v0), 2);
        EXPECT_EQ(forward_graph.get_degree<graphs::ForwardTag>(v1), 1);
        EXPECT_EQ(forward_graph.get_degree<graphs::ForwardTag>(v2), 2);
        EXPECT_EQ(forward_graph.get_degree<graphs::ForwardTag>(v3), 1);
        EXPECT_EQ(forward_graph.get_degree<graphs::BackwardTag>(v0), 1);
        EXPECT_EQ(forward_graph.get_degree<graphs::BackwardTag>(v1), 2);
        EXPECT_EQ(forward_graph.get_degree<graphs::BackwardTag>(v2), 1);
        EXPECT_EQ(forward_graph.get_degree<graphs::BackwardTag>(v3), 2);

        /* Test that iterators yield correct values.
           Non exhaustive test because it should work for other vertices as well. */

        // VertexIndexIterator
        auto vertex_indices = graphs::VertexIndexSet(forward_graph.get_vertex_indices().begin(), forward_graph.get_vertex_indices().end());
        EXPECT_EQ(vertex_indices.size(), 4);
        EXPECT_TRUE(vertex_indices.contains(v0));
        EXPECT_TRUE(vertex_indices.contains(v1));
        EXPECT_TRUE(vertex_indices.contains(v2));
        EXPECT_TRUE(vertex_indices.contains(v3));

        // EdgeIndexIterator
        auto edge_indices = graphs::EdgeIndexSet(forward_graph.get_edge_indices().begin(), forward_graph.get_edge_indices().end());
        EXPECT_EQ(edge_indices.size(), 6);
        EXPECT_TRUE(edge_indices.contains(e0));
        EXPECT_TRUE(edge_indices.contains(e1));
        EXPECT_TRUE(edge_indices.contains(e2));
        EXPECT_TRUE(edge_indices.contains(e3));
        EXPECT_TRUE(edge_indices.contains(e4));
        EXPECT_TRUE(edge_indices.contains(e5));

        // AdjacentVertexIndexIterator
        auto v0_forward_adjacent_vertex_indices = graphs::VertexIndexSet(forward_graph.get_adjacent_vertex_indices<graphs::ForwardTag>(v0).begin(),
                                                                         forward_graph.get_adjacent_vertex_indices<graphs::ForwardTag>(v0).end());
        auto v0_backward_adjacent_vertex_indices = graphs::VertexIndexSet(forward_graph.get_adjacent_vertex_indices<graphs::BackwardTag>(v0).begin(),
                                                                          forward_graph.get_adjacent_vertex_indices<graphs::BackwardTag>(v0).end());

        EXPECT_EQ(v0_forward_adjacent_vertex_indices.size(), 2);
        EXPECT_TRUE(v0_forward_adjacent_vertex_indices.contains(v1));
        EXPECT_TRUE(v0_forward_adjacent_vertex_indices.contains(v3));

        EXPECT_EQ(v0_backward_adjacent_vertex_indices.size(), 1);
        EXPECT_TRUE(v0_backward_adjacent_vertex_indices.contains(v3));

        // AdjacentVertexIterator
        using VertexType = typename graphs::DynamicDigraph::VertexType;
        using VertexSetType = std::unordered_set<VertexType, loki::Hash<VertexType>, loki::EqualTo<VertexType>>;
        auto v0_foward_adjacent_vertices = VertexSetType(forward_graph.get_adjacent_vertices<graphs::ForwardTag>(v0).begin(),
                                                         forward_graph.get_adjacent_vertices<graphs::ForwardTag>(v0).end());
        auto v0_backward_adjacent_vertices = VertexSetType(forward_graph.get_adjacent_vertices<graphs::BackwardTag>(v0).begin(),
                                                           forward_graph.get_adjacent_vertices<graphs::BackwardTag>(v0).end());

        EXPECT_EQ(v0_foward_adjacent_vertices.size(), 2);
        EXPECT_TRUE(v0_foward_adjacent_vertices.contains(forward_graph.get_vertices().at(v1)));
        EXPECT_TRUE(v0_foward_adjacent_vertices.contains(forward_graph.get_vertices().at(v3)));

        EXPECT_EQ(v0_backward_adjacent_vertices.size(), 1);
        EXPECT_TRUE(v0_backward_adjacent_vertices.contains(forward_graph.get_vertices().at(v3)));

        // AdjacentEdgeIndexIterator
        auto v0_forward_adjacent_edge_indices = graphs::EdgeIndexSet(forward_graph.get_adjacent_edge_indices<graphs::ForwardTag>(v0).begin(),
                                                                     forward_graph.get_adjacent_edge_indices<graphs::ForwardTag>(v0).end());
        auto v0_backward_adjacent_edge_indices = graphs::EdgeIndexSet(forward_graph.get_adjacent_edge_indices<graphs::BackwardTag>(v0).begin(),
                                                                      forward_graph.get_adjacent_edge_indices<graphs::BackwardTag>(v0).end());

        EXPECT_EQ(v0_forward_adjacent_edge_indices.size(), 2);
        EXPECT_TRUE(v0_forward_adjacent_edge_indices.contains(e0));
        EXPECT_TRUE(v0_forward_adjacent_edge_indices.contains(e5));

        EXPECT_EQ(v0_backward_adjacent_edge_indices.size(), 1);
        EXPECT_TRUE(v0_backward_adjacent_edge_indices.contains(e4));

        // AdjacentEdgeIterator
        using EdgeType = typename graphs::DynamicDigraph::EdgeType;
        using EdgeSetType = std::unordered_set<EdgeType, loki::Hash<EdgeType>, loki::EqualTo<EdgeType>>;
        auto v0_forward_adjacent_edge =
            EdgeSetType(forward_graph.get_adjacent_edges<graphs::ForwardTag>(v0).begin(), forward_graph.get_adjacent_edges<graphs::ForwardTag>(v0).end());
        auto v0_backward_adjacent_edge =
            EdgeSetType(forward_graph.get_adjacent_edges<graphs::BackwardTag>(v0).begin(), forward_graph.get_adjacent_edges<graphs::BackwardTag>(v0).end());

        EXPECT_EQ(v0_forward_adjacent_edge.size(), 2);
        EXPECT_TRUE(v0_forward_adjacent_edge.contains(forward_graph.get_edge(e0)));
        EXPECT_TRUE(v0_forward_adjacent_edge.contains(forward_graph.get_edge(e5)));

        EXPECT_EQ(v0_backward_adjacent_edge.size(), 1);
        EXPECT_TRUE(v0_backward_adjacent_edge.contains(forward_graph.get_edge(e4)));
    }

    /**
     * StaticBidirectionalDigraph
     */

    auto bidirectional_graph = graphs::StaticBidirectionalDigraph(graph);

    {
        EXPECT_EQ(bidirectional_graph.get_num_vertices(), 4);
        EXPECT_EQ(bidirectional_graph.get_num_edges(), 6);
        // Non exhaustive test because it should work for others as well.
        EXPECT_EQ(bidirectional_graph.get_source<graphs::ForwardTag>(e0), v0);
        EXPECT_EQ(bidirectional_graph.get_source<graphs::BackwardTag>(e0), v1);
        EXPECT_EQ(bidirectional_graph.get_target<graphs::ForwardTag>(e0), v1);
        EXPECT_EQ(bidirectional_graph.get_target<graphs::BackwardTag>(e0), v0);
        // Ensure correct out- (Forward) and in- (Backward) degrees.
        EXPECT_EQ(bidirectional_graph.get_degree<graphs::ForwardTag>(v0), 2);
        EXPECT_EQ(bidirectional_graph.get_degree<graphs::ForwardTag>(v1), 1);
        EXPECT_EQ(bidirectional_graph.get_degree<graphs::ForwardTag>(v2), 2);
        EXPECT_EQ(bidirectional_graph.get_degree<graphs::ForwardTag>(v3), 1);
        EXPECT_EQ(bidirectional_graph.get_degree<graphs::BackwardTag>(v0), 1);
        EXPECT_EQ(bidirectional_graph.get_degree<graphs::BackwardTag>(v1), 2);
        EXPECT_EQ(bidirectional_graph.get_degree<graphs::BackwardTag>(v2), 1);
        EXPECT_EQ(bidirectional_graph.get_degree<graphs::BackwardTag>(v3), 2);

        /* Test that iterators yield correct values.
           Non exhaustive test because it should work for other vertices as well. */

        // VertexIndexIterator
        auto vertex_indices = graphs::VertexIndexSet(bidirectional_graph.get_vertex_indices().begin(), bidirectional_graph.get_vertex_indices().end());
        EXPECT_EQ(vertex_indices.size(), 4);
        EXPECT_TRUE(vertex_indices.contains(v0));
        EXPECT_TRUE(vertex_indices.contains(v1));
        EXPECT_TRUE(vertex_indices.contains(v2));
        EXPECT_TRUE(vertex_indices.contains(v3));

        // EdgeIndexIterator
        auto edge_indices = graphs::EdgeIndexSet(bidirectional_graph.get_edge_indices().begin(), bidirectional_graph.get_edge_indices().end());
        EXPECT_EQ(edge_indices.size(), 6);
        EXPECT_TRUE(edge_indices.contains(e0));
        EXPECT_TRUE(edge_indices.contains(e1));
        EXPECT_TRUE(edge_indices.contains(e2));
        EXPECT_TRUE(edge_indices.contains(e3));
        EXPECT_TRUE(edge_indices.contains(e4));
        EXPECT_TRUE(edge_indices.contains(e5));

        // AdjacentVertexIndexIterator
        auto v0_forward_adjacent_vertex_indices = graphs::VertexIndexSet(bidirectional_graph.get_adjacent_vertex_indices<graphs::ForwardTag>(v0).begin(),
                                                                         bidirectional_graph.get_adjacent_vertex_indices<graphs::ForwardTag>(v0).end());
        auto v0_backward_adjacent_vertex_indices = graphs::VertexIndexSet(bidirectional_graph.get_adjacent_vertex_indices<graphs::BackwardTag>(v0).begin(),
                                                                          bidirectional_graph.get_adjacent_vertex_indices<graphs::BackwardTag>(v0).end());

        EXPECT_EQ(v0_forward_adjacent_vertex_indices.size(), 2);
        EXPECT_TRUE(v0_forward_adjacent_vertex_indices.contains(v1));
        EXPECT_TRUE(v0_forward_adjacent_vertex_indices.contains(v3));

        EXPECT_EQ(v0_backward_adjacent_vertex_indices.size(), 1);
        EXPECT_TRUE(v0_backward_adjacent_vertex_indices.contains(v3));

        // AdjacentVertexIterator
        using VertexType = typename graphs::DynamicDigraph::VertexType;
        using VertexSetType = std::unordered_set<VertexType, loki::Hash<VertexType>, loki::EqualTo<VertexType>>;
        auto v0_foward_adjacent_vertices = VertexSetType(bidirectional_graph.get_adjacent_vertices<graphs::ForwardTag>(v0).begin(),
                                                         bidirectional_graph.get_adjacent_vertices<graphs::ForwardTag>(v0).end());
        auto v0_backward_adjacent_vertices = VertexSetType(bidirectional_graph.get_adjacent_vertices<graphs::BackwardTag>(v0).begin(),
                                                           bidirectional_graph.get_adjacent_vertices<graphs::BackwardTag>(v0).end());

        EXPECT_EQ(v0_foward_adjacent_vertices.size(), 2);
        EXPECT_TRUE(v0_foward_adjacent_vertices.contains(bidirectional_graph.get_vertices().at(v1)));
        EXPECT_TRUE(v0_foward_adjacent_vertices.contains(bidirectional_graph.get_vertices().at(v3)));

        EXPECT_EQ(v0_backward_adjacent_vertices.size(), 1);
        EXPECT_TRUE(v0_backward_adjacent_vertices.contains(bidirectional_graph.get_vertices().at(v3)));

        // AdjacentEdgeIndexIterator
        auto v0_forward_adjacent_edge_indices = graphs::EdgeIndexSet(bidirectional_graph.get_adjacent_edge_indices<graphs::ForwardTag>(v0).begin(),
                                                                     bidirectional_graph.get_adjacent_edge_indices<graphs::ForwardTag>(v0).end());
        auto v0_backward_adjacent_edge_indices = graphs::EdgeIndexSet(bidirectional_graph.get_adjacent_edge_indices<graphs::BackwardTag>(v0).begin(),
                                                                      bidirectional_graph.get_adjacent_edge_indices<graphs::BackwardTag>(v0).end());

        EXPECT_EQ(v0_forward_adjacent_edge_indices.size(), 2);
        EXPECT_TRUE(v0_forward_adjacent_edge_indices.contains(e0));
        EXPECT_TRUE(v0_forward_adjacent_edge_indices.contains(e5));

        EXPECT_EQ(v0_backward_adjacent_edge_indices.size(), 1);
        EXPECT_TRUE(v0_backward_adjacent_edge_indices.contains(e4));

        // AdjacentEdgeIterator
        using EdgeType = typename graphs::DynamicDigraph::EdgeType;
        using EdgeSetType = std::unordered_set<EdgeType, loki::Hash<EdgeType>, loki::EqualTo<EdgeType>>;
        auto v0_forward_adjacent_edge = EdgeSetType(bidirectional_graph.get_adjacent_edges<graphs::ForwardTag>(v0).begin(),
                                                    bidirectional_graph.get_adjacent_edges<graphs::ForwardTag>(v0).end());
        auto v0_backward_adjacent_edge = EdgeSetType(bidirectional_graph.get_adjacent_edges<graphs::BackwardTag>(v0).begin(),
                                                     bidirectional_graph.get_adjacent_edges<graphs::BackwardTag>(v0).end());

        EXPECT_EQ(v0_forward_adjacent_edge.size(), 2);
        EXPECT_TRUE(v0_forward_adjacent_edge.contains(bidirectional_graph.get_edge(e0)));
        EXPECT_TRUE(v0_forward_adjacent_edge.contains(bidirectional_graph.get_edge(e5)));

        EXPECT_EQ(v0_backward_adjacent_edge.size(), 1);
        EXPECT_TRUE(v0_backward_adjacent_edge.contains(bidirectional_graph.get_edge(e4)));
    }
}

}
