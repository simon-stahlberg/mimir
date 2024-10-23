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

    /* Compression of k-color to map (c_1^i, ...,c_k^i) to an integer color. */
    template<size_t K>
    using TupleColorCompressionFunction = std::unordered_map<std::array<Color, K>, Color>;
    template<size_t K>
    using CanonicalTupleColorCompressionFunction = std::map<std::array<Color, K>, Color>;

    /* Compression of new color to map (C(bar{v}), {{(c_1^1, ...,c_k^1), ..., (c_1^r, ...,c_k^r)}}) to an integer color for bar{v} in V^k */
    template<size_t K>
    using ConfigurationCompressionFunction = std::unordered_map<std::pair<Color, std::array<Color, K>>, Color>;
    template<size_t K>
    using CanonicalConfigurationCompressionFunction = std::map<std::pair<Color, std::array<Color, K>>, Color>;

    using CanonicalColoring = std::set<Color>;

    Certificate(IsomorphicTypeCompressionFunction f, TupleColorCompressionFunction<K> g, ConfigurationCompressionFunction<K> h, ColorList tuple_to_color);

private:
    ColorList m_tuple_to_color;

    CanonicalIsomorphicTypeCompressionFunction m_f;
    CanonicalTupleColorCompressionFunction<K> m_g;
    CanonicalConfigurationCompressionFunction<K> m_h;
    CanonicalColoring m_coloring;
};

template<size_t K>
bool operator==(const Certificate<K>& lhs, const Certificate<K>& rhs);

/// @brief Compute the perfect hash of the given k-tuple.
/// @tparam K is the dimensionality.
/// @param tuple is the k-tuple.
/// @param num_vertices is the number of vertices in the graph.
/// @return the perfect hash of the k-tuple.
template<size_t K>
size_t tuple_to_hash(const std::array<Index, K>& tuple, size_t num_vertices);

/// @brief Compute the k-tuple of the perfect hash.
/// This operation takes O(k) time.
/// @tparam K is the dimensionality.
/// @param hash is the perfect hash.
/// @param num_vertices is the number of vertices in the graph.
/// @return the k-tuple of the perfect hash.
template<size_t K>
std::array<Index, K> hash_to_tuple(size_t hash, size_t num_vertices);

/// @brief Compute the perfect hash of the J-neighbor of the given tuple by replacing J-th entry by `y`.
/// This operation takes O(1) time.
/// @tparam K is the dimensionality.
/// @param tuple is the k-tuple
/// @param j is the index.
/// @param y is the replacement value
/// @param num_vertices is the number of vertices in the graph.
/// @return the perfect hash of the J-neighbour of the given tuple.
template<size_t K>
size_t tuple_to_neighbor_hash(const std::array<Index, K>& tuple, Index j, Index y, size_t num_vertices);

/// @brief `compute_certificate` implements the k-dimensional Folklore Weisfeiler-Leman algorithm.
/// Source: https://arxiv.org/pdf/1907.09582
/// @tparam G is the vertex-colored graph.
/// @tparam K is the dimensionality.
/// @return
template<size_t K, typename G>
requires IsVertexListGraph<G> && IsIncidenceGraph<G> && IsVertexColoredGraph<G> Certificate<K> compute_certificate(const G& graph)
{
    /* Fetch some data. */
    const auto num_vertices = graph.get_num_vertices<ForwardTraversal>();

    /* Bookkeeping to support dynamic graphs. */
    auto vertex_to_v = IndexMap<Index>();
    auto v_to_vertex = IndexMap<Index>();
    for (const auto& vertex : graph.get_vertex_indices())
    {
        auto v = Index(vertex_to_v.size());
        vertex_to_v.emplace(vertex, v);
        v_to_vertex.emplace(v, vertex);
    }

    /* Initialize result data structures. */
    auto tuple_to_color = ColorList();
    auto f = Certificate<K>::IsomorphicTypeCompressionFunction();
    auto g = Certificate<K>::TupleColorCompressionFunction();
    auto h = Certificate<K>::ConfigurationCompressionFunction();

    /* Compute isomorphism types. */
    // Create adj matrix for fast creation of subgraph induced by k-tuple.
    auto adj_matrix = std::vector<std::vector<bool>>(std::vector<bool>(false, num_vertices), num_vertices);
    for (const auto& vertex1 : graph.get_vertex_indices<ForwardTraversal>())
    {
        for (const auto& vertex2 : graph.get_adjacent_vertex_indices<ForwardTraversal>())
        {
            adj_matrix.at(vertex_to_v.at(vertex1)).at(vertex_to_v.at(vertex2)) = true;
        }
    }

    auto subgraph = nauty_wrapper::SparseGraph(K);
    auto subgraph_coloring = ColorList();
    auto iso_types = std::vector<std::pair<nauty_wrapper::Certificate, Index>>();

    for (size_t h = 0; h < std::pow(num_vertices, K), ++h)
    {
        auto t = hash_to_tuple<K>(h, num_vertices);
        for (const auto& v1 : h)
        {
            for (const auto& v2 : h)
            {
                if (adj_matrix.at(v1).at(v2))
                {
                    subgraph.add_edge(v1, v2);
                }
            }
            subgraph_coloring.push_back(get_color(graph.get_vertex(v_to_vertex.at(v1))));
        }

        iso_types.emplace(subgraph.compute_certificate(), h);

        subgraph.clear();
        subgraph_coloring.clear();
    }

    // Create ordered iso-types.
    std::sort(iso_types.begin(), iso_types.end());
    for (const auto& [certificate, h] : iso_types)
    {
        auto result = f.insert(certificate, f.size());
        tuple_to_color.push_back(result.first->second);
    }

    /* TODO: refine */

    /* TODO: return certificate */
}

}

template<size_t K>
struct std::hash<mimir::kfwl::Certificate<K>>
{
    size_t operator()(const mimir::kfwl::Certificate<K>& element) const;
};

#endif