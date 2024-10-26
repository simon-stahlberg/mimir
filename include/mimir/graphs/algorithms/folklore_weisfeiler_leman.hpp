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

#include "mimir/algorithms/nauty.hpp"
#include "mimir/common/equal_to.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/common/types.hpp"
#include "mimir/graphs/algorithms/color_refinement.hpp"
#include "mimir/graphs/digraph_vertex_colored.hpp"
#include "mimir/graphs/graph_interface.hpp"
#include "mimir/graphs/graph_traversal_interface.hpp"
#include "mimir/graphs/graph_vertices.hpp"

#include <cassert>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>

namespace mimir::kfwl
{
/// @brief `Certificate` encapsulates the final tuple colorings and the decoding tables.
/// @tparam K is the dimensionality.
template<size_t K>
class Certificate
{
public:
    /* Compression of isomorphic types. */
    using IsomorphicTypeCompressionFunction = std::unordered_map<nauty_wrapper::Certificate, Color>;
    using CanonicalIsomorphicTypeCompressionFunction = std::map<nauty_wrapper::Certificate, Color>;

    /* Compression of new color to map (C(bar{v}), {{(c_1^1, ...,c_k^1), ..., (c_1^r, ...,c_k^r)}}) to an integer color for bar{v} in V^k */
    using ConfigurationCompressionFunction =
        std::unordered_map<std::pair<Color, std::vector<ColorArray<K>>>, Color, Hash<std::pair<Color, std::vector<ColorArray<K>>>>>;
    using CanonicalConfigurationCompressionFunction = std::map<std::pair<Color, std::vector<ColorArray<K>>>, Color>;

    using CanonicalColoring = std::set<Color>;

    Certificate(IsomorphicTypeCompressionFunction f1, ConfigurationCompressionFunction f2, ColorList hash_to_color) :
        m_hash_to_color(std::move(hash_to_color)),
        m_f1(f1.begin(), f1.end()),
        m_f2(f2.begin(), f2.end()),
        m_coloring_coloring(m_hash_to_color.begin(), m_hash_to_color.end())
    {
    }

    const CanonicalIsomorphicTypeCompressionFunction& get_canonical_isomorphic_type_compression_function() const { return m_f1; }
    const CanonicalConfigurationCompressionFunction& get_canonical_configuration_compression_function() const { return m_f2; }
    const CanonicalColoring& get_canonical_coloring() const { return m_coloring_coloring; }

private:
    ColorList m_hash_to_color;

    CanonicalIsomorphicTypeCompressionFunction m_f1;
    CanonicalConfigurationCompressionFunction m_f2;
    CanonicalColoring m_coloring_coloring;
};

template<size_t K>
bool operator==(const Certificate<K>& lhs, const Certificate<K>& rhs)
{
    if (&lhs != &rhs)
    {
        return (lhs.get_canonical_coloring() == rhs.get_canonical_coloring())
               && (lhs.get_canonical_isomorphic_type_compression_function() == rhs.get_canonical_isomorphic_type_compression_function())
               && (lhs.get_canonical_configuration_compression_function() == rhs.get_canonical_configuration_compression_function());
    }
    return true;
}

/// @brief Compute the perfect hash of the given k-tuple.
/// @tparam K is the dimensionality.
/// @param tuple is the k-tuple.
/// @param num_vertices is the number of vertices in the graph.
/// @return the perfect hash of the k-tuple.
template<size_t K>
size_t tuple_to_hash(const IndexArray<K>& tuple, size_t num_vertices)
{
    size_t hash = 0;
    for (size_t i = 0; i < K; ++i)
    {
        hash = hash * num_vertices + tuple[i];
    }
    return hash;
}

/// @brief Compute the k-tuple of the perfect hash.
/// This operation takes O(k) time.
/// @tparam K is the dimensionality.
/// @param hash is the perfect hash.
/// @param num_vertices is the number of vertices in the graph.
/// @return the k-tuple of the perfect hash.
template<size_t K>
IndexArray<K> hash_to_tuple(size_t hash, size_t num_vertices)
{
    auto result = IndexArray<K>();
    for (int64_t i = K - 1; i >= 0; --i)
    {
        result[i] = hash % num_vertices;
        hash /= num_vertices;  // Reduces the hash for the next position
    }
    return result;
}

template<size_t K, typename G>
requires IsVertexListGraph<G> && IsIncidenceGraph<G> && IsVertexColoredGraph<G>  //
    std::tuple<typename Certificate<K>::IsomorphicTypeCompressionFunction, ColorList, ColorMap<IndexList>>
    compute_ordered_isomorphism_types(const G& graph, Color& ref_max_color)
{
    const auto num_vertices = graph.get_num_vertices();

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
        for (const auto& vertex2 : graph.template get_adjacent_vertex_indices<ForwardTraversal>(vertex1))
        {
            adj_matrix.at(vertex_to_v.at(vertex1)).at(vertex_to_v.at(vertex2)) = true;
        }
    }

    const auto num_hashes = std::pow(num_vertices, K);

    auto f1 = typename Certificate<K>::IsomorphicTypeCompressionFunction();
    auto hash_to_color = ColorList(num_hashes);
    auto color_to_hashes = ColorMap<IndexList>();

    auto subgraph = nauty_wrapper::SparseGraph(K);
    auto subgraph_coloring = ColorList();
    auto iso_types = std::vector<std::pair<nauty_wrapper::Certificate, Index>>();

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

        // Initialize empty subgraph and coloring
        const auto subgraph_num_vertices = v_to_i.size();
        subgraph.clear(subgraph_num_vertices);
        subgraph_coloring.resize(subgraph_num_vertices);

        // Instantiate vertex-colored subgraph.
        for (const auto [v1, i1] : v_to_i)
        {
            for (const auto [v2, i2] : v_to_i)
            {
                if (adj_matrix[v1][v2])
                {
                    subgraph.add_edge(i1, i2);
                }
            }
            subgraph_coloring[i1] = get_color(graph.get_vertex(v_to_vertex.at(v1)));
        }
        subgraph.add_vertex_coloring(subgraph_coloring);

        // Compute certificate for k-tuple hash.
        iso_types.emplace_back(subgraph.compute_certificate(), hash);
    }

    // Create ordered iso-types.
    std::sort(iso_types.begin(), iso_types.end());
    for (const auto& [certificate, hash] : iso_types)
    {
        auto result = f1.insert(std::make_pair(certificate, ref_max_color + 1));
        const auto new_color = result.first->second;
        ref_max_color = new_color;

        hash_to_color.at(hash) = new_color;
        color_to_hashes[new_color].push_back(hash);
    }

    return std::make_tuple(std::move(f1), std::move(hash_to_color), std::move(color_to_hashes));
}

/// @brief `compute_certificate` implements the k-dimensional Folklore Weisfeiler-Leman algorithm.
/// Source: https://arxiv.org/pdf/1907.09582
/// @tparam G is the vertex-colored graph.
/// @tparam K is the dimensionality.
/// @return the `Certicate`
template<size_t K, typename G>
requires IsVertexListGraph<G> && IsIncidenceGraph<G> && IsVertexColoredGraph<G>  //
    Certificate<K> compute_certificate(const G& graph)
{
    // Toggle verbosity
    const bool debug = false;

    /* Fetch some data. */
    const auto num_vertices = graph.get_num_vertices();

    /* Compute max color used in graph. */
    auto max_color = Color();
    for (const auto& vertex : graph.get_vertex_indices())
    {
        max_color = std::max(max_color, get_color(graph.get_vertex(vertex)));
    }

    /* Compute isomorphism types. */
    auto f1 = typename Certificate<K>::IsomorphicTypeCompressionFunction();
    auto hash_to_color = ColorList();
    auto color_to_hashes = ColorMap<IndexList>();

    const auto [f1_, hash_to_color_, color_to_hashes_] = compute_ordered_isomorphism_types<K>(graph, max_color);

    f1 = std::move(f1_);
    hash_to_color = std::move(hash_to_color_);
    color_to_hashes = std::move(color_to_hashes_);

    /* Initialize work list of color. */
    auto L = ColorSet(hash_to_color.begin(), hash_to_color.end());

    /* Refine colors of k-tuples. */
    auto f2 = typename Certificate<K>::ConfigurationCompressionFunction();
    auto M = std::vector<std::pair<Index, ColorArray<K>>>();
    auto M_replaced = std::vector<std::tuple<Color, std::vector<ColorArray<K>>, Index>>();
    // (line 3-18): subroutine to find stable coloring
    while (!L.empty())
    {
        if (debug)
            std::cout << "L: " << L << std::endl;

        // Clear data structures that are reused.
        M.clear();
        M_replaced.clear();

        {
            // (lines 4-14): Subroutine to fill multiset
            for (const auto& color : L)
            {
                for (const auto& hash : color_to_hashes.at(color))
                {
                    const auto tuple = hash_to_tuple<K>(hash, num_vertices);

                    for (size_t j = 0; j < K; ++j)
                    {
                        for (size_t u = 0; u < num_vertices; ++u)
                        {
                            auto tmp_tuple = tuple;
                            tmp_tuple[j] = u;
                            const auto v_hash = tuple_to_hash(tmp_tuple, num_vertices);
                            auto k_coloring = ColorArray<K>();
                            for (size_t i = 0; i < K; ++i)
                            {
                                tmp_tuple[i] = u;
                                k_coloring.at(i) = hash_to_color.at(tuple_to_hash<K>(tmp_tuple, num_vertices));
                            }
                            M.emplace_back(v_hash, std::move(k_coloring));
                        }
                    }
                }
            }
        }

        // (line 15): Perform radix sort of M
        std::sort(M.begin(), M.end());

        if (debug)
            std::cout << "M: " << M << std::endl;

        // (line 16): Scan M and replace tuples (vec{v},c_1^1,...,c_k^1,...,vec{v},c_1^r,...,c_k^r) with single tuple
        // (C(vec{v}),(c_1^1,...,c_k^1),...,(c_1^r,...,c_k^r))
        color_refinement::replace_tuples(M, hash_to_color, M_replaced);

        // (line 17): Perform radix sort of M
        std::sort(M_replaced.begin(), M_replaced.end());

        if (debug)
            std::cout << "M_replaced: " << M_replaced << std::endl;

        // (line 18): Split color classes
        color_refinement::split_color_classes(M_replaced, f2, max_color, hash_to_color, color_to_hashes, L);
    }

    /* Return the certificate */
    return Certificate(std::move(f1), std::move(f2), std::move(hash_to_color));
}

}

namespace mimir
{
template<size_t K>
std::ostream& operator<<(std::ostream& out, const kfwl::Certificate<K>& element)
{
    out << "Certificate" << K << "FWL("
        << "canonical_coloring=" << element.get_canonical_coloring() << ", "
        << "canonical_isomorphic_type_compression_function=" << element.get_canonical_isomorphic_type_compression_function() << ", "
        << "canonical_configuration_compression_function=" << element.get_canonical_configuration_compression_function() << ")";
    return out;
}
}

template<size_t K>
struct std::hash<mimir::kfwl::Certificate<K>>
{
    size_t operator()(const mimir::kfwl::Certificate<K>& element) const
    {
        return mimir::hash_combine(element.get_canonical_coloring(),
                                   element.get_canonical_isomorphic_type_compression_function(),
                                   element.get_canonical_configuration_compression_function());
    }
};

#endif