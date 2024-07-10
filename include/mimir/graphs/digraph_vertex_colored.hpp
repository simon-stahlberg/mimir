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

#ifndef MIMIR_GRAPHS_DIGRAPH_VERTEX_COLORE_HPP_
#define MIMIR_GRAPHS_DIGRAPH_VERTEX_COLORE_HPP_

#include "mimir/graphs/coloring_function.hpp"
#include "mimir/graphs/digraph.hpp"

#include <ranges>
#include <span>
#include <vector>

namespace mimir
{

/**
 * Declarations
 */

class VertexColoredDigraph
{
private:
    Digraph m_digraph;
    ColorList m_vertex_colors;

public:
    explicit VertexColoredDigraph(bool is_directed = false);

    DigraphVertexIndex add_vertex(Color color = 0);

    DigraphEdgeIndex add_edge(DigraphVertexIndex source, DigraphVertexIndex target, DigraphEdgeWeight weight = 1.);

    void reset(size_t num_vertices, bool is_directed = false);

    const Digraph& get_digraph() const;
    const ColorList& get_vertex_colors() const;
};

}
#endif
