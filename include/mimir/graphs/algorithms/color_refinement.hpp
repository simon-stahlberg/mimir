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
#include "mimir/graphs/graph_vertices.hpp"

namespace mimir
{

/// @brief `ColorRefinementCertificate` encapsulates the final coloring and the decoding table.
class ColorRefinementCertificate
{
public:
    using CompressionFunction = std::unordered_map<std::pair<Color, ColorList>, Color>;

    ColorRefinementCertificate(CompressionFunction f, ColorList coloring);

private:
    CompressionFunction m_f;
    ColorList m_coloring;
};

bool operator==(const ColorRefinementCertificate& lhs, const ColorRefinementCertificate& rhs);

/// @brief `compute_color_refinement_certificate` implements the color refinement algorithm.
/// Source: https://people.cs.umass.edu/~immerman/pub/opt.pdf
/// @tparam G is the vertex-colored graph.
/// @return
template<typename G>
requires IsVertexListGraph<G> && IsIncidenceGraph<G> && IsVertexColoredGraph<G>
extern ColorRefinementCertificate compute_color_refinement_certificate(const G& graph);

}

struct std::hash<mimir::ColorRefinementCertificate>
{
    size_t operator()(const mimir::ColorRefinementCertificate& element) const;
};

#endif