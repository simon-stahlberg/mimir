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

#include "mimir/algorithms/nauty.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, AlgorithmsNautyDenseGraphConstructorEmptyTest)
{
    auto graph = nauty_wrapper::DenseGraph();

    EXPECT_ANY_THROW(graph.add_edge(0, 1));
    EXPECT_ANY_THROW(graph.add_vertex_coloring({ 0, 0 }));

    graph.clear(2);

    EXPECT_NO_THROW(graph.add_edge(0, 1));
    EXPECT_NO_THROW(graph.add_vertex_coloring({ 0, 0 }));
}

TEST(MimirTests, AlgorithmsNautySparseGraphConstructorEmptyTest)
{
    auto graph = nauty_wrapper::SparseGraph();

    EXPECT_ANY_THROW(graph.add_edge(0, 1));
    EXPECT_ANY_THROW(graph.add_vertex_coloring({ 0, 0 }));

    graph.clear(2);

    EXPECT_NO_THROW(graph.add_edge(0, 1));
    EXPECT_NO_THROW(graph.add_vertex_coloring({ 0, 0 }));
}

}
