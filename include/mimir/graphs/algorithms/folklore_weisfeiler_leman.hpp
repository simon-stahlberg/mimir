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

    Certificate(IsomorphicTypeCompressionFunction f, ConfigurationCompressionFunction g, ColorList hash_to_color) :
        m_hash_to_color(std::move(hash_to_color)),
        m_f(f.begin(), f.end()),
        m_g(g.begin(), g.end())
    {
    }

    const CanonicalIsomorphicTypeCompressionFunction& get_canonical_isomorphic_type_compression_function() const { return m_f; }
    const CanonicalConfigurationCompressionFunction& get_canonical_configuration_compression_function() const { return m_g; }
    const CanonicalColoring& get_canonical_coloring() const { return m_coloring; }

private:
    ColorList m_hash_to_color;

    CanonicalIsomorphicTypeCompressionFunction m_f;
    CanonicalConfigurationCompressionFunction m_g;
    CanonicalColoring m_coloring;
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

/// @brief `compute_certificate` implements the k-dimensional Folklore Weisfeiler-Leman algorithm.
/// Source: https://arxiv.org/pdf/1907.09582
/// @tparam G is the vertex-colored graph.
/// @tparam K is the dimensionality.
/// @return
template<size_t K, typename G>
requires IsVertexListGraph<G> && IsIncidenceGraph<G> && IsVertexColoredGraph<G> Certificate<K> compute_certificate(const G& graph)
{
    // Toggle verbosity
    const bool debug = false;

    /* Fetch some data. */
    const auto num_vertices = graph.template get_num_vertices();
    const auto num_hashes = std::pow(num_vertices, K);

    /* Bookkeeping to support dynamic graphs. */
    auto max_color = Color();
    auto vertex_to_v = IndexMap<Index>();
    auto v_to_vertex = IndexMap<Index>();
    for (const auto& vertex : graph.get_vertex_indices())
    {
        max_color = std::max(max_color, get_color(graph.get_vertex(vertex)));
        const auto v = Index(vertex_to_v.size());
        vertex_to_v.emplace(vertex, v);
        v_to_vertex.emplace(v, vertex);
    }

    /* Decoding table. */
    auto f = typename Certificate<K>::IsomorphicTypeCompressionFunction();
    auto g = typename Certificate<K>::ConfigurationCompressionFunction();

    /* Compute isomorphism types. */
    // Create adj matrix for fast creation of subgraph induced by k-tuple.
    auto adj_matrix = std::vector<std::vector<bool>>(num_vertices, std::vector<bool>(num_vertices, false));
    for (const auto& vertex1 : graph.get_vertex_indices())
    {
        for (const auto& vertex2 : graph.template get_adjacent_vertex_indices<ForwardTraversal>(vertex1))
        {
            adj_matrix.at(vertex_to_v.at(vertex1)).at(vertex_to_v.at(vertex2)) = true;
        }
    }

    auto subgraph = nauty_wrapper::SparseGraph(K);
    auto subgraph_coloring = ColorList();
    auto iso_types = std::vector<std::pair<nauty_wrapper::Certificate, Index>>();

    auto L = ColorSet();
    auto hash_to_color = ColorList(num_hashes);
    auto color_to_hashes = ColorMap<IndexSet>();
    {
        // Subroutine to compute (ordered) isomorphic types of all k-tuples of vertices.
        auto v_to_i = IndexMap<Index>();
        for (size_t h = 0; h < num_hashes; ++h)
        {
            // Compress indexing of subgraph.
            v_to_i.clear();
            auto t = hash_to_tuple<K>(h, num_vertices);
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

            // Compute certificate for k-tuple h.
            iso_types.emplace_back(subgraph.compute_certificate(), h);
        }

        // Create ordered iso-types.
        std::sort(iso_types.begin(), iso_types.end());
        for (const auto& [certificate, h] : iso_types)
        {
            auto result = f.insert(std::make_pair(certificate, max_color + 1));
            const auto new_color = result.first->second;
            max_color = new_color;

            L.insert(new_color);

            hash_to_color.at(h) = new_color;
            color_to_hashes[new_color].insert(h);
        }
    }

    /* Refine colors of k-tuples. */
    auto M = std::vector<std::pair<Index, ColorArray<K>>>();
    // (line 3-18): subroutine to find stable coloring
    while (!L.empty())
    {
        if (debug)
            std::cout << "L: " << L << std::endl;

        {
            // (lines 4-14): Subroutine to fill multiset
            for (const auto& color : L)
            {
                for (const auto& h : color_to_hashes.at(color))
                {
                    const auto tuple = hash_to_tuple<K>(h, num_vertices);

                    for (size_t j = 0; j < K; ++j)
                    {
                        for (size_t u = 0; u < num_vertices; ++u)
                        {
                            auto v_tuple = tuple;
                            v_tuple[j] = u;
                            const auto v_hash = tuple_to_hash(v_tuple, num_vertices);
                            auto k_coloring = ColorArray<K>();
                            for (size_t i = 0; i < K; ++i)
                            {
                                v_tuple[i] = u;
                                k_coloring.at(i) = hash_to_color.at(tuple_to_hash<K>(v_tuple, num_vertices));
                            }
                            M.emplace_back(v_hash, std::move(k_coloring));
                        }
                    }
                }
            }
        }

        // (line 15): Perform radix sort of M
        std::sort(M.begin(), M.end());

        // (line 16): Scan M and replace tuples (vec{v},c_1^1,...,c_k^1,...,vec{v},c_1^r,...,c_k^r) with single tuple
        // (C(vec{v}),(c_1^1,...,c_k^1),...,(c_1^r,...,c_k^r))
        auto M_replaced = std::vector<std::tuple<Color, std::vector<ColorArray<K>>, Index>>();
        {
            // Subroutine to construct signatures.
            auto it = M.begin();
            while (it != M.end())
            {
                auto signature = std::vector<ColorArray<K>>();
                const auto v_hash = it->first;

                auto it2 = it;
                while (it2 != M.end() && it2->first == v_hash)
                {
                    signature.push_back(it2->second);
                    ++it2;
                }
                it = it2;

                // Ensure canonical signature.
                assert(std::is_sorted(signature.begin(), signature.end()));
                M_replaced.emplace_back(hash_to_color.at(v_hash), std::move(signature), v_hash);
            }
        }

        // (line 17): Perform radix sort of M
        std::sort(M_replaced.begin(), M_replaced.end());

        if (debug)
            std::cout << "M_replaced: " << M_replaced << std::endl;

        // (line 18): Add new colors to work list
        color_to_hashes.clear();
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
                            const auto new_color = ++max_color;

                            // Add new color to work list.
                            L.insert(new_color);

                            // Add mapping to decoding table
                            const auto result = g.emplace(std::make_pair(old_color, signature), new_color);
                            // Ensure that we are not overwritting table entries.
                            assert(result.second);

                            {
                                // Subroutine to assign new color to vertices with same signature.
                                while (it2 != M_replaced.end() && old_color == std::get<0>(*it2) && signature == std::get<1>(*it2))
                                {
                                    auto h = std::get<2>(*it2);
                                    hash_to_color[h] = new_color;
                                    color_to_hashes[new_color].insert(h);
                                    ++it2;
                                }
                            }

                            // Ensure that we either finished or entered the next old color class.
                            assert(it2 == M_replaced.end() || old_color != std::get<0>(*it2) || signature != std::get<1>(*it2));
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

    /* Return the certificate */
    return Certificate(std::move(f), std::move(g), std::move(hash_to_color));
}

}

template<size_t K>
struct std::hash<mimir::kfwl::Certificate<K>>
{
    size_t operator()(const mimir::kfwl::Certificate<K>& element) const;
};

#endif