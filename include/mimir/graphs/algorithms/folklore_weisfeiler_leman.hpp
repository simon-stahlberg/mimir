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
using mimir::operator<<;

template<size_t K>
using Signature = std::pair<std::vector<ColorArray<K>>, std::vector<ColorArray<K>>>;

/// @brief `Certificate` encapsulates the final tuple colorings and the decoding tables.
/// @tparam K is the dimensionality.
template<size_t K>
class Certificate
{
public:
    /* Compression of new color to map (C(bar{v}), {{(c_1^1, ...,c_k^1), ..., (c_1^r, ...,c_k^r)}}) to an integer color for bar{v} in V^k */
    using ConfigurationCompressionFunction = std::unordered_map<std::pair<Color, Signature<K>>, Color, Hash<std::pair<Color, Signature<K>>>>;
    using CanonicalConfigurationCompressionFunction = std::map<std::pair<Color, Signature<K>>, Color>;

    Certificate(ConfigurationCompressionFunction f, ColorList hash_to_color) :
        m_hash_to_color(std::move(hash_to_color)),
        m_f(f.begin(), f.end()),
        m_coloring_coloring(m_hash_to_color.begin(), m_hash_to_color.end())
    {
        std::sort(m_coloring_coloring.begin(), m_coloring_coloring.end());
    }

    const CanonicalConfigurationCompressionFunction& get_canonical_configuration_compression_function() const { return m_f; }
    const ColorList& get_canonical_coloring() const { return m_coloring_coloring; }

private:
    ColorList m_hash_to_color;

    CanonicalConfigurationCompressionFunction m_f;
    ColorList m_coloring_coloring;
};

template<size_t K>
class IsomorphismTypeFunction
{
public:
    /* Compression of isomorphic types. */
    using IsomorphicTypeCompressionFunction = std::unordered_map<nauty_wrapper::Certificate, Color>;

    IsomorphicTypeCompressionFunction& get_isomorphic_type_compression_function() { return m_f1; }

private:
    IsomorphicTypeCompressionFunction m_f1;
};

template<size_t K>
bool operator==(const Certificate<K>& lhs, const Certificate<K>& rhs)
{
    if (&lhs != &rhs)
    {
        return (lhs.get_canonical_coloring() == rhs.get_canonical_coloring())
               && (lhs.get_canonical_configuration_compression_function() == rhs.get_canonical_configuration_compression_function());
    }
    return true;
}

template<size_t K>
std::ostream& operator<<(std::ostream& out, const Certificate<K>& element)
{
    out << "Certificate" << K << "FWL("
        << "canonical_coloring=" << element.get_canonical_coloring() << ", "
        << "canonical_configuration_compression_function=" << element.get_canonical_configuration_compression_function() << ")";
    return out;
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
    size_t weight = 1;
    for (size_t i = 0; i < K; ++i)
    {
        hash += weight * tuple[i];
        weight *= num_vertices;
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
    auto weight = (size_t) std::pow(num_vertices, K - 1);
    for (int64_t i = K - 1; i >= 0; --i)
    {
        result[i] = hash / weight;
        hash %= weight;
        weight /= num_vertices;
    }
    return result;
}

template<size_t K, typename G>
requires IsVertexListGraph<G> && IsIncidenceGraph<G> && IsVertexColoredGraph<G>  //
    std::tuple<ColorList, ColorList, ColorMap<IndexList>, ColorMap<IndexList>> compute_ordered_isomorphism_types(const G& graph,
                                                                                                                 IsomorphismTypeFunction<K>& iso_type_function)
{
    const auto num_vertices = graph.get_num_vertices();
    const auto is_undirected = is_undirected_graph(graph);
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

    auto compute_iso_types =
        [&graph, &iso_type_function, &vertex_to_v, &v_to_vertex, num_vertices, num_hashes](const std::vector<std::vector<bool>>& adj_matrix,
                                                                                           ColorList& out_hash_to_color,
                                                                                           ColorMap<IndexList>& out_color_to_hashes)
    {
        auto subgraph = nauty_wrapper::SparseGraph(K);
        auto subgraph_coloring = ColorList();

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

            // Isomorphism function is shared among several runs to ensure canonical form for different runs.
            auto result =
                iso_type_function.get_isomorphic_type_compression_function().emplace(subgraph.compute_certificate(),
                                                                                     iso_type_function.get_isomorphic_type_compression_function().size());

            const auto color = result.first->second;
            out_hash_to_color.at(hash) = color;
            out_color_to_hashes[color].push_back(hash);
        }
    };

    // Create adj matrix for fast creation of subgraph induced by k-tuple.
    auto adj_matrix_outgoing = std::vector<std::vector<bool>>(num_vertices, std::vector<bool>(num_vertices, false));
    for (const auto& edge : graph.get_edges())
    {
        adj_matrix_outgoing.at(vertex_to_v.at(edge.get_source())).at(vertex_to_v.at(edge.get_target())) = true;
    }

    auto hash_to_color_outgoing = ColorList(num_hashes);
    auto color_to_hashes_outgoing = ColorMap<IndexList>();
    auto hash_to_color_ingoing = ColorList(num_hashes);
    auto color_to_hashes_ingoing = ColorMap<IndexList>();

    compute_iso_types(adj_matrix_outgoing, hash_to_color_outgoing, color_to_hashes_outgoing);

    if (!is_undirected)
    {
        auto adj_matrix_ingoing = std::vector<std::vector<bool>>(num_vertices, std::vector<bool>(num_vertices, false));
        for (const auto& edge : graph.get_edges())
        {
            adj_matrix_ingoing.at(vertex_to_v.at(edge.get_target())).at(vertex_to_v.at(edge.get_source())) = true;
        }

        compute_iso_types(adj_matrix_ingoing, hash_to_color_ingoing, color_to_hashes_ingoing);
    }

    return std::make_tuple(hash_to_color_outgoing, hash_to_color_ingoing, color_to_hashes_outgoing, color_to_hashes_ingoing);
}

/// @brief `compute_certificate` implements the k-dimensional Folklore Weisfeiler-Leman algorithm.
/// Source: https://arxiv.org/pdf/1907.09582
/// @tparam G is the vertex-colored graph.
/// @tparam K is the dimensionality.
/// @return the `Certicate`
template<size_t K, typename G>
requires IsVertexListGraph<G> && IsIncidenceGraph<G> && IsVertexColoredGraph<G>  //
    Certificate<K> compute_certificate(const G& graph, IsomorphismTypeFunction<K>& iso_type_function)
{
    // Toggle verbosity
    const bool debug = false;

    /* Fetch some data. */
    const auto num_vertices = graph.get_num_vertices();
    const auto is_undirected = is_undirected_graph(graph);
    const auto num_hashes = std::pow(num_vertices, K);

    if (debug)
        std::cout << "num_vertices=" << num_vertices << std::endl    //
                  << "is_undirected=" << is_undirected << std::endl  //
                  << "num_hashes=" << num_hashes << std::endl;

    /* Compute max color used in graph. */
    auto max_color = Color();
    for (const auto& vertex : graph.get_vertex_indices())
    {
        max_color = std::max(max_color, get_color(graph.get_vertex(vertex)));
    }

    /* Compute isomorphism types. */
    auto hash_to_color_outgoing = ColorList();
    auto hash_to_color_ingoing = ColorList();
    auto color_to_hashes_outgoing = ColorMap<IndexList>();
    auto color_to_hashes_ingoing = ColorMap<IndexList>();

    const auto [hash_to_color_outgoing_, hash_to_color_ingoing_, color_to_hashes_outgoing_, color_to_hashes_ingoing_] =
        compute_ordered_isomorphism_types<K>(graph, iso_type_function);

    hash_to_color_outgoing = std::move(hash_to_color_outgoing_);
    hash_to_color_ingoing = std::move(hash_to_color_ingoing_);
    color_to_hashes_outgoing = std::move(color_to_hashes_outgoing_);
    color_to_hashes_ingoing = std::move(color_to_hashes_ingoing_);

    /* Initialize work list of color. */
    auto L = ColorSet(hash_to_color_outgoing.begin(), hash_to_color_outgoing.end());

    /* Refine colors of k-tuples. */
    auto f = typename Certificate<K>::ConfigurationCompressionFunction();
    auto M_outgoing = std::vector<std::pair<Index, ColorArray<K>>>();
    auto M_ingoing = std::vector<std::pair<Index, ColorArray<K>>>();
    auto M_combined_replaced = std::vector<std::tuple<Color, Signature<K>, Index>>();
    // (line 3-18): subroutine to find stable coloring

    while (!L.empty())
    {
        if (debug)
            std::cout << "L: " << L << std::endl;

        // Clear data structures that are reused.
        M_outgoing.clear();
        M_outgoing.clear();
        M_combined_replaced.clear();

        {
            const auto fill_multiset = [num_vertices, num_hashes](const ColorSet& L,
                                                                  const ColorList& hash_to_color,
                                                                  const ColorMap<IndexList>& color_to_hashes,
                                                                  std::vector<std::pair<Index, ColorArray<K>>>& out_M)
            {
                // (lines 4-14): Subroutine to fill multiset
                for (size_t h = 0; h < num_hashes; ++h)
                {
                    const auto w = hash_to_tuple<K>(h, num_vertices);

                    for (size_t j = 0; j < K; ++j)
                    {
                        for (size_t u = 0; u < num_vertices; ++u)
                        {
                            // C[\vec{v}[1,u]],...,C[\vec{v}[k,u]]
                            auto k_coloring = ColorArray<K>();
                            for (size_t i = 0; i < K; ++i)
                            {
                                // \vec{x} = \vec{v}[i,u]
                                auto x = w;
                                x[i] = u;
                                const auto x_hash = tuple_to_hash<K>(x, num_vertices);

                                k_coloring.at(i) = hash_to_color.at(x_hash);
                            }

                            out_M.emplace_back(h, std::move(k_coloring));
                        }
                    }
                }
            };

            fill_multiset(L, hash_to_color_outgoing, color_to_hashes_outgoing, M_outgoing);

            if (!is_undirected)
            {
                fill_multiset(L, hash_to_color_ingoing, color_to_hashes_ingoing, M_ingoing);
            }
        }

        // (line 15): Perform radix sort of M
        std::sort(M_outgoing.begin(), M_outgoing.end());
        std::sort(M_ingoing.begin(), M_ingoing.end());

        if (debug)
            std::cout << "M_outgoing: " << M_outgoing << std::endl  //
                      << "M_ingoing: " << M_ingoing << std::endl;

        // (line 16): Scan M and replace tuples (vec{v},c_1^1,...,c_k^1,...,vec{v},c_1^r,...,c_k^r) with single tuple
        // (C(vec{v}),(c_1^1,...,c_k^1),...,(c_1^r,...,c_k^r))
        color_refinement::replace_tuples(M_outgoing, M_ingoing, hash_to_color_outgoing, M_combined_replaced);

        // (line 17): Perform radix sort of M
        std::sort(M_combined_replaced.begin(), M_combined_replaced.end());

        if (debug)
            std::cout << "M_combined_replaced: " << M_combined_replaced << std::endl;

        // (line 18): Split color classes
        color_refinement::split_color_classes(M_combined_replaced, f, max_color, hash_to_color_outgoing, color_to_hashes_outgoing, L);
    }

    /* Report final neighborhood structures in the decoding table. */
    for (const auto& [old_color, signature, hash] : M_combined_replaced)
    {
        f.emplace(std::make_pair(old_color, signature), old_color);
    }

    /* Return the certificate */
    return Certificate(std::move(f), std::move(hash_to_color_outgoing));
}
}

template<size_t K>
struct std::hash<mimir::kfwl::Certificate<K>>
{
    size_t operator()(const mimir::kfwl::Certificate<K>& element) const
    {
        return mimir::hash_combine(element.get_canonical_coloring(), element.get_canonical_configuration_compression_function());
    }
};

#endif