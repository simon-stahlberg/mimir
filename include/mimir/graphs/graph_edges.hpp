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

#include "mimir/graphs/color.hpp"
#include "mimir/graphs/declarations.hpp"
#include "mimir/graphs/graph_edge_interface.hpp"

#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>

namespace mimir::graphs
{

/// @brief `Edge` implements a directed edge with additional `EdgeProperties`.
/// @tparam ...EdgeProperties are additional edge properties.
template<typename... EdgeProperties>
class Edge
{
public:
    using EdgePropertiesTypes = std::tuple<EdgeProperties...>;

    Edge(EdgeIndex index, VertexIndex source, VertexIndex target, EdgeProperties... properties) :
        m_index(index),
        m_source(source),
        m_target(target),
        m_properties(std::move(properties)...)
    {
    }

    EdgeIndex get_index() const { return m_index; }
    VertexIndex get_source() const { return m_source; }
    VertexIndex get_target() const { return m_target; }

    /// @brief Get a reference to the I-th `EdgeProperties`.
    /// We recommend providing free inline functions to access properties with more meaningful names.
    /// @tparam I the index of the property in the parameter pack.
    /// @return a reference to the I-th property.
    template<size_t I>
    const auto& get_property() const
    {
        static_assert(I < sizeof...(EdgeProperties), "Index out of bounds for EdgeProperties");
        return std::get<I>(m_properties);
    }

    const std::tuple<EdgeProperties...>& get_properties() const { return m_properties; }

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_index(), get_source(), get_target(), std::cref(get_properties())); }

private:
    EdgeIndex m_index;
    VertexIndex m_source;
    VertexIndex m_target;
    std::tuple<EdgeProperties...> m_properties;
};

template<typename... EdgeProperties>
std::ostream& operator<<(std::ostream& os, const Edge<EdgeProperties...>& edge)
{
    mimir::operator<<(os, edge.get_properties());
    return os;
}

/**
 * EmptyEdge
 */

/// @brief `EmptyEdge` is an edge without `EdgeProperties`.
using EmptyEdge = Edge<>;

/**
 * ColoredEdge
 */

using ColoredEdge = Edge<Color>;

inline const auto& get_color(const Edge<Color>& edge) { return edge.get_property<0>(); }

}

#endif
