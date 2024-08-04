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

#include <concepts>

namespace mimir
{

struct ForwardTraversal
{
};
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

}

#endif
