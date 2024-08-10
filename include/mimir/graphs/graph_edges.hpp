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

#include "mimir/common/hash_utils.hpp"
#include "mimir/graphs/graph_edge_interface.hpp"

namespace mimir
{

/// @brief `Edge` implements a directed edge with additional `EdgeProperties`
/// @tparam Tag is an empty struct used for dispatching.
/// @tparam ...EdgeProperties are additional edge properties.
template<typename Tag, typename... EdgeProperties>
class Edge
{
public:
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

    bool operator==(const Edge& other) const
    {
        if (this != &other)
        {
            return (m_index == other.m_index) && (m_source == other.m_source) && (m_target == other.m_target) && (m_properties == other.m_properties);
        }
        return true;
    }

    size_t hash() const
    {
        size_t seed = hash_combine(m_index, m_source, m_target);
        apply_properties_hash(seed, std::make_index_sequence<sizeof...(EdgeProperties)> {});
        return seed;
    }

    /// @brief Return a const reference to the I-th `EdgeProperties`.
    /// We recommend providing free inline functions to access properties with more meaningful names.
    /// @tparam I the index of the property in the parameter pack.
    /// @return a reference to the I-th property.
    template<size_t I>
    const auto& get_property() const
    {
        static_assert(I < sizeof...(EdgeProperties), "Index out of bounds for EdgeProperties");
        return std::get<I>(m_properties);
    }

private:
    EdgeIndex m_index;
    VertexIndex m_source;
    VertexIndex m_target;
    std::tuple<EdgeProperties...> m_properties;

    // Helper function to apply hashing to all properties
    template<std::size_t... Is>
    void apply_properties_hash(size_t& seed, std::index_sequence<Is...>) const
    {
        (..., mimir::hash_combine(seed, Hash<std::tuple_element_t<Is, std::tuple<EdgeProperties...>>>()(get_property<Is>())));
    }
};

/**
 * EmptyEdge
 */

struct EmptyEdgeTag
{
};

using EmptyEdge = Edge<EmptyEdgeTag>;

/**
 * ColoredEdge
 */

struct ColoredEdgeTag
{
};

using ColoredEdge = Edge<ColoredEdgeTag, Color>;

inline Color get_color(const ColoredEdge& edge) { return edge.get_property<0>(); }

}

#endif
