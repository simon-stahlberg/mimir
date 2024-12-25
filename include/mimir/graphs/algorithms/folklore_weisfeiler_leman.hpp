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
#include "mimir/common/printers.hpp"
#include "mimir/common/types.hpp"
#include "mimir/graphs/algorithms/color_refinement.hpp"
#include "mimir/graphs/digraph_vertex_colored.hpp"
#include "mimir/graphs/graph_interface.hpp"
#include "mimir/graphs/graph_properties.hpp"
#include "mimir/graphs/graph_traversal_interface.hpp"
#include "mimir/graphs/graph_vertices.hpp"

#include <cassert>
#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>
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
    /* Compression of new color to map (C(bar{v}), {{(c_1^1, ...,c_k^1), ..., (c_1^r, ...,c_k^r)}}) to an integer color for bar{v} in V^k */
    using ConfigurationCompressionFunction =
        std::unordered_map<std::pair<Color, std::vector<ColorArray<K>>>, Color, loki::Hash<std::pair<Color, std::vector<ColorArray<K>>>>>;
    using CanonicalConfigurationCompressionFunction = std::map<std::pair<Color, std::vector<ColorArray<K>>>, Color>;

    Certificate(ConfigurationCompressionFunction f, ColorList hash_to_color);

    const CanonicalConfigurationCompressionFunction& get_canonical_configuration_compression_function() const;
    const ColorList& get_canonical_coloring() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifiable_members() const { return std::forward_as_tuple(std::as_const(m_f), std::as_const(m_canonical_coloring)); }

private:
    ColorList m_hash_to_color;

    CanonicalConfigurationCompressionFunction m_f;
    ColorList m_canonical_coloring;
};

/// @brief `IsomorphismTypeCompressionFunction` encapsulates mappings from canonical subgraphs to colors.
using IsomorphismTypeCompressionFunction =
    std::unordered_map<nauty_wrapper::Certificate, Color, loki::Hash<nauty_wrapper::Certificate>, loki::EqualTo<nauty_wrapper::Certificate>>;

/// @brief Compare two certificates for equality.
/// @param lhs is the first certificate.
/// @param rhs is the second certificate.
/// @return Return true iff both certificates are equal.
template<size_t K>
bool operator==(const Certificate<K>& lhs, const Certificate<K>& rhs);

/// @brief Print a certificate to the ostream.
/// @param out is the ostream.
/// @param element is the certificate.
/// @return a reference to the ostream.
template<size_t K>
std::ostream& operator<<(std::ostream& out, const Certificate<K>& element);

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
/// @tparam G is the vertex-colored graph.
/// @tparam K is the dimensionality.
/// @return the `Certicate`
template<size_t K, typename G>
    requires IsVertexListGraph<G> && IsIncidenceGraph<G> && IsVertexColoredGraph<G>  //
Certificate<K> compute_certificate(const G& graph, IsomorphismTypeCompressionFunction& iso_type_function);
}

#include "mimir/graphs/algorithms/folklore_weisfeiler_leman_impl.hpp"

#endif