#include "fixture.hpp"

#include <gtest/gtest.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

using namespace mimir;

constexpr std::string_view project_dir = "../../../";

TEST_F(PymimirFixture, static_graphs)
{
    {
        // Test mutable graph
        auto graph = pymimir().attr("EmptyStaticGraph")();
        const auto v0 = graph.attr("add_vertex")().cast<VertexIndex>();

        EXPECT_EQ(v0, 0);

        const auto v1 = graph.attr("add_vertex")().cast<VertexIndex>();

        EXPECT_EQ(v1, 1);

        EXPECT_ANY_THROW(graph.attr("add_vertex")(1, 2, 3));  ///< EmptyVertex expects zero arguments

        EXPECT_EQ(graph.attr("get_num_vertices")(), 2);

        // Test immutable graph
        auto immutable_graph = pymimir().attr("ImmutableEmptyStaticGraph")(graph);

        EXPECT_ANY_THROW(immutable_graph.attr("add_vertex")().cast<VertexIndex>());

        EXPECT_EQ(immutable_graph.attr("get_num_vertices")(), 2);

        // Test translation
        auto forward_graph = pymimir().attr("ForwardEmptyStaticGraph")(immutable_graph);

        EXPECT_EQ(forward_graph.attr("get_num_vertices")(), 2);
    }
}
