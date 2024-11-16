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

#ifndef MIMIR_GRAPHS_ALGORITHMS_COLOR_REFINEMENT_IMPL_HPP_
#define MIMIR_GRAPHS_ALGORITHMS_COLOR_REFINEMENT_IMPL_HPP_

#include "mimir/common/equal_to.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/common/types.hpp"
#include "mimir/graphs/digraph_vertex_colored.hpp"
#include "mimir/graphs/graph_interface.hpp"
#include "mimir/graphs/graph_properties.hpp"
#include "mimir/graphs/graph_traversal_interface.hpp"
#include "mimir/graphs/graph_vertices.hpp"

#include <cassert>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>

namespace mimir::color_refinement
{
using mimir::operator<<;

/// @brief Replace tuples by grouping colors with same hash.
/// @tparam ColorType
/// @param M
/// @param hash_to_color
/// @param out_M_replaced
template<typename ColorType>
void replace_tuples(const std::vector<std::pair<Index, ColorType>>& M,
                    const ColorList& hash_to_color,
                    std::vector<std::tuple<Color, std::vector<ColorType>, Index>>& out_M_replaced)
{
    // Subroutine to construct signatures.
    auto it = M.begin();
    while (it != M.end())
    {
        auto signature = std::vector<ColorType>();
        const auto hash = it->first;

        auto it2 = it;
        while (it2 != M.end() && it2->first == hash)
        {
            signature.push_back(it2->second);
            ++it2;
        }
        it = it2;

        // Ensure canonical signature.
        assert(std::is_sorted(signature.begin(), signature.end()));
        out_M_replaced.emplace_back(hash_to_color.at(hash), std::move(signature), hash);
    }
}

/// @brief Split the color classes into new colors.
/// @tparam ColorType is Color for color-refinement and ColorArray<K> for k-FWL.
/// @param M_replaced
/// @param ref_f
/// @param ref_max_color
/// @param ref_hash_to_color
/// @param out_color_to_hashes
/// @param out_L
template<typename ColorType>
void split_color_classes(const std::vector<std::tuple<Color, std::vector<ColorType>, Index>>& M_replaced,
                         std::unordered_map<std::pair<Color, std::vector<ColorType>>, Color, Hash<std::pair<Color, std::vector<ColorType>>>>& ref_f,
                         Color& ref_max_color,
                         ColorList& ref_hash_to_color,
                         ColorMap<IndexList>& out_color_to_hashes,
                         ColorSet& out_L)
{
    out_color_to_hashes.clear();
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
                if (signature != std::get<1>(*it2))
                {
                    has_split = true;
                    break;
                }
                ++it2;
            }
        }

        if (!has_split)
        {
            /* Move to next old color class or the end. */
            it = it2;
        }
        else
        {
            /* Split old_color class. */
            {
                while (it != M_replaced.end() && old_color == std::get<0>(*it))
                {
                    auto current_color_class_size = size_t(1);
                    // Determine new color for (old_color, signature)
                    const auto& signature = std::get<1>(*it);
                    const auto new_color = ++ref_max_color;

                    // Add new color to work list.
                    out_L.insert(new_color);

                    // Add mapping to decoding table
                    [[maybe_unused]] const auto result = ref_f.emplace(std::make_pair(old_color, signature), new_color);
                    // Ensure that we are not overwritting table entries.
                    assert(result.second);

                    {
                        // Subroutine to assign new color to vertices with same signature.
                        while (it != M_replaced.end() && old_color == std::get<0>(*it) && signature == std::get<1>(*it))
                        {
                            ++current_color_class_size;
                            auto hash = std::get<2>(*it);
                            ref_hash_to_color[hash] = new_color;
                            out_color_to_hashes[new_color].push_back(hash);
                            ++it;
                        }
                    }
                }
            }
        }
    }
}

template<typename G>
requires IsVertexListGraph<G> && IsIncidenceGraph<G> && IsVertexColoredGraph<G>  //
    Certificate compute_certificate(const G& graph)
{
    if (!is_undirected_graph(graph))
    {
        throw std::runtime_error("Color-refinement does not support directed graphs because they can be translated into undirected graphs by introducing two "
                                 "vertices along the edge with different colors to encode the direction.");
    }

    // Toggle verbosity
    const bool debug = false;

    /* Fetch some data. */
    const auto num_vertices = graph.get_num_vertices();

    // (line 1-2): Initialize vertex colors and perfect hashes for vertex indices.
    auto vertex_to_hash = IndexMap<Index>();
    auto hash_to_vertex = IndexMap<Index>();
    auto max_color = Color();
    auto hash_to_color = ColorList(num_vertices);
    auto color_to_hashes = ColorMap<IndexList>();
    // (line 1-2): Initialize work list.
    auto L = ColorSet();
    for (const auto& vertex : graph.get_vertices())
    {
        const auto hash = vertex_to_hash.size();
        vertex_to_hash.emplace(vertex.get_index(), hash);
        hash_to_vertex.emplace(hash, vertex.get_index());

        const auto color = get_color(vertex);
        max_color = std::max(max_color, color);
        hash_to_color[hash] = color;
        color_to_hashes[color].push_back(hash);
        L.insert(color);
    }

    // (line 1-2): Initialize multi set.
    auto f = Certificate::CompressionFunction();
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
            // Subroutine to compute multiset M.
            // Note: this computes the stable coloring, not the coarsest stable coloring.
            for (size_t h = 0; h < num_vertices; ++h)
            {
                for (const auto& outgoing_vertex : graph.template get_adjacent_vertices<ForwardTraversal>(hash_to_vertex.at(h)))
                {
                    const auto hash = vertex_to_hash.at(outgoing_vertex.get_index());

                    M.emplace_back(h, hash_to_color.at(hash));
                }
            }
        }

        // (line 12): Perform radix sort of M by vertex and colors (TODO radix sort)
        std::sort(M.begin(), M.end());

        if (debug)
            std::cout << "M: " << M << std::endl;

        // (line 13): Scan M and replace tuples (v,c1),...,(v,cr) with single tuple (C(v),c1,...,cr,v) to construct signatures.
        replace_tuples(M, hash_to_color, M_replaced);

        // (line 14): Perform radix sort of M by old color and neighborhood colors (TODO radix sort)
        std::sort(M_replaced.begin(), M_replaced.end());

        if (debug)
            std::cout << "M_replaced: " << M_replaced << std::endl;

        /* (line 15): Add new colors to work list */
        split_color_classes(M_replaced, f, max_color, hash_to_color, color_to_hashes, L);
    }

    /* Report final neighborhood structures in the decoding table. */
    for (const auto& [old_color, signature, hash] : M_replaced)
    {
        f.emplace(std::make_pair(old_color, signature), old_color);
    }

    /* Return the certificate */
    return Certificate(std::move(f), std::move(hash_to_color));
}
}

#endif