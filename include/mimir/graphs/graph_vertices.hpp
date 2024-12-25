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

#ifndef MIMIR_GRAPHS_GRAPH_VERTICES_HPP_
#define MIMIR_GRAPHS_GRAPH_VERTICES_HPP_

#include "mimir/graphs/graph_vertex_interface.hpp"

#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>
#include <tuple>

namespace mimir
{

/// @brief `Vertex` implements a vertex with additional `VertexProperties`.
/// See examples on how to define vertices below.
/// @tparam ...VertexProperties are additional vertex properties.
template<typename... VertexProperties>
class Vertex
{
public:
    using VertexPropertiesTypes = std::tuple<VertexProperties...>;

    Vertex(VertexIndex index, VertexProperties... properties) : m_index(index), m_properties(std::move(properties)...) {}

    VertexIndex get_index() const { return m_index; }

    template<size_t I>
    const auto& get_property() const
    {
        static_assert(I < sizeof...(VertexProperties), "Index out of bounds for VertexProperties");
        return std::get<I>(m_properties);
    }

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifiable_members() const { return std::forward_as_tuple(std::as_const(m_index), std::as_const(m_properties)); }

private:
    VertexIndex m_index;
    std::tuple<VertexProperties...> m_properties;
};

/**
 * EmptyVertex
 */

/// @brief `EmptyVertex` is a vertex without `VertexProperties`.
using EmptyVertex = Vertex<>;

/**
 * ColoredVertex
 */

/// @brief `ColoredVertex` is a vertex with a color `VertexProperties`.
/// For readability of code that uses a `ColoredVertex`, we provide a free function get_color to access the color of a given vertex.
using ColoredVertex = Vertex<Color>;

/// @brief Get the color of a colored vertex.
/// @param edge the colored vertex.
/// @return the color of the vertex.
inline Color get_color(const ColoredVertex& vertex) { return vertex.get_property<0>(); }

template<typename T>
concept IsVertexColoredGraph = requires(T::VertexType vertex) {
    {
        get_color(vertex)
    } -> std::same_as<Color>;
};

}

#endif
