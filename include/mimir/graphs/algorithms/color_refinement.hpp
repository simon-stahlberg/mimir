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

using Signature = std::pair<ColorList, ColorList>;

/// @brief `Certificate` encapsulates the canonical coloring and the canonical compression function (decoding table).
class Certificate
{
public:
    using CompressionFunction = std::unordered_map<std::pair<Color, Signature>, Color, Hash<std::pair<Color, Signature>>>;

    using CanonicalCompressionFunction = std::map<std::pair<Color, Signature>, Color>;

    Certificate(CompressionFunction f, ColorList hash_to_color);

    const ColorList& get_hash_to_color() const;

    const CanonicalCompressionFunction& get_canonical_compression_function() const;
    const ColorList& get_canonical_coloring() const;

private:
    ColorList m_hash_to_color;

    CanonicalCompressionFunction m_f;
    ColorList m_canonical_coloring;
};

extern bool operator==(const Certificate& lhs, const Certificate& rhs);

extern std::ostream& operator<<(std::ostream& out, const Certificate& element);

/// @brief Replace tuples by grouping colors with same hash.
/// @tparam ColorType
/// @param M
/// @param hash_to_color
/// @param out_M_replaced
template<typename ColorType>
void replace_tuples(const std::vector<std::pair<Index, ColorType>>& M_outgoing,
                    const std::vector<std::pair<Index, ColorType>>& M_ingoing,
                    const ColorList& hash_to_color,
                    std::vector<std::tuple<Color, std::pair<std::vector<ColorType>, std::vector<ColorType>>, Index>>& out_M_replaced)
{
    using M_IteratorType = std::vector<std::pair<Index, ColorType>>::const_iterator;

    auto collect_color = [](Index hash, std::vector<ColorType>& signature, M_IteratorType& M_iter, M_IteratorType M_end)
    {
        auto it2 = M_iter;
        while (it2 != M_end && it2->first == hash)
        {
            signature.push_back(it2->second);
            ++it2;
        }
        M_iter = it2;
    };

    // Subroutine to construct signatures.
    auto it_outgoing = M_outgoing.begin();
    auto it_ingoing = M_ingoing.begin();

    while (it_outgoing != M_outgoing.end() || it_ingoing != M_ingoing.end())
    {
        const auto hash_outgoing = (it_outgoing != M_outgoing.end()) ? it_outgoing->first : std::numeric_limits<Index>::max();
        const auto hash_ingoing = (it_ingoing != M_ingoing.end()) ? it_ingoing->first : std::numeric_limits<Index>::max();

        auto signature_outgoing = std::vector<ColorType>();
        auto signature_ingoing = std::vector<ColorType>();

        if (hash_outgoing < hash_ingoing)  // only has outgoing
        {
            collect_color(hash_outgoing, signature_outgoing, it_outgoing, M_outgoing.end());
        }
        else if (hash_outgoing > hash_ingoing)  // only has ingoing
        {
            collect_color(hash_ingoing, signature_ingoing, it_ingoing, M_ingoing.end());
        }
        else  // has outgoing and ingoing
        {
            collect_color(hash_outgoing, signature_outgoing, it_outgoing, M_outgoing.end());
            collect_color(hash_ingoing, signature_ingoing, it_ingoing, M_ingoing.end());
        }

        // Ensure canonical signature.
        assert(std::is_sorted(signature_outgoing.begin(), signature_outgoing.end()));
        assert(std::is_sorted(signature_ingoing.begin(), signature_ingoing.end()));

        // In the k-FWL case, it might look strange to only consider either the hash_to_color_outgoing and ingoing.
        // The reason it works is that one isomorphic type implies the other.
        // TODO: double check that it works.
        out_M_replaced.emplace_back(hash_to_color.at(hash_outgoing),
                                    std::make_pair(std::move(signature_outgoing), std::move(signature_ingoing)),
                                    hash_outgoing);
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
void split_color_classes(const std::vector<std::tuple<Color, std::pair<std::vector<ColorType>, std::vector<ColorType>>, Index>>& M_replaced,
                         std::unordered_map<std::pair<Color, std::pair<std::vector<ColorType>, std::vector<ColorType>>>,
                                            Color,
                                            Hash<std::pair<Color, std::pair<std::vector<ColorType>, std::vector<ColorType>>>>>& ref_f,
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
                // Keep track of largest color class to stop refining it.
                auto largest_color_class_size = size_t(0);
                auto largest_color_class = Color(0);
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
                    if (current_color_class_size > largest_color_class_size)
                    {
                        largest_color_class_size = current_color_class_size;
                        largest_color_class = new_color;
                    }
                }
                // dont need to keep refining largest color class
                out_L.erase(largest_color_class);
            }
        }
    }
}

/// @brief `compute_certificate` implements the color refinement algorithm.
/// Sources: https://arxiv.org/pdf/1907.09582
/// @tparam G is the vertex-colored graph.
/// @return the `Certicate`
template<typename G>
requires IsVertexListGraph<G> && IsIncidenceGraph<G> && IsVertexColoredGraph<G>  //
    Certificate compute_certificate(const G& graph)
{
    // Toggle verbosity
    const bool debug = true;

    /* Fetch some data. */
    const auto num_vertices = graph.get_num_vertices();
    const auto is_undirected = is_undirected_graph(graph);

    if (debug)
        std::cout << "num_vertices=" << num_vertices << std::endl  //
                  << "is_undirected=" << is_undirected << std::endl;

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
    auto M_outgoing = std::vector<std::pair<Index, Color>>();
    auto M_ingoing = std::vector<std::pair<Index, Color>>();
    auto M_combined_replaced = std::vector<std::tuple<Color, Signature, Index>>();
    // (line 3): Process work list until all vertex colors have stabilized.

    while (!L.empty())
    {
        if (debug)
            std::cout << "L: " << L << std::endl;

        // Clear data structures that are reused.
        M_outgoing.clear();
        M_ingoing.clear();
        M_combined_replaced.clear();

        {
            // Subroutine to compute multiset M_outgoing and M_ingoing.
            for (size_t h = 0; h < num_vertices; ++h)
            {
                const auto vertex = hash_to_vertex.at(h);

                for (const auto& outgoing_vertex : graph.template get_adjacent_vertex_indices<ForwardTraversal>(vertex))
                {
                    const auto hash = vertex_to_hash.at(outgoing_vertex);

                    M_outgoing.emplace_back(h, hash_to_color.at(hash));
                }

                if (!is_undirected)
                {
                    for (const auto& ingoing_vertex : graph.template get_adjacent_vertex_indices<BackwardTraversal>(vertex))
                    {
                        const auto hash = vertex_to_hash.at(ingoing_vertex);

                        M_ingoing.emplace_back(h, hash_to_color.at(hash));
                    }
                }
            }
        }

        // (line 12): Perform radix sort of M by vertex and colors (TODO radix sort)
        std::sort(M_outgoing.begin(), M_outgoing.end());
        std::sort(M_ingoing.begin(), M_ingoing.end());

        if (debug)
            std::cout << "M_outgoing: " << M_outgoing << std::endl  //
                      << "M_ingoing: " << M_ingoing << std::endl;

        // (line 13): Scan M and replace tuples (v,c1),...,(v,cr) with single tuple (C(v),c1,...,cr,v) to construct signatures.
        replace_tuples(M_outgoing, M_ingoing, hash_to_color, M_combined_replaced);

        // (line 14): Perform radix sort of M by old color and neighborhood colors (TODO radix sort)
        std::sort(M_combined_replaced.begin(), M_combined_replaced.end());

        if (debug)
            std::cout << "M_combined_replaced: " << M_combined_replaced << std::endl;

        /* (line 15): Add new colors to work list */
        split_color_classes(M_combined_replaced, f, max_color, hash_to_color, color_to_hashes, L);
    }

    /* Report final neighborhood structures in the decoding table. */
    for (const auto& [old_color, signature, hash] : M_combined_replaced)
    {
        f.emplace(std::make_pair(old_color, signature), old_color);
    }

    /* Return the certificate */
    return Certificate(std::move(f), std::move(hash_to_color));
}
}

template<>
struct std::hash<mimir::color_refinement::Certificate>
{
    size_t operator()(const mimir::color_refinement::Certificate& element) const;
};

#endif