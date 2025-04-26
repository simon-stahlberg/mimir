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
#include "mimir/graphs/color.hpp"
#include "mimir/graphs/graph_interface.hpp"
#include "mimir/graphs/graph_properties.hpp"
#include "mimir/graphs/graph_traversal_interface.hpp"
#include "mimir/graphs/graph_vertices.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>
#include <map>
#include <unordered_map>
#include <vector>

namespace mimir::graphs::color_refinement
{

/// @brief `CertificateImpl` encapsulates the canonical coloring and the canonical compression function (decoding table).
class CertificateImpl
{
public:
    using CanonicalColorCompressionFunction = std::map<Color, ColorIndex>;

    using ConfigurationCompressionFunction = UnorderedMap<std::pair<ColorIndex, ColorIndexList>, ColorIndex>;
    using CanonicalConfigurationCompressionFunction = std::map<std::pair<ColorIndex, ColorIndexList>, ColorIndex>;

    CertificateImpl(CanonicalColorCompressionFunction c, ConfigurationCompressionFunction f, ColorIndexList hash_to_color);

    const CanonicalColorCompressionFunction& get_canonical_color_compression_function() const;
    const CanonicalConfigurationCompressionFunction& get_canonical_configuration_compression_function() const;
    const ColorIndexList& get_hash_to_color() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const
    {
        return std::tuple(std::cref(get_canonical_configuration_compression_function()), std::cref(get_canonical_configuration_compression_function()));
    }

private:
    CanonicalColorCompressionFunction m_c;
    CanonicalConfigurationCompressionFunction m_f;

    ColorIndexList m_hash_to_color;  ///< Is essentially vertex index to color but name is kept consistent with kfwl.
};

/// @brief Compare two certificates for equality.
/// @param lhs is the first certificate.
/// @param rhs is the second certificate.
/// @return Return true iff both certificates are equal.
extern bool operator==(const CertificateImpl& lhs, const CertificateImpl& rhs);

/// @brief Print a certificate to the ostream.
/// @param out is the ostream.
/// @param element is the certificate.
/// @return a reference to the ostream.
extern std::ostream& operator<<(std::ostream& out, const CertificateImpl& element);

/// @brief `compute_certificate` implements the color refinement algorithm.
/// Sources: https://arxiv.org/pdf/1907.09582
/// @tparam G is the vertex-colored graph.
/// @return the `Certicate`
template<typename G>
    requires IsVertexListGraph<G> && IsIncidenceGraph<G> && IsVertexColoredGraph<G>
std::shared_ptr<CertificateImpl> compute_certificate(const G& graph);

/**
 * Implementations
 */

/// @brief Replace tuples by grouping colors with same hash.
/// @tparam ColorType
/// @param M
/// @param hash_to_color
/// @param out_M_replaced
template<typename ColorType>
void replace_tuples(const std::vector<std::pair<Index, ColorType>>& M,
                    const ColorIndexList& hash_to_color,
                    std::vector<std::tuple<ColorIndex, std::vector<ColorType>, Index>>& out_M_replaced)
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
void split_color_classes(const std::vector<std::tuple<ColorIndex, std::vector<ColorType>, Index>>& M_replaced,
                         UnorderedMap<std::pair<ColorIndex, std::vector<ColorType>>, ColorIndex>& ref_f,
                         ColorIndex& ref_max_color,
                         ColorIndexList& ref_hash_to_color,
                         ColorIndexMap<IndexList>& out_color_to_hashes,
                         ColorIndexSet& out_L)
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
std::shared_ptr<CertificateImpl> compute_certificate(const G& graph)
{
    if (!is_undirected(graph))
    {
        throw std::runtime_error("Color-refinement does not support directed graphs because they can be translated into undirected graphs by introducing two "
                                 "vertices along the edge with different colors to encode the direction.");
    }

    // Toggle verbosity
    const bool debug = false;

    /* Fetch some data. */
    const auto num_vertices = graph.get_num_vertices();

    /* Canonize and compress the IColors to a list of integers. */
    auto c = CertificateImpl::CanonicalColorCompressionFunction {};
    auto canonical_coloring = ColorList {};
    for (const auto& vertex : graph.get_vertices())
    {
        canonical_coloring.push_back(get_color(vertex));
    }
    std::sort(canonical_coloring.begin(), canonical_coloring.end());
    for (const auto& color : canonical_coloring)
    {
        c.emplace(color, c.size());
    }

    // (line 1-2): Initialize vertex colors and perfect hashes for vertex indices.
    auto vertex_to_hash = IndexMap<Index>();
    auto hash_to_vertex = IndexMap<Index>();
    auto max_color = ColorIndex();
    auto hash_to_color = ColorIndexList(num_vertices);
    auto color_to_hashes = ColorIndexMap<IndexList>();
    // (line 1-2): Initialize work list.
    auto L = ColorIndexSet();
    for (const auto& vertex : graph.get_vertices())
    {
        const auto hash = vertex_to_hash.size();
        vertex_to_hash.emplace(vertex.get_index(), hash);
        hash_to_vertex.emplace(hash, vertex.get_index());

        const auto color = c.at(get_color(vertex));
        max_color = std::max(max_color, color);
        hash_to_color[hash] = color;
        color_to_hashes[color].push_back(hash);
        L.insert(color);
    }

    // (line 1-2): Initialize multi set.
    auto f = CertificateImpl::ConfigurationCompressionFunction();
    auto M = std::vector<std::pair<Index, ColorIndex>>();
    auto M_replaced = std::vector<std::tuple<ColorIndex, ColorIndexList, Index>>();
    // (line 3): Process work list until all vertex colors have stabilized.
    while (!L.empty())
    {
        if (debug)
        {
            std::cout << "L: ";
            mimir::operator<<(std::cout, L);
            std::cout << std::endl;
        }

        // Clear data structures that are reused.
        M.clear();
        M_replaced.clear();

        {
            // Subroutine to compute multiset M.
            // Note: this computes the stable coloring, not the coarsest stable coloring.
            for (size_t h = 0; h < num_vertices; ++h)
            {
                for (const auto& outgoing_vertex : graph.template get_adjacent_vertices<ForwardTag>(hash_to_vertex.at(h)))
                {
                    const auto hash = vertex_to_hash.at(outgoing_vertex.get_index());

                    M.emplace_back(h, hash_to_color.at(hash));
                }
            }
        }

        // (line 12): Perform radix sort of M by vertex and colors (TODO radix sort)
        std::sort(M.begin(), M.end());

        if (debug)
        {
            std::cout << "M: ";
            mimir::operator<<(std::cout, M);
            std::cout << std::endl;
        }

        // (line 13): Scan M and replace tuples (v,c1),...,(v,cr) with single tuple (C(v),c1,...,cr,v) to construct signatures.
        replace_tuples(M, hash_to_color, M_replaced);

        // (line 14): Perform radix sort of M by old color and neighborhood colors (TODO radix sort)
        std::sort(M_replaced.begin(), M_replaced.end());

        if (debug)
        {
            std::cout << "M_replaced: ";
            mimir::operator<<(std::cout, M_replaced);
            std::cout << std::endl;
        }

        /* (line 15): Add new colors to work list */
        split_color_classes(M_replaced, f, max_color, hash_to_color, color_to_hashes, L);
    }

    /* Report final neighborhood structures in the decoding table. */
    for (const auto& [old_color, signature, hash] : M_replaced)
    {
        f.emplace(std::make_pair(old_color, signature), old_color);
    }

    /* Return the certificate */
    return std::make_shared<CertificateImpl>(std::move(c), std::move(f), std::move(hash_to_color));
}
}

#endif