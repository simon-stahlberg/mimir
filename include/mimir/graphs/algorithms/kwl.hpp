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

#ifndef MIMIR_GRAPHS_ALGORITHMS_KWL_HPP_
#define MIMIR_GRAPHS_ALGORITHMS_KWL_HPP_

#include "mimir/graphs/graph_interface.hpp"

namespace mimir
{

/// @brief `WeisfeilerLeman` implements the k-dimensional Weisfeiler-Leman algorithm.
/// Source: https://arxiv.org/pdf/1907.09582
/// @tparam k is the dimensionality.
template<size_t k>
class WeisfeilerLeman
{
public:
    // The algorithm ensures lexicographically sorted k-tuples
    using KTuple = std::array<Index, k>;
    using KTupleToColor = std::unordered_map<KTuple, Color>;

    /// @brief Initialization step (lines 1-2)
    /// @tparam G is the type of the given graph.
    /// @param graph is the given graph.
    /// @param vertex_colors are the given vertex colors.
    /// @param out_k_coloring is the initial coloring of all ordered k-tuples.
    /// @param work_list is the set of unstable colors that require further refinement.
    template<typename G>
    requires IsVertexListGraph<G> && IsIncidenceGraph<G>
    void initialize(const G& graph, const ColorMap& vertex_colors, KTupleToColor& out_k_coloring, ColorSet& work_list);

    /// @brief Refinement step (lines 3-18)
    /// @tparam G is the type of the given graph.
    /// @param graph is the given graph.
    /// @param out_k_coloring is the current coloring of all ordered k-tuples.
    /// @param work_list is the set of unstable colors that require further refinement.
    template<typename G>
    requires IsVertexListGraph<G> && IsIncidenceGraph<G>
    void refine_coloring(const G& graph, KTupleToColor& out_k_coloring, ColorSet& work_list);

private:
    /* Compression used in line 16 to map (c_1^i, ...,c_k^i) = (c_1^j, ...,c_k^j) to the same integer color. */
    using KColorCompressionFunction = std::unordered_map<std::array<Color, k>, Color>;

    KColorCompressionFunction m_f;

    /* Compression used in line 18 to map (C(v), (c_1^1, ...,c_k^1), ..., (c_1^r, ...,c_k^r)) to an integer color */
    using ConfigurationCompressionFunction = std::unordered_map<std::pair<Color, ColorList>, Color>;

    ConfigurationCompressionFunction m_g;
};

}

#endif