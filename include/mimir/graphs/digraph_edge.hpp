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

#ifndef MIMIR_GRAPHS_DIGRAPH_EDGE_HPP_
#define MIMIR_GRAPHS_DIGRAPH_EDGE_HPP_

#include "mimir/graphs/digraph_edge_interface.hpp"

#include <ranges>
#include <span>
#include <vector>

namespace mimir
{

class DigraphEdge
{
private:
    DigraphEdgeIndex m_index;
    DigraphVertexIndex m_source;
    DigraphVertexIndex m_target;
    DigraphEdgeWeight m_weight;

public:
    DigraphEdge(DigraphEdgeIndex index, DigraphVertexIndex source, DigraphVertexIndex target, DigraphEdgeWeight weight);

    bool operator==(const DigraphEdge& other) const;
    size_t hash() const;

    DigraphEdgeIndex get_index() const;
    DigraphVertexIndex get_source() const;
    DigraphVertexIndex get_target() const;
    DigraphEdgeWeight get_weight() const;
};

}
#endif
