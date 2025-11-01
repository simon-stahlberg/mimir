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

#ifndef MIMIR_GRAPHS_ALGORITHMS_FOLKLORE_WEISFEILER_LEMAN_HPP_
#define MIMIR_GRAPHS_ALGORITHMS_FOLKLORE_WEISFEILER_LEMAN_HPP_

#include "mimir/common/declarations.hpp"
#include "mimir/common/equal_to.hpp"
#include "mimir/common/formatter.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/graphs/algorithms/color_refinement.hpp"
#include "mimir/graphs/algorithms/nauty.hpp"
#include "mimir/graphs/graph_properties.hpp"
#include "mimir/graphs/graph_traversal_interface.hpp"
#include "mimir/graphs/graph_vertices.hpp"
#include "mimir/graphs/static_graph.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>
#include <map>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace mimir::graphs::kfwl
{

/// @brief `CertificateImpl` encapsulates the final tuple colorings and the decoding tables.
/// @tparam K is the dimensionality.
template<size_t K>
class CertificateImpl
{
public:
    using CanonicalColorCompressionFunction = std::map<PropertyValue, ColorIndex>;

    /* Compression of new color to map (C(bar{v}), {{(c_1^1, ...,c_k^1), ..., (c_1^r, ...,c_k^r)}}) to an integer color for bar{v} in V^k */
    using ConfigurationCompressionFunction = UnorderedMap<std::pair<ColorIndex, ColorIndexArrayList<K>>, ColorIndex>;
    using CanonicalConfigurationCompressionFunction = std::map<std::pair<ColorIndex, ColorIndexArrayList<K>>, ColorIndex>;

    CertificateImpl(CanonicalColorCompressionFunction c, ConfigurationCompressionFunction f, ColorIndexList hash_to_color);

    const CanonicalColorCompressionFunction& get_canonical_color_compression_function() const;
    const CanonicalConfigurationCompressionFunction& get_canonical_configuration_compression_function() const;
    const ColorIndexList& get_hash_to_color() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept
    {
        return std::tuple(std::cref(get_canonical_color_compression_function()), std::cref(get_canonical_configuration_compression_function()));
    }

private:
    CanonicalColorCompressionFunction m_c;
    CanonicalConfigurationCompressionFunction m_f;

    ColorIndexList m_hash_to_color;
};

/// @brief `IsomorphismTypeCompressionFunction` encapsulates mappings from canonical subgraphs to colors.
struct IsomorphismTypeCompressionFunction
{
    UnorderedMap<nauty::SparseGraph, ColorIndex> data;

    IsomorphismTypeCompressionFunction() = default;
};

/// @brief Compare two certificates for equality.
/// @param lhs is the first certificate.
/// @param rhs is the second certificate.
/// @return Return true iff both certificates are equal.
template<size_t K>
bool operator==(const CertificateImpl<K>& lhs, const CertificateImpl<K>& rhs);

/// @brief Compute the perfect hash of the given k-tuple.
/// @tparam K is the dimensionality.
/// @param tuple is the k-tuple.
/// @param num_vertices is the number of vertices in the graph.
/// @return the perfect hash of the k-tuple.
template<size_t K>
size_t tuple_to_hash(const IndexArray<K>& tuple, size_t num_vertices);

/// @brief Compute the k-tuple of the perfect hash.
/// This operation takes O(k) time.
/// @tparam K is the dimensionality.
/// @param hash is the perfect hash.
/// @param num_vertices is the number of vertices in the graph.
/// @return the k-tuple of the perfect hash.
template<size_t K>
IndexArray<K> hash_to_tuple(size_t hash, size_t num_vertices);

/// @brief `compute_certificate` implements the k-dimensional Folklore Weisfeiler-Leman algorithm.
/// Source: https://arxiv.org/pdf/1907.09582
/// @tparam K is the dimensionality.
/// @tparam G is the vertex-colored graph.
/// @return the `Certicate`
template<std::size_t K, typename G>
    requires IsVertexListGraph<G> && IsIncidenceGraph<G>
std::shared_ptr<CertificateImpl<K>> compute_certificate(const G& graph, IsomorphismTypeCompressionFunction& iso_type_function);

/**
 * Implementations
 */

/// @brief `CertificateImpl` encapsulates the final tuple colorings and the decoding tables.
/// @tparam K is the dimensionality.
template<size_t K>
CertificateImpl<K>::CertificateImpl(CanonicalColorCompressionFunction c, ConfigurationCompressionFunction f, ColorIndexList hash_to_color) :
    m_c(std::move(c)),
    m_f(f.begin(), f.end()),
    m_hash_to_color(std::move(hash_to_color))
{
}

template<size_t K>
const CertificateImpl<K>::CanonicalColorCompressionFunction& CertificateImpl<K>::get_canonical_color_compression_function() const
{
    return m_c;
}

template<size_t K>
const CertificateImpl<K>::CanonicalConfigurationCompressionFunction& CertificateImpl<K>::get_canonical_configuration_compression_function() const
{
    return m_f;
}

template<size_t K>
const ColorIndexList& CertificateImpl<K>::get_hash_to_color() const
{
    return m_hash_to_color;
}

template<size_t K>
bool operator==(const CertificateImpl<K>& lhs, const CertificateImpl<K>& rhs)
{
    return loki::EqualTo<CertificateImpl<K>>()(lhs, rhs);
}

template<size_t K>
size_t tuple_to_hash(const IndexArray<K>& tuple, size_t num_vertices)
{
    size_t hash = 0;
    size_t weight = 1;
    for (size_t i = 0; i < K; ++i)
    {
        hash += weight * tuple[i];
        weight *= num_vertices;
    }
    return hash;
}

template<size_t K>
IndexArray<K> hash_to_tuple(size_t hash, size_t num_vertices)
{
    auto result = IndexArray<K>();
    auto weight = (size_t) std::pow(num_vertices, K - 1);
    for (int64_t i = K - 1; i >= 0; --i)
    {
        result[i] = hash / weight;
        hash %= weight;
        weight /= num_vertices;
    }
    return result;
}

/// @brief Compute the ordered isomorphism types of all k-tuples in the graph.
/// @tparam G is the graph type.
/// @tparam K is the dimensionality.
/// @param graph is the graph
/// @param iso_type_function is the function that tracks assigned colors to canonical subgraphs.
/// @return two mappings: k-tuple hash to color and color to k-tuple hashes.
template<size_t K, typename G>
    requires IsVertexListGraph<G> && IsIncidenceGraph<G>  //
auto compute_ordered_isomorphism_types(const G& graph, IsomorphismTypeCompressionFunction& iso_type_function)
{
    const auto num_vertices = graph.get_num_vertices();
    const auto num_hashes = std::pow(num_vertices, K);

    /* Temporary bookkeeping to support dynamic graphs. */
    auto vertex_to_v = IndexMap<Index>();
    auto v_to_vertex = IndexMap<Index>();
    for (const auto& vertex : graph.get_vertex_indices())
    {
        const auto v = Index(vertex_to_v.size());
        vertex_to_v.emplace(vertex, v);
        v_to_vertex.emplace(v, vertex);
    }

    // Create adj matrix for fast creation of subgraph induced by k-tuple.
    auto adj_matrix = std::vector<std::vector<bool>>(num_vertices, std::vector<bool>(num_vertices, false));
    for (const auto& vertex1 : graph.get_vertex_indices())
    {
        for (const auto& vertex2 : graph.template get_adjacent_vertex_indices<ForwardTag>(vertex1))
        {
            adj_matrix.at(vertex_to_v.at(vertex1)).at(vertex_to_v.at(vertex2)) = true;
        }
    }

    /* Canonize and compress the IColors to a list of integers. */
    auto c = typename CertificateImpl<K>::CanonicalColorCompressionFunction {};
    auto canonical_coloring = PropertyValueList {};
    for (const auto& vertex : graph.get_vertices())
    {
        canonical_coloring.push_back(vertex.get_properties());
    }
    std::sort(canonical_coloring.begin(), canonical_coloring.end());
    for (const auto& color : canonical_coloring)
    {
        c.emplace(color, c.size());
    }

    auto hash_to_color = ColorIndexList(num_hashes);
    auto color_to_hashes = ColorIndexMap<IndexList>();

    // Subroutine to compute (ordered) isomorphic types of all k-tuples of vertices.
    auto v_to_i = IndexMap<Index>();
    for (size_t hash = 0; hash < num_hashes; ++hash)
    {
        // Compress indexing of subgraph.
        v_to_i.clear();
        auto t = hash_to_tuple<K>(hash, num_vertices);
        for (size_t i = 0; i < K; ++i)
        {
            v_to_i.emplace(t[i], v_to_i.size());
        }

        // Instantiate vertex-colored subgraph.
        auto subgraph = StaticGraph<Vertex<PropertyValue>, Edge<>>();
        for (const auto [v1, i1] : v_to_i)
        {
            subgraph.add_vertex(graph.get_vertex(v_to_vertex.at(v1)).get_properties());
        }
        for (const auto [v1, i1] : v_to_i)
        {
            for (const auto [v2, i2] : v_to_i)
            {
                if (adj_matrix[v1][v2])
                {
                    subgraph.add_directed_edge(i1, i2);
                }
            }
        }

        // Isomorphism function is shared among several runs to ensure canonical form for different runs.
        auto result = iso_type_function.data.emplace(nauty::SparseGraph(subgraph).canonize(), iso_type_function.data.size());

        const auto color = result.first->second;
        hash_to_color.at(hash) = color;
        color_to_hashes[color].push_back(hash);
    }

    return std::tuple(c, hash_to_color, color_to_hashes);
}

template<size_t K, typename G>
    requires IsVertexListGraph<G> && IsIncidenceGraph<G>  //
std::shared_ptr<CertificateImpl<K>> compute_certificate(const G& graph, IsomorphismTypeCompressionFunction& iso_type_function)
{
    if (!is_undirected(graph))
    {
        throw std::runtime_error("K-FWL does not support directed graphs because they can be translated into undirected graphs by introducing two vertices "
                                 "along the edge with different colors to encode the direction.");
    }

    // Toggle verbosity
    const bool debug = false;

    /* Fetch some data. */
    const auto num_vertices = graph.get_num_vertices();
    const auto num_hashes = std::pow(num_vertices, K);

    if (debug)
    {
        std::cout << "num_vertices=" << num_vertices << std::endl  //
                  << "num_hashes=" << num_hashes << std::endl;
    }

    /* Compute isomorphism types. */
    auto c = typename CertificateImpl<K>::CanonicalColorCompressionFunction();
    auto hash_to_color = ColorIndexList();
    auto color_to_hashes = ColorIndexMap<IndexList>();

    const auto [c_, hash_to_color_, color_to_hashes_] = compute_ordered_isomorphism_types<K>(graph, iso_type_function);
    c = std::move(c_);
    hash_to_color = std::move(hash_to_color_);
    color_to_hashes = std::move(color_to_hashes_);

    auto max_color = ColorIndex(c.size());

    /* Initialize work list of color. */
    auto L = ColorIndexSet(hash_to_color.begin(), hash_to_color.end());

    /* Refine colors of k-tuples. */
    auto f = typename CertificateImpl<K>::ConfigurationCompressionFunction();
    auto M = std::vector<std::pair<Index, ColorIndexArray<K>>>();
    auto M_replaced = std::vector<std::tuple<ColorIndex, ColorIndexArrayList<K>, Index>>();
    // (line 3-18): subroutine to find stable coloring

    while (!L.empty())
    {
        if (debug)
        {
            std::cout << "L: ";
            mimir::print(std::cout, L);
            std::cout << std::endl;
        }

        // Clear data structures that are reused.
        M.clear();
        M_replaced.clear();

        {
            // (lines 4-14): Subroutine to fill multiset
            // Note: this computes the stable coloring, not the coarsest stable coloring.
            for (size_t h = 0; h < num_hashes; ++h)
            {
                const auto w = hash_to_tuple<K>(h, num_vertices);

                for (size_t j = 0; j < K; ++j)
                {
                    for (size_t u = 0; u < num_vertices; ++u)
                    {
                        // C[\vec{v}[1,u]],...,C[\vec{v}[k,u]]
                        auto k_coloring = ColorIndexArray<K>();
                        for (size_t i = 0; i < K; ++i)
                        {
                            // \vec{x} = \vec{v}[i,u]
                            auto x = w;
                            x[i] = u;
                            const auto x_hash = tuple_to_hash<K>(x, num_vertices);

                            k_coloring.at(i) = hash_to_color.at(x_hash);
                        }

                        M.emplace_back(h, std::move(k_coloring));
                    }
                }
            }
        }

        // (line 15): Perform radix sort of M
        std::sort(M.begin(), M.end());

        if (debug)
        {
            std::cout << "M: ";
            mimir::print(std::cout, M);
            std::cout << std::endl;
        }

        // (line 16): Scan M and replace tuples (vec{v},c_1^1,...,c_k^1,...,vec{v},c_1^r,...,c_k^r) with single tuple
        // (C(vec{v}),(c_1^1,...,c_k^1),...,(c_1^r,...,c_k^r))
        color_refinement::replace_tuples(M, hash_to_color, M_replaced);

        // (line 17): Perform radix sort of M
        std::sort(M_replaced.begin(), M_replaced.end());

        if (debug)
        {
            std::cout << "M_replaced: ";
            mimir::print(std::cout, M_replaced);
            std::cout << std::endl;
        }

        // (line 18): Split color classes
        color_refinement::split_color_classes(M_replaced, f, max_color, hash_to_color, color_to_hashes, L);
    }

    /* Report final neighborhood structures in the decoding table. */
    for (const auto& [old_color, signature, hash] : M_replaced)
    {
        f.emplace(std::make_pair(old_color, signature), old_color);
    }

    /* Return the certificate */
    return std::make_shared<CertificateImpl<K>>(std::move(c), std::move(f), std::move(hash_to_color));
}
}

#endif