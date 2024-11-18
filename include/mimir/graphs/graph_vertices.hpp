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

#include "mimir/common/equal_to.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/graphs/graph_vertex_interface.hpp"

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

    bool operator==(const Vertex& other) const
    {
        if (this != &other)
        {
            return (m_index == other.m_index) && (m_properties == other.m_properties);
        }
        return true;
    }

    template<size_t I>
    const auto& get_property() const
    {
        static_assert(I < sizeof...(VertexProperties), "Index out of bounds for VertexProperties");
        return std::get<I>(m_properties);
    }

private:
    VertexIndex m_index;
    std::tuple<VertexProperties...> m_properties;
};
}

template<typename... VertexProperties>
struct std::hash<mimir::Vertex<VertexProperties...>>
{
    size_t operator()(const mimir::Vertex<VertexProperties...>& element) const
    {
        size_t seed = element.get_index();
        apply_properties_hash(seed, element, std::make_index_sequence<sizeof...(VertexProperties)> {});
        return seed;
    }

    // Helper function to apply hashing to all properties
    template<std::size_t... Is>
    void apply_properties_hash(size_t& seed, const mimir::Vertex<VertexProperties...>& element, std::index_sequence<Is...>) const
    {
        (..., mimir::hash_combine(seed, element.template get_property<Is>()));
    }
};

namespace mimir
{

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
concept IsVertexColoredGraph = requires(T::VertexType vertex)
{
    {
        get_color(vertex)
        } -> std::same_as<Color>;
};

}

#endif
