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

#ifndef MIMIR_GRAPHS_GRAPH_TRAVERSAL_INTERFACE_HPP_
#define MIMIR_GRAPHS_GRAPH_TRAVERSAL_INTERFACE_HPP_

#include "mimir/common/concepts.hpp"

#include <concepts>

namespace mimir::graphs
{

struct BackwardTag;

/// @brief `ForwardTag` represents traversal of edges in forward direction.
struct ForwardTag
{
    using Inverse = BackwardTag;
};
/// @brief `BackwardTag` represents traversal of edges in backward direction.
struct BackwardTag
{
    using Inverse = ForwardTag;
};

template<typename T>
concept IsDirection = std::same_as<T, ForwardTag> || std::same_as<T, BackwardTag>;

/// @brief `DirectionTaggedType` associates a type `T` with a `IsDirection`.
/// @tparam T is the type.
/// @tparam Direction is the associated direction.
template<typename T, IsDirection Direction>
class DirectionTaggedType
{
public:
    using DirectionType = Direction;

    explicit DirectionTaggedType(T& data, Direction) : m_data(data) {}

    const T& get() const { return m_data.get(); }
    T& get() { return m_data.get(); }

private:
    std::reference_wrapper<T> m_data;
};

}

#endif
