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

namespace mimir
{

/**
 * We traverse graphs in forward and backward direction.
 * The direction is chosen by adding a traversal direction tag to the graph before passing it to the graph algorithm.
 */

/// @brief ForwardTraversal represents traversal of edges in forward direction.
struct ForwardTraversal
{
};
/// @brief BackwardTraversal represents traversal of edges in backward direction.
struct BackwardTraversal
{
};

template<typename T>
concept IsTraversalDirection = std::same_as<T, ForwardTraversal> || std::same_as<T, BackwardTraversal>;

/**
 * Type trait to obtain the opposite traversal direction.
 */

template<typename Direction>
struct InverseTraversalDirection;

template<>
struct InverseTraversalDirection<ForwardTraversal>
{
    using type = BackwardTraversal;
};

template<>
struct InverseTraversalDirection<BackwardTraversal>
{
    using type = ForwardTraversal;
};

/// @brief TraversalDirectionTaggedType associates a type T with a traversal direction.
/// @tparam T is the type to be associated.
/// @tparam Direction is the associated traversal direction.
template<typename T, IsTraversalDirection Direction>
class TraversalDirectionTaggedType
{
public:
    using DirectionType = Direction;

    explicit TraversalDirectionTaggedType(T& data, Direction) : m_data(data) {}

    const T& get() const { return m_data.get(); }
    T& get() { return m_data.get(); }

private:
    std::reference_wrapper<T> m_data;
};

/// @brief TraversalDirectionContainer contains a type T for forward and backward traversal.
/// @tparam T is the type that we want to store for forward and backward traversal.
template<typename T>
class TraversalDirectionStorage
{
public:
    template<IsTraversalDirection Direction>
    const T& get() const
    {
        if constexpr (std::is_same_v<Direction, ForwardTraversal>)
        {
            return m_forward_element;
        }
        else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
        {
            return m_backward_element;
        }
        else
        {
            static_assert(dependent_false<Direction>::value, "TraversalDirectionDependentStorage.get(): Missing implementation for IsTraversalDirection.");
        }
    }

    template<IsTraversalDirection Direction>
    T& get()
    {
        if constexpr (std::is_same_v<Direction, ForwardTraversal>)
        {
            return m_forward_element;
        }
        else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
        {
            return m_backward_element;
        }
        else
        {
            static_assert(dependent_false<Direction>::value, "TraversalDirectionDependentStorage.get(): Missing implementation for IsTraversalDirection.");
        }
    }

private:
    T m_forward_element;
    T m_backward_element;
};

}

#endif
