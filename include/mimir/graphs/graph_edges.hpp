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

#ifndef MIMIR_GRAPHS_GRAPH_EDGES_HPP_
#define MIMIR_GRAPHS_GRAPH_EDGES_HPP_

#include "mimir/common/formatter.hpp"
#include "mimir/graphs/declarations.hpp"

#include <loki/loki.hpp>
#include <tuple>

namespace mimir::graphs
{

/// @brief `Edge` implements a directed edge with additional `EdgeProperties`.
/// @tparam ...EdgeProperties are additional edge properties.
template<Property P>
class Edge
{
public:
    using PropertyType = P;

    Edge(EdgeIndex index, VertexIndex source, VertexIndex target, P properties) :
        m_index(index),
        m_source(source),
        m_target(target),
        m_properties(std::move(properties))
    {
    }

    // Convenience: construct with only index if P is default-constructible
    template<class Q = P>
        requires std::is_same_v<Q, std::tuple<>>
    Edge(EdgeIndex index, VertexIndex source, VertexIndex target) : m_index(index), m_source(source), m_target(target), m_properties()
    {
    }

    EdgeIndex get_index() const { return m_index; }
    VertexIndex get_source() const { return m_source; }
    VertexIndex get_target() const { return m_target; }

    const P& get_properties() const { return m_properties; }

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_index(), get_source(), get_target(), std::cref(get_properties())); }

private:
    EdgeIndex m_index;
    VertexIndex m_source;
    VertexIndex m_target;
    [[no_unique_address]] P m_properties;
};
}

#endif
