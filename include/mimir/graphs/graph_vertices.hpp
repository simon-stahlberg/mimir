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

#include "mimir/common/hash_utils.hpp"
#include "mimir/graphs/graph_vertex_interface.hpp"

#include <tuple>

namespace mimir
{

/// @brief `Vertex` implements a vertex with additional `VertexProperties`
/// @tparam Tag is an empty struct used for dispatching.
/// @tparam ...VertexProperties are additional vertex properties.
template<typename Tag, typename... VertexProperties>
class Vertex
{
public:
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

    size_t hash() const
    {
        size_t seed = 0;
        mimir::hash_combine(seed, m_index);
        apply_properties_hash(seed, std::make_index_sequence<sizeof...(VertexProperties)> {});
        return seed;
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

    // Helper function to apply hashing to all properties
    template<std::size_t... Is>
    void apply_properties_hash(size_t& seed, std::index_sequence<Is...>) const
    {
        (..., mimir::hash_combine(seed, Hash<std::tuple_element_t<Is, std::tuple<VertexProperties...>>>()(get_property<Is>())));
    }
};

/**
 * EmptyVertex
 */

struct EmptyVertexTag
{
};

using EmptyVertex = Vertex<EmptyVertexTag>;

/**
 * ColoredVertex
 */

struct ColoredVertexTag
{
};

using ColoredVertex = Vertex<ColoredVertexTag, Color>;

inline Color get_color(const ColoredVertex& vertex) { return vertex.get_property<0>(); }

}

#endif
