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
        auto graph = pymimir().attr("BasicStaticGraph")();
        const auto v0 = graph.attr("add_vertex")().cast<VertexIndex>();
        const auto v1 = graph.attr("add_vertex")().cast<VertexIndex>();

        EXPECT_ANY_THROW(graph.attr("add_vertex")(1, 2, 3));  ///< EmptyVertex expects zero arguments

        EXPECT_EQ(v0, 0);
        EXPECT_EQ(v1, 1);

        // Test immutable graph
        auto immutable_graph = pymimir().attr("BasicStaticGraphImmutable")(graph);
        EXPECT_ANY_THROW(immutable_graph.attr("add_vertex")().cast<VertexIndex>());
    }
}
