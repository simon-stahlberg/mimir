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

#ifndef MIMIR_GRAPHS_ALGORITHMS_COLOR_REFINEMENT_HPP_
#define MIMIR_GRAPHS_ALGORITHMS_COLOR_REFINEMENT_HPP_

#include "mimir/graphs/graph_interface.hpp"

namespace mimir
{

/// @brief `ColorHistogram` stores a mapping of colors to the number of occurences in a canonical form.
class ColorHistogram
{
public:
    /// @brief Constructs the empty color histogram.
    ColorHistogram();

    /// @brief Constructs the color histogram for the given coloring.
    /// @param coloring is the given coloring.
    explicit ColorHistogram(const ColorList& coloring);

    /// @brief Initialize the color histogram for the given coloring.
    /// @param coloring is the given coloring.
    void initialize(const ColorList& coloring);

private:
    std::vector<std::pair<Color, Count>> m_data;
};

/// @brief `ColorRefinement` implements the color refinement step of the 1-dimensional Weisfeiler-Leman algorithm. A single instantiation can be used on several
/// graphs since the compression function is shared.
///
/// When handling several graphs, one should ideally use a divide-and-conquer approach where after each refinement iteration the graphs are partitioned such
/// that each partition only contains graphs with the same color histogram. This approach keeps the compression function manageable, as graphs in a partition
/// are indistinguishable, effectively keeping the size of the compression function small. The divide-and-conquer approach should then proceed in a
/// depth-first-manner.
class ColorRefinement
{
public:
    /// @brief Perform one iteration of color refinement on the given graph and the given current coloring.
    /// @tparam G is the type of the graph.
    /// @param graph is the given graph.
    /// @param in_out_unstable_vertices is the set of unstable vertices and should initially contain all vertex indices.
    /// @param in_out_vertex_colors is the given coloring of the graph and holds the coloring after the iteration.
    template<typename G>
    requires IsVertexListGraph<G> && IsIncidenceGraph<G>
    void refine_coloring(const G& graph, IndexSet& in_out_unstable_vertices, ColorMap& in_out_vertex_colors);

private:
    using Configuration = std::pair<Color, ColorHistogram>;
    using CompressionFunction = std::unordered_map<Configuration, Color>;

    CompressionFunction m_f;
};

}

#endif