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

namespace mimir::kfwl
{
/// @brief `Certificate` encapsulates the final tuple colorings and the decoding table.
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

    Certificate(IsomorphicTypeCompressionFunction f, TupleColorCompressionFunction<K> g, ConfigurationCompressionFunction<K> h, IndexMap<Color> tuple_to_color);

private:
    IndexMap<Color> m_tuple_to_color;

    CanonicalIsomorphicTypeCompressionFunction m_f;
    CanonicalTupleColorCompressionFunction<K> m_g;
    CanonicalConfigurationCompressionFunction<K> m_h;
    CanonicalColoring m_coloring;
};

/// @brief `TuplePerfectHashFunction` implements a perfect hash function for k-tuples.
/// @tparam K is the dimensionality.
template<size_t K>
class TuplePerfectHashFunction
{
    /// @brief Compute the perfect hash of the given k-tuple.
    /// @param tuple is the k-tuple.
    /// @param num_vertices is the number of vertices in the graph.
    /// @return the perfect hash of the k-tuple.
    static size_t hash(const std::array<Index, K>& tuple, size_t num_vertices) const;

    /// @brief Compute the k-tuple of the perfect hash.
    /// This operation takes O(k) time.
    /// @param hash is the perfect hash.
    /// @param num_vertices is the number of vertices in the graph.
    /// @return the k-tuple of the perfect hash.
    static std::array<Index, K> tuple(size_t hash, size_t num_vertices) const;

    /// @brief Compute the perfect hash of the J-neighbor of the given tuple by replacing J-th entry by `y`.
    /// This operation takes O(1) time.
    /// @param tuple is the k-tuple
    /// @param j is the index.
    /// @param y is the replacement value
    /// @param num_vertices is the number of vertices in the graph.
    /// @return the perfect hash of the J-neighbour of the given tuple.
    static size_t hash_neighbor(const std::array<Index, K>& tuple, Index j, Index y, size_t num_vertices);
};

template<size_t K>
bool operator==(const Certificate<K>& lhs, const Certificate<K>& rhs);

/// @brief `compute_certificate` implements the k-dimensional Folklore Weisfeiler-Leman algorithm.
/// Source: https://arxiv.org/pdf/1907.09582
/// @tparam G is the vertex-colored graph.
/// @tparam K is the dimensionality.
/// @return
template<size_t K, typename G>
requires IsVertexListGraph<G> && IsIncidenceGraph<G> && IsVertexColoredGraph<G> Certificate<K> compute_certificate(const G& graph) {}

}

template<size_t K>
struct std::hash<mimir::kfwl::Certificate<K>>
{
    size_t operator()(const mimir::kfwl::Certificate<K>& element) const;
};

#endif