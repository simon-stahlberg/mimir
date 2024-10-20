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

#include "mimir/graphs/digraph_vertex_colored.hpp"
#include "mimir/graphs/graph_interface.hpp"
#include "mimir/graphs/graph_vertices.hpp"

namespace mimir
{

/// @brief `ColorRefinementCertificate` encapsulates the canonical coloring and the canonical compression function (decoding table).
class ColorRefinementCertificate
{
public:
    using CanonicalCompressionFunction = std::vector<std::tuple<Color, ColorList, Color>>;

    ColorRefinementCertificate(CanonicalCompressionFunction canonical_compression_function, ColorList canonical_coloring);

    const CanonicalCompressionFunction& get_canonical_compression_function() const;
    const ColorList& get_canonical_coloring() const;

private:
    CanonicalCompressionFunction m_canonical_compression_function;
    ColorList m_canonical_coloring;
};

extern bool operator==(const ColorRefinementCertificate& lhs, const ColorRefinementCertificate& rhs);

/// @brief `compute_color_refinement_certificate` implements the color refinement algorithm.
/// Source: https://people.cs.umass.edu/~immerman/pub/opt.pdf
/// @tparam G is the vertex-colored graph.
/// @return
template<typename G>
requires IsVertexListGraph<G> && IsIncidenceGraph<G> && IsVertexColoredGraph<G>  //
    ColorRefinementCertificate compute_color_refinement_certificate(const G& graph)
{
    using CompressionFunction = std::unordered_map<std::pair<Color, ColorList>, Color, Hash<std::pair<Color, ColorList>>>;

    auto f = CompressionFunction();

    auto coloring = ColorMap();
    for (const auto& vertex : graph.get_vertices())
    {
        coloring.emplace(vertex.get_index(), get_color(vertex));
    }

    /* TODO: implement color-refinement. */

    /* Canonize the coloring function. */
    auto canonical_compression_function = ColorRefinementCertificate::CanonicalCompressionFunction();
    for (const auto& pair : f)
    {
        canonical_compression_function.emplace_back(pair.first.first, pair.first.second, pair.second);
    }
    std::sort(canonical_compression_function.begin(), canonical_compression_function.end());

    /* Canonize the final coloring. */
    std::sort(coloring.begin(), coloring.end());

    /* Return the certificate */
    return ColorRefinementCertificate(std::move(canonical_compression_function), coloring);
}

}

template<>
struct std::hash<mimir::ColorRefinementCertificate>
{
    size_t operator()(const mimir::ColorRefinementCertificate& element) const;
};

#endif