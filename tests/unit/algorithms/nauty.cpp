#include "mimir/algorithms/nauty.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, AlgorithmsNautyDenseGraphConstructorEmptyTest)
{
    auto graph = nauty_wrapper::DenseGraph();

    EXPECT_ANY_THROW(graph.add_edge(0, 1));
    EXPECT_ANY_THROW(graph.add_vertex_coloring({ 0, 0 }));

    graph.reset(2);

    EXPECT_NO_THROW(graph.add_edge(0, 1));
    EXPECT_NO_THROW(graph.add_vertex_coloring({ 0, 0 }));
}

TEST(MimirTests, AlgorithmsNautySparseGraphConstructorEmptyTest)
{
    auto graph = nauty_wrapper::SparseGraph();

    EXPECT_ANY_THROW(graph.add_edge(0, 1));
    EXPECT_ANY_THROW(graph.add_vertex_coloring({ 0, 0 }));

    graph.reset(2);

    EXPECT_NO_THROW(graph.add_edge(0, 1));
    EXPECT_NO_THROW(graph.add_vertex_coloring({ 0, 0 }));
}

}
