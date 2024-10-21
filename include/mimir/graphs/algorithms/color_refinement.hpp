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

#include <queue>

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
/// Sources: https://arxiv.org/pdf/1907.09582
/// @tparam G is the vertex-colored graph.
/// @return the `ColorRefinementCerticate` that contains the canonical decoding table and the canonical vertex coloring.
template<typename G>
requires IsVertexListGraph<G> && IsIncidenceGraph<G> && IsVertexColoredGraph<G>  //
    ColorRefinementCertificate compute_color_refinement_certificate(const G& graph)
{
    using CompressionFunction = std::unordered_map<std::pair<Color, ColorList>, Color, Hash<std::pair<Color, ColorList>>>;

    auto f = CompressionFunction();

    // (line 1-2): Initialize vertex colors + some additional bookkeeping to work with dynamic graphs which might skip vertex indices.
    auto max_color = Color();
    auto vertex_to_color = IndexMap<Color>();
    auto color_to_vertices = ColorMap<IndexSet>();
    // (line 1-2): Initialize work list.
    auto L = ColorSet();
    for (const auto& vertex : graph.get_vertices())
    {
        max_color = max(max_color, get_color(vertex));
        vertex_to_color.emplace(vertex.get_index(), get_color(vertex));
        color_to_vertices[get_color(vertex)].insert(vertex.get_index());
        L.insert(get_color(vertex));
    }
    auto next_color = max_color + 1;

    // (line 1-2): Initialize multi set.
    auto M = std::vector<std::pair<Index, Color>>();
    // (line 3): Process work list until all vertex colors have stabilized.
    while (!L.empty())
    {
        // (lines 4-11): Create multiset of colors of neighbors of each vertex whose color must be updated.
        for (const auto& color : L)
        {
            for (const auto& vertex : color_to_vertices.at(color))
            {
                for (const auto& outgoing_vertex : graph.get_adjacent_vertices(vertex))
                {
                    M.emplace_back(outgoing_vertex.get_index(), vertex_to_color.at(outgoing_vertex));
                }
            }
        }

        // (line 12): Perform radix sort of M by vertex and colors (TODO radix sort)
        std::sort(M.begin(), M.end());

        // (line 13): Scan M and replace tuples (v,c1),...,(v,cr) with single tuple (C(v),c1,...,cr,v).
        auto M_replaced = std::vector<std::tuple<Color, ColorList, Index>>();
        auto it = M.begin();
        while (it != M.end())
        {
            auto colors = ColorList();
            auto vertex = it->first;

            auto it2 = it;
            while (it2 != M.end() && it2->first == vertex)
            {
                colors.push_back(it2->second);
                ++it2;
            }
            it = it2;

            assert(std::is_sorted(colors.begin(), colors.end()));
            M_replaced.emplace_back(vertex_to_color.at(vertex), std::move(colors), vertex);
        }

        // (line 14): Perform radix sort of M by old color and neighborhood colors (TODO radix sort)
        std::sort(M_replaced.begin(), M_replaced.end());

        /* (line 15): Add new colors to work list */
        color_to_vertices.clear();
        L.clear();
        for (auto& pair : M_replaced)
        {
            auto old_color = std::get<0>(pair);
            auto& neighbor_colors = std::get<1>(pair);
            auto vertex = std::get<2>(pair);

            auto result = f.emplace(std::make_pair(old_color, std::move(neighbor_colors)), next_color);
            auto new_color = result.first->second;
            auto inserted = result.second;
            if (inserted)
            {
                ++next_color;
                vertex_to_color[vertex] = new_color;
                color_to_vertices[new_color].insert(vertex);
                L.insert(new_color);
            }
        }
        /* (line 15): Clear multiset */
        M.clear();
    }

    /* Canonize the coloring function. */
    auto canonical_compression_function = ColorRefinementCertificate::CanonicalCompressionFunction();
    for (const auto& pair : f)
    {
        canonical_compression_function.emplace_back(pair.first.first, pair.first.second, pair.second);
    }
    std::sort(canonical_compression_function.begin(), canonical_compression_function.end());

    /* Canonize the final coloring. */
    auto canonical_coloring = ColorList();
    for (const auto& pair : vertex_to_color)
    {
        canonical_coloring.push_back(pair.second);
    }
    std::sort(canonical_coloring.begin(), canonical_coloring.end());

    /* Return the certificate */
    return ColorRefinementCertificate(std::move(canonical_compression_function), std::move(canonical_coloring));
}

}

template<>
struct std::hash<mimir::ColorRefinementCertificate>
{
    size_t operator()(const mimir::ColorRefinementCertificate& element) const;
};

#endif