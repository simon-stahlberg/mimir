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

#include <loki/loki.hpp>

namespace mimir
{

/**
 * Declarations
 */

/// @brief BaseVertex implements the IsVertex concept and acts as a common base class for concrete vertex types.
/// @tparam Derived_ the concrete vertex type.
template<typename Derived_>
class BaseVertex
{
public:
    explicit BaseVertex(VertexIndex index);

    bool operator==(const BaseVertex& other) const;
    size_t hash() const;

    VertexIndex get_index() const;

private:
    VertexIndex m_index;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }
};

/// @brief EmptyPropertiesVertex stores no additional edge properties.
class EmptyPropertiesVertex : public BaseVertex<EmptyPropertiesVertex>
{
private:
    bool is_equal_impl(const EmptyPropertiesVertex& other) const;
    size_t hash_impl() const;

    friend class BaseVertex<EmptyPropertiesVertex>;

public:
    EmptyPropertiesVertex(VertexIndex index);
};

/// @brief ColoredVertex has an additional color property.
class ColoredVertex : public BaseVertex<ColoredVertex>
{
public:
    ColoredVertex(VertexIndex index, Color color);

    Color get_color() const;

private:
    Color m_color;

    bool is_equal_impl(const ColoredVertex& other) const;
    size_t hash_impl() const;

    friend class BaseVertex<ColoredVertex>;
};

/**
 * Implementations
 */

template<typename Derived_>
BaseVertex<Derived_>::BaseVertex(VertexIndex index) : m_index(index)
{
}

template<typename Derived_>
bool BaseVertex<Derived_>::operator==(const BaseVertex& other) const
{
    return (m_index == other.m_index) && self().is_equal_impl(other);
}

template<typename Derived_>
size_t BaseVertex<Derived_>::hash() const
{
    return loki::hash_combine(m_index, self().hash_impl());
}

template<typename Derived_>
VertexIndex BaseVertex<Derived_>::get_index() const
{
    return m_index;
}

}

#endif
