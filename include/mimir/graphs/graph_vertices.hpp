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

#include "mimir/graphs/color.hpp"
#include "mimir/graphs/declarations.hpp"
#include "mimir/graphs/graph_vertex_interface.hpp"

#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>
#include <tuple>

namespace mimir::graphs
{

/// @brief `Vertex` implements a vertex with additional `VertexProperties`.
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

    const std::tuple<VertexProperties...>& get_properties() const { return m_properties; }

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_index(), std::cref(get_properties())); }

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

using ColoredVertex = Vertex<Color>;

inline const auto& get_color(const Vertex<Color>& vertex) { return vertex.get_property<0>(); }

}

#endif
