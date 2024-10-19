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

// https://arxiv.org/pdf/1907.09582

// TODO: we need an effective way to compute neighbors of a k-tuple!

template<size_t k>
class WeisfeilerLeman
{
public:
    /// @brief Refine the colors of the unstable k-tuples.
    /// @tparam G
    /// @param graph
    /// @param in_out_unstable_vertex_k_tuples
    /// @param in_out_k_tuple_colors
    /// @return
    template<typename G>
    requires IsVertexListGraph<G> && IsIncidenceGraph<G>
    bool refine_coloring(const G& graph, std::vector<std::pair<Index, std::array<Index, k>>>& in_out_unstable_vertex_k_tuples, ColorMap& in_out_k_tuple_colors);

    /// @brief Compress the colors of each k-tuple of vertices into their (ordered) isomorphism type.
    /// @param vertex_k_tuples
    /// @param vertex_colors
    /// @return
    ColorList compute_initial_k_coloring(const std::vector<std::pair<Index, std::array<Index, k>>>& vertex_k_tuples, const ColorMap& vertex_colors);

    /// @brief Compute all ordered k-tuples of vertices of a given graph.
    /// @tparam G
    /// @param graph
    /// @return
    template<typename G>
    requires IsVertexListGraph<G> && IsIncidenceGraph<G>
    static std::vector<std::pair<Index, std::array<Index, k>>> compute_all_ordered_vertex_k_tuples(const G& graph);

private:
    // TODO: add mapping for ordered isomorphism types

    using Configuration = std::pair<std::array<Color, k>, std::vector<std::tuple<Index, Color, Count>>>;
    using CompressionFunction = std::unordered_map<Configuration, Color>;

    CompressionFunction m_f;
};

}

#endif