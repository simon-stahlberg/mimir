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

/// @brief `Certificate` encapsulates the canonical coloring and the canonical compression function (decoding table).
class Certificate
{
public:
    using CompressionFunction = std::unordered_map<std::pair<Color, ColorList>, Color, Hash<std::pair<Color, ColorList>>>;

    using CanonicalCompressionFunction = std::map<std::pair<Color, ColorList>, Color>;

    Certificate(CompressionFunction f, ColorList hash_to_color);

    const ColorList& get_hash_to_color() const;

    const CanonicalCompressionFunction& get_canonical_compression_function() const;
    const ColorList& get_canonical_coloring() const;

private:
    ColorList m_hash_to_color;

    CanonicalCompressionFunction m_f;
    ColorList m_canonical_coloring;
};

/// @brief Compare two certificates for equality.
/// @param lhs is the first certificate.
/// @param rhs is the second certificate.
/// @return Return true iff both certificates are equal.
extern bool operator==(const Certificate& lhs, const Certificate& rhs);

/// @brief Print a certificate to the ostream.
/// @param out is the ostream.
/// @param element is the certificate.
/// @return a reference to the ostream.
extern std::ostream& operator<<(std::ostream& out, const Certificate& element);

/// @brief `compute_certificate` implements the color refinement algorithm.
/// Sources: https://arxiv.org/pdf/1907.09582
/// @tparam G is the vertex-colored graph.
/// @return the `Certicate`
template<typename G>
requires IsVertexListGraph<G> && IsIncidenceGraph<G> && IsVertexColoredGraph<G>  //
    Certificate compute_certificate(const G& graph);
}

/// @brief std::hash specialization for the certificate.
template<>
struct std::hash<mimir::color_refinement::Certificate>
{
    size_t operator()(const mimir::color_refinement::Certificate& element) const;
};

#include "mimir/graphs/algorithms/color_refinement_impl.hpp"

#endif