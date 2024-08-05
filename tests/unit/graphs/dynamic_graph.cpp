#include "mimir/graphs/digraph.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, GraphsDynamicDigraphTest)
{
    auto graph = DynamicDigraph();

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
    EXPECT_EQ(graph.get_source<ForwardTraversal>(e0), v0);
    EXPECT_EQ(graph.get_source<BackwardTraversal>(e0), v1);
    EXPECT_EQ(graph.get_target<ForwardTraversal>(e0), v1);
    EXPECT_EQ(graph.get_target<BackwardTraversal>(e0), v0);

    /* Remove a vertex including its three adjacent edges and a single edge. */
    graph.remove_vertex(v0);
    graph.remove_edge(e1);

    EXPECT_EQ(graph.get_num_vertices(), 3);
    EXPECT_EQ(graph.get_num_edges(), 2);
    EXPECT_TRUE(graph.get_vertices().contains(v1));
    EXPECT_TRUE(graph.get_vertices().contains(v2));
    EXPECT_TRUE(graph.get_vertices().contains(v3));
    EXPECT_FALSE(graph.get_edges().contains(e0));
    EXPECT_FALSE(graph.get_edges().contains(e1));
    EXPECT_TRUE(graph.get_edges().contains(e2));
    EXPECT_TRUE(graph.get_edges().contains(e3));
    EXPECT_FALSE(graph.get_edges().contains(e4));
    EXPECT_FALSE(graph.get_edges().contains(e5));

    /* Add two more vertices resulting in a reuse from the free vertex list, and an index obtained from next_vertex_index. */
    auto v4 = graph.add_vertex();
    auto v5 = graph.add_vertex();

    EXPECT_EQ(graph.get_num_vertices(), 5);
    EXPECT_EQ(v4, v0);
    EXPECT_EQ(v5, 4);

    /* Add five more edges resulting in a reuse from the free edge list, and an index obtained from next_edge_index. */
    auto [e6, e7] = graph.add_undirected_edge(v1, v5);
    auto [e8, e9] = graph.add_undirected_edge(v2, v5);
    auto e10 = graph.add_directed_edge(v3, v5);

    EXPECT_EQ(graph.get_num_edges(), 7);
    EXPECT_EQ(e6, e1);
    EXPECT_EQ(e7, e4);
    EXPECT_EQ(e8, e0);
    EXPECT_EQ(e9, e5);
    EXPECT_EQ(e10, 6);
    // Ensure that edge from free list has correct source and target.
    EXPECT_EQ(graph.get_source<ForwardTraversal>(e6), v1);
    EXPECT_EQ(graph.get_source<BackwardTraversal>(e6), v5);
    EXPECT_EQ(graph.get_target<ForwardTraversal>(e6), v5);
    EXPECT_EQ(graph.get_target<BackwardTraversal>(e6), v1);

    /* Test that iterators yield correct values. */
    auto vertex_indices = VertexIndexSet(graph.get_vertex_indices().begin(), graph.get_vertex_indices().end());
    EXPECT_EQ(vertex_indices.size(), 5);
    EXPECT_TRUE(vertex_indices.contains(v1));
    EXPECT_TRUE(vertex_indices.contains(v2));
    EXPECT_TRUE(vertex_indices.contains(v3));
    EXPECT_TRUE(vertex_indices.contains(v4));
    EXPECT_TRUE(vertex_indices.contains(v5));

    auto edge_indices = EdgeIndexSet(graph.get_edge_indices().begin(), graph.get_edge_indices().end());
    EXPECT_EQ(edge_indices.size(), 7);
    EXPECT_TRUE(edge_indices.contains(e2));
    EXPECT_TRUE(edge_indices.contains(e3));
    EXPECT_TRUE(edge_indices.contains(e6));
    EXPECT_TRUE(edge_indices.contains(e7));
    EXPECT_TRUE(edge_indices.contains(e8));
    EXPECT_TRUE(edge_indices.contains(e9));
    EXPECT_TRUE(edge_indices.contains(e10));

    auto v1_forward_adjacent_edge_indices =
        EdgeIndexSet(graph.get_adjacent_edge_indices<ForwardTraversal>(v1).begin(), graph.get_adjacent_edge_indices<ForwardTraversal>(v1).end());
    auto v1_backward_adjacent_edge_indices =
        EdgeIndexSet(graph.get_adjacent_edge_indices<BackwardTraversal>(v1).begin(), graph.get_adjacent_edge_indices<BackwardTraversal>(v1).end());
}

}
