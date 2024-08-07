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

#include "mimir/graphs/graph_edge_interface.hpp"

#include <loki/loki.hpp>

namespace mimir
{

/**
 * Declarations
 */

/// @brief `BaseEdge` implements the `IsEdge` concept and acts as a common base class for concrete edge types.
/// @tparam Derived_ the concrete edge type.
template<typename Derived_>
class BaseEdge
{
public:
    BaseEdge(EdgeIndex index, VertexIndex source, VertexIndex target);

    bool operator==(const BaseEdge& other) const;
    size_t hash() const;

    EdgeIndex get_index() const;
    VertexIndex get_source() const;
    VertexIndex get_target() const;

private:
    EdgeIndex m_index;
    VertexIndex m_source;
    VertexIndex m_target;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }
};

/// @brief EmptyPropertiesEdge stores no additional edge properties.
class EmptyPropertiesEdge : public BaseEdge<EmptyPropertiesEdge>
{
public:
    EmptyPropertiesEdge(EdgeIndex index, VertexIndex source, VertexIndex target);

private:
    bool is_equal_impl(const BaseEdge<EmptyPropertiesEdge>& other) const;
    size_t hash_impl() const;

    friend class BaseEdge<EmptyPropertiesEdge>;
};

/// @brief ColoredEdge has an additional color property.
class ColoredEdge : public BaseEdge<ColoredEdge>
{
public:
    ColoredEdge(EdgeIndex index, VertexIndex source, VertexIndex target, Color color);

    Color get_color() const;

private:
    Color m_color;

    bool is_equal_impl(const BaseEdge<ColoredEdge>& other) const;
    size_t hash_impl() const;

    friend class BaseEdge<ColoredEdge>;
};

/**
 * Implementations
 */

template<typename Derived_>
BaseEdge<Derived_>::BaseEdge(EdgeIndex index, VertexIndex source, VertexIndex target) : m_index(index), m_source(source), m_target(target)
{
}

template<typename Derived_>
bool BaseEdge<Derived_>::operator==(const BaseEdge& other) const
{
    return (m_index == other.m_index) && (m_source == other.m_source) && (m_target == other.m_target) && self().is_equal_impl(other);
}

template<typename Derived_>
size_t BaseEdge<Derived_>::hash() const
{
    return loki::hash_combine(m_index, m_source, m_target, self().hash_impl());
}

template<typename Derived_>
EdgeIndex BaseEdge<Derived_>::get_index() const
{
    return m_index;
}

template<typename Derived_>
VertexIndex BaseEdge<Derived_>::get_source() const
{
    return m_source;
}

template<typename Derived_>
VertexIndex BaseEdge<Derived_>::get_target() const
{
    return m_target;
}

}

#endif
