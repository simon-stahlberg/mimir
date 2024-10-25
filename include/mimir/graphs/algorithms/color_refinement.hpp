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

#include "mimir/common/equal_to.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/common/types.hpp"
#include "mimir/graphs/digraph_vertex_colored.hpp"
#include "mimir/graphs/graph_interface.hpp"
#include "mimir/graphs/graph_traversal_interface.hpp"
#include "mimir/graphs/graph_vertices.hpp"

#include <cassert>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>

namespace mimir::color_refinement
{

/// @brief `Certificate` encapsulates the canonical coloring and the canonical compression function (decoding table).
class Certificate
{
public:
    using CompressionFunction = std::unordered_map<std::pair<Color, ColorList>, Color, Hash<std::pair<Color, ColorList>>>;

    using CanonicalCompressionFunction = std::map<std::pair<Color, ColorList>, Color>;
    using CanonicalColoring = std::set<Color>;

    Certificate(CompressionFunction compression_function, IndexMap<Color> vertex_to_color);

    const IndexMap<Color>& get_vertex_to_color() const;

    const CanonicalCompressionFunction& get_canonical_compression_function() const;
    const CanonicalColoring& get_canonical_coloring() const;

private:
    IndexMap<Color> m_vertex_to_color;

    CanonicalCompressionFunction m_canonical_compression_function;
    CanonicalColoring m_canonical_coloring;
};

extern bool operator==(const Certificate& lhs, const Certificate& rhs);

template<typename T>
concept IsIndexToColorMapping = requires(T a, Index i)
{
    {
        a.at(i)
        } -> std::convertible_to<Color>;
};

template<typename ColorType, IsIndexToColorMapping IndexToColor>
void replace_tuples(const std::vector<std::pair<Index, ColorType>>& M,
                    const IndexToColor& index_to_color,
                    std::vector<std::tuple<Color, std::vector<ColorType>, Index>>& out_M_replaced)
{
    // Subroutine to construct signatures.
    auto it = M.begin();
    while (it != M.end())
    {
        auto signature = std::vector<ColorType>();
        const auto vertex = it->first;

        auto it2 = it;
        while (it2 != M.end() && it2->first == vertex)
        {
            signature.push_back(it2->second);
            ++it2;
        }
        it = it2;

        // Ensure canonical signature.
        assert(std::is_sorted(signature.begin(), signature.end()));
        out_M_replaced.emplace_back(index_to_color.at(vertex), std::move(signature), vertex);
    }
}

/// @brief Split the color classes into new colors.
/// @tparam ColorType is Color for color-refinement and ColorArray<K> for k-FWL.
/// @tparam IndexToColor is a mapping from index to color where index is vertex for color-refinement or hash of k-tuple for k-FWL.
/// TODO: template parameters can go if we make the following changes:
/// 1) color refinement must map vertices to indexing schema 0,1,2,...
/// 2) k-FWL must compress colors of k-tuples.
/// @param M_replaced
/// @param ref_f
/// @param ref_max_color
/// @param ref_index_to_color
/// @param out_color_to_indices
/// @param out_L
template<typename ColorType, IsIndexToColorMapping IndexToColor>
void split_color_classes(const std::vector<std::tuple<Color, std::vector<ColorType>, Index>>& M_replaced,
                         std::unordered_map<std::pair<Color, std::vector<ColorType>>, Color, Hash<std::pair<Color, std::vector<ColorType>>>>& ref_f,
                         Color& ref_max_color,
                         IndexToColor& ref_index_to_color,
                         ColorMap<IndexList>& out_color_to_indices,
                         ColorSet& out_L)
{
    out_color_to_indices.clear();
    out_L.clear();

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
                    const auto new_color = ++ref_max_color;

                    // Add new color to work list.
                    out_L.insert(new_color);

                    // Add mapping to decoding table
                    const auto result = ref_f.emplace(std::make_pair(old_color, signature), new_color);
                    // Ensure that we are not overwritting table entries.
                    assert(result.second);

                    {
                        // Subroutine to assign new color to vertices with same signature.
                        while (it2 != M_replaced.end() && old_color == std::get<0>(*it2) && signature == std::get<1>(*it2))
                        {
                            auto vertex = std::get<2>(*it2);
                            ref_index_to_color[vertex] = new_color;
                            out_color_to_indices[new_color].push_back(vertex);
                            ++it2;
                        }
                    }

                    // Ensure that we either finished or entered the next color class.
                    assert(it2 == M_replaced.end() || old_color != std::get<0>(*it2) || signature != std::get<1>(*it2));
                }
            }
        }

        /* Move to next old color class or the end. */
        it = it2;
    }
}

/// @brief `compute_certificate` implements the color refinement algorithm.
/// Sources: https://arxiv.org/pdf/1907.09582
/// @tparam G is the vertex-colored graph.
/// @return the `ColorRefinementCerticate`
template<typename G>
requires IsVertexListGraph<G> && IsIncidenceGraph<G> && IsVertexColoredGraph<G>  //
    Certificate compute_certificate(const G& graph)
{
    // Toggle verbosity
    const bool debug = false;

    // Decoding table.
    auto f = Certificate::CompressionFunction();

    // (line 1-2): Initialize vertex colors + some additional bookkeeping to work with dynamic graphs which might skip vertex indices.
    auto max_color = Color();
    auto vertex_to_color = IndexMap<Color>();
    auto color_to_vertices = ColorMap<IndexList>();
    // (line 1-2): Initialize work list.
    auto L = ColorSet();
    for (const auto& vertex : graph.get_vertices())
    {
        max_color = std::max(max_color, get_color(vertex));
        vertex_to_color.emplace(vertex.get_index(), get_color(vertex));
        color_to_vertices[get_color(vertex)].push_back(vertex.get_index());
        L.insert(get_color(vertex));
    }

    // (line 1-2): Initialize multi set.
    auto M = std::vector<std::pair<Index, Color>>();
    auto M_replaced = std::vector<std::tuple<Color, ColorList, Index>>();
    // (line 3): Process work list until all vertex colors have stabilized.
    while (!L.empty())
    {
        if (debug)
            std::cout << "L: " << L << std::endl;

        // Clear data structures that are reused.
        M.clear();
        M_replaced.clear();

        {
            // (lines 4-11): Subroutine to fill multiset of colors of neighbors of each vertex whose color must be updated.
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
        }

        // (line 12): Perform radix sort of M by vertex and colors (TODO radix sort)
        std::sort(M.begin(), M.end());

        if (debug)
            std::cout << "M: " << M << std::endl;

        // (line 13): Scan M and replace tuples (v,c1),...,(v,cr) with single tuple (C(v),c1,...,cr,v) to construct signatures.
        replace_tuples(M, vertex_to_color, M_replaced);

        // (line 14): Perform radix sort of M by old color and neighborhood colors (TODO radix sort)
        std::sort(M_replaced.begin(), M_replaced.end());

        if (debug)
            std::cout << "M_replaced: " << M_replaced << std::endl;

        /* (line 15): Add new colors to work list */
        split_color_classes(M_replaced, f, max_color, vertex_to_color, color_to_vertices, L);
    }

    /* Return the certificate */
    return Certificate(std::move(f), std::move(vertex_to_color));
}
}

template<>
struct std::hash<mimir::color_refinement::Certificate>
{
    size_t operator()(const mimir::color_refinement::Certificate& element) const;
};

#endif