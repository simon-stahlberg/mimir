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

#include "mimir/common/equal_to.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/graphs/graph_edge_interface.hpp"

namespace mimir
{

/// @brief `Edge` implements a directed edge with additional `EdgeProperties`.
/// See examples on how to define edges below.
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

    bool operator==(const Edge& other) const
    {
        if (this != &other)
        {
            return (m_index == other.m_index) && (m_source == other.m_source) && (m_target == other.m_target) && (m_properties == other.m_properties);
        }
        return true;
    }

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

private:
    EdgeIndex m_index;
    VertexIndex m_source;
    VertexIndex m_target;
    std::tuple<EdgeProperties...> m_properties;
};
}

template<typename... EdgeProperties>
struct std::hash<mimir::Edge<EdgeProperties...>>
{
    size_t operator()(const mimir::Edge<EdgeProperties...>& element) const
    {
        size_t seed = mimir::hash_combine(element.get_index(), element.get_source(), element.get_target());
        apply_properties_hash(seed, element, std::make_index_sequence<sizeof...(EdgeProperties)> {});
        return seed;
    }

    // Helper function to apply hashing to all properties
    template<std::size_t... Is>
    void apply_properties_hash(size_t& seed, const mimir::Edge<EdgeProperties...>& element, std::index_sequence<Is...>) const
    {
        (..., mimir::hash_combine(seed, element.template get_property<Is>()));
    }
};

namespace mimir
{

/**
 * EmptyEdge
 */

/// @brief `EmptyEdge` is an edge without `EdgeProperties`.
using EmptyEdge = Edge<>;

/**
 * ColoredEdge
 */

/// @brief `ColoredEdge` is an edge with a color `EdgeProperties`.
/// For readability of code that uses a `ColoredEdge`, we provide a free function get_color to access the color of a given edge.
using ColoredEdge = Edge<Color>;

/// @brief Get the color of a colored edge.
/// @param edge the colored edge.
/// @return the color of the edge.
inline Color get_color(const ColoredEdge& edge) { return edge.get_property<0>(); }

template<typename T>
concept IsEdgeColoredGraph = requires(T::EdgeType edge)
{
    {
        get_color(edge)
        } -> std::same_as<Color>;
};

}

#endif
