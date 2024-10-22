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

#include "mimir/common/printers.hpp"
#include "mimir/graphs/digraph_vertex_colored.hpp"
#include "mimir/graphs/graph_interface.hpp"
#include "mimir/graphs/graph_traversal_interface.hpp"
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
/// @return the `ColorRefinementCerticate`
template<typename G>
    requires IsVertexListGraph<G> && IsIncidenceGraph<G> && IsVertexColoredGraph<G>  //
ColorRefinementCertificate compute_color_refinement_certificate(const G& graph)
{
    bool debug = false;

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
        max_color = std::max(max_color, get_color(vertex));
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
        if (debug)
            std::cout << "L: " << L << std::endl;

        // (lines 4-11): Create multiset of colors of neighbors of each vertex whose color must be updated.
        for (const auto& color : L)
        {
            for (const auto& vertex : color_to_vertices.at(color))
            {
                for (const auto& outgoing_vertex : graph.template get_adjacent_vertices<ForwardTraversal>(vertex))
                {
                    M.emplace_back(vertex, vertex_to_color.at(outgoing_vertex.get_index()));
                }
            }
        }

        // (line 12): Perform radix sort of M by vertex and colors (TODO radix sort)
        std::sort(M.begin(), M.end());

        if (debug)
            std::cout << "M: " << M << std::endl;

        // (line 13): Scan M and replace tuples (v,c1),...,(v,cr) with single tuple (C(v),c1,...,cr,v) to construct signatures.
        auto M_replaced = std::vector<std::tuple<Color, ColorList, Index>>();
        {
            // Subroutine to construct signatures.
            auto it = M.begin();
            while (it != M.end())
            {
                auto signature = ColorList();
                auto vertex = it->first;

                auto it2 = it;
                while (it2 != M.end() && it2->first == vertex)
                {
                    signature.push_back(it2->second);
                    ++it2;
                }
                it = it2;

                // Ensure canonical signature.
                assert(std::is_sorted(signature.begin(), signature.end()));
                M_replaced.emplace_back(vertex_to_color.at(vertex), std::move(signature), vertex);
            }
        }
        // (line 14): Perform radix sort of M by old color and neighborhood colors (TODO radix sort)
        std::sort(M_replaced.begin(), M_replaced.end());

        if (debug)
            std::cout << "M_replaced: " << M_replaced << std::endl;

        /* (line 15): Add new colors to work list */
        color_to_vertices.clear();
        L.clear();
        {
            // Subroutine to split color classes.
            auto it = M_replaced.begin();
            while (it != M_replaced.end())
            {
                const auto old_color = std::get<0>(*it);

                /* Check whether old color has split. */
                auto has_split = false;
                auto it2 = it;
                {
                    // Subroutine to detect split
                    const auto& signature = std::get<1>(*it);
                    while (it2 != M_replaced.end() && old_color == std::get<0>(*it2))
                    {
                        ++it2;

                        if (signature != std::get<1>(*it2))
                        {
                            has_split = true;
                            break;
                        }
                    }
                }
                if (has_split)
                {
                    /* Split old_color class. */
                    {
                        // Subroutine to split color class
                        while (it2 != M_replaced.end() && old_color == std::get<0>(*it2))
                        {
                            // Determine new color for (old_color, signature)
                            const auto& signature = std::get<1>(*it2);
                            auto new_color = next_color++;

                            // Add new color to work list.
                            L.insert(new_color);

                            // Add mapping to decoding table
                            auto result = f.emplace(std::make_pair(old_color, signature), new_color);
                            // Ensure that we are not overwritting table entries.
                            assert(result.second);

                            {
                                // Subroutine to assign new color to vertices with same signature.
                                while (it2 != M_replaced.end() && old_color == std::get<0>(*it2) && signature == std::get<1>(*it2))
                                {
                                    auto vertex = std::get<2>(*it2);
                                    vertex_to_color[vertex] = new_color;
                                    color_to_vertices[new_color].insert(vertex);
                                    ++it2;
                                }
                            }

                            // Ensure that we either finished or entered the next old color class.
                            assert(it2 == M_replaced.end() || old_color != std::get<0>(*it2));
                        }
                    }
                }

                /* Move to next old color class or the end. */
                it = it2;
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