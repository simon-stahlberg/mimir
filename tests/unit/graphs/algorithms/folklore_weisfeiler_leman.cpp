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

#include "mimir/graphs/algorithms/folklore_weisfeiler_leman.hpp"

#include "mimir/graphs/concrete/vertex_colored_graph.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, GraphsAlgorithmsFolkloreWeisfeilerLemanTest)
{
    {
        /* The famous two triangle / sixgon example */
        auto iso_type_function = graphs::kfwl::IsomorphismTypeCompressionFunction();
        auto two_triangle = graphs::StaticVertexColoredGraph();
        {
            auto v1 = two_triangle.add_vertex(graphs::Color(graphs::VariadicColor(0)));
            auto v2 = two_triangle.add_vertex(graphs::Color(graphs::VariadicColor(0)));
            auto v3 = two_triangle.add_vertex(graphs::Color(graphs::VariadicColor(0)));
            auto v4 = two_triangle.add_vertex(graphs::Color(graphs::VariadicColor(0)));
            auto v5 = two_triangle.add_vertex(graphs::Color(graphs::VariadicColor(0)));
            auto v6 = two_triangle.add_vertex(graphs::Color(graphs::VariadicColor(0)));
            two_triangle.add_undirected_edge(v1, v2);
            two_triangle.add_undirected_edge(v2, v3);
            two_triangle.add_undirected_edge(v3, v1);
            two_triangle.add_undirected_edge(v4, v5);
            two_triangle.add_undirected_edge(v5, v6);
            two_triangle.add_undirected_edge(v6, v4);
        }
        auto two_triangle_certificate = *graphs::kfwl::compute_certificate<2>(two_triangle, iso_type_function);

        auto sixgon = graphs::StaticVertexColoredGraph();
        {
            auto v1 = sixgon.add_vertex(graphs::Color(graphs::VariadicColor(0)));
            auto v2 = sixgon.add_vertex(graphs::Color(graphs::VariadicColor(0)));
            auto v3 = sixgon.add_vertex(graphs::Color(graphs::VariadicColor(0)));
            auto v4 = sixgon.add_vertex(graphs::Color(graphs::VariadicColor(0)));
            auto v5 = sixgon.add_vertex(graphs::Color(graphs::VariadicColor(0)));
            auto v6 = sixgon.add_vertex(graphs::Color(graphs::VariadicColor(0)));
            sixgon.add_undirected_edge(v1, v2);
            sixgon.add_undirected_edge(v2, v3);
            sixgon.add_undirected_edge(v3, v4);
            sixgon.add_undirected_edge(v4, v5);
            sixgon.add_undirected_edge(v5, v6);
            sixgon.add_undirected_edge(v6, v1);
        }
        auto sixgon_certificate = *graphs::kfwl::compute_certificate<2>(sixgon, iso_type_function);

        EXPECT_NE(two_triangle_certificate, sixgon_certificate);
    }

    {
        /* 2-vertex graphs where the order of colors is flipped to test canonical decoding table. */
        auto iso_type_function = graphs::kfwl::IsomorphismTypeCompressionFunction();
        auto line_graph_1 = graphs::StaticVertexColoredGraph();
        {
            auto v1 = line_graph_1.add_vertex(graphs::Color(graphs::VariadicColor(1)));
            auto v2 = line_graph_1.add_vertex(graphs::Color(graphs::VariadicColor(0)));
            line_graph_1.add_undirected_edge(v1, v2);
        }
        auto line_graph_1_certificate = *graphs::kfwl::compute_certificate<2>(line_graph_1, iso_type_function);

        auto line_graph_2 = graphs::StaticVertexColoredGraph();
        {
            auto v1 = line_graph_2.add_vertex(graphs::Color(graphs::VariadicColor(0)));
            auto v2 = line_graph_2.add_vertex(graphs::Color(graphs::VariadicColor(1)));
            line_graph_2.add_undirected_edge(v1, v2);
        }
        auto line_graph_2_certificate = *graphs::kfwl::compute_certificate<2>(line_graph_2, iso_type_function);

        EXPECT_EQ(line_graph_1_certificate, line_graph_2_certificate);
    }
}
}