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
#include "mimir/graphs/graph_interface.hpp"
#include "mimir/graphs/graph_vertices.hpp"

namespace mimir
{

/// @brief `FWLCertificate` encapsulates the final tuple colorings and the decoding table.
/// @tparam K is the dimensionality.
template<size_t K>
class FWLCertificate
{
public:
    /* Compression of isomorphic types. */
    using IsomorphicTypeCompressionFunction = std::unordered_map<nauty_wrapper::Certificate, Color>;

    /* Compression of k-color to map (c_1^i, ...,c_k^i) to an integer color. */
    template<size_t K>
    using TupleColorCompressionFunction = std::unordered_map<std::array<Color, K>, Color>;

    /* Compression of new color to map (C(bar{v}), {{(c_1^1, ...,c_k^1), ..., (c_1^r, ...,c_k^r)}}) to an integer color for bar{v} in V^k */
    template<size_t K>
    using ConfigurationCompressionFunction = std::unordered_map<std::pair<Color, std::array<Color, K>>, Color>;

    FWLCertificate(IsomorphicTypeCompressionFunction f, TupleColorCompressionFunction<K> g, ConfigurationCompressionFunction<K> h, ColorList coloring);

private:
    IsomorphicTypeCompressionFunction m_f;
    TupleColorCompressionFunction<K> m_g;
    ConfigurationCompressionFunction<K> m_h;
    ColorList m_coloring;
};

template<size_t K>
bool operator==(const FWLCertificate<K>& lhs, const FWLCertificate<K>& rhs);

/// @brief `compute_fwl_certificate` implements the k-dimensional Folklore Weisfeiler-Leman algorithm.
/// Source: https://people.cs.umass.edu/~immerman/pub/opt.pdf
/// @tparam G is the vertex-colored graph.
/// @tparam K is the dimensionality.
/// @return
template<size_t K, typename G>
requires IsVertexListGraph<G> && IsIncidenceGraph<G> && IsVertexColoredGraph<G>
extern FWLCertificate<K> compute_fwl_certificate(const G& graph);

}

template<size_t K>
struct std::hash<mimir::FWLCertificate<K>>
{
    size_t operator()(const mimir::FWLCertificate<K>& element) const;
};

#endif