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

#ifndef MIMIR_GRAPHS_DIGRAPH_HPP_
#define MIMIR_GRAPHS_DIGRAPH_HPP_

#include "mimir/graphs/graph.hpp"
#include "mimir/graphs/graph_edge_interface.hpp"
#include "mimir/graphs/graph_interface.hpp"

namespace mimir
{
class DigraphVertex
{
private:
    VertexIndex m_index;

public:
    explicit DigraphVertex(VertexIndex index);

    bool operator==(const DigraphVertex& other) const;
    size_t hash() const;

    VertexIndex get_index() const;
};

class DigraphEdge
{
private:
    EdgeIndex m_index;
    VertexIndex m_source;
    VertexIndex m_target;

public:
    DigraphEdge(EdgeIndex index, VertexIndex source, VertexIndex target);

    bool operator==(const DigraphEdge& other) const;
    size_t hash() const;

    EdgeIndex get_index() const;
    VertexIndex get_source() const;
    VertexIndex get_target() const;
};

using Digraph = Graph<DigraphVertex, DigraphEdge>;

static_assert(IsGraph<Digraph>);

/**
 * Pretty printing
 */

extern std::ostream& operator<<(std::ostream& out, const Digraph& digraph);

}
#endif
