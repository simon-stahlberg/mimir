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

#ifndef MIMIR_GRAPHS_GRAPH_VERTEX_INTERFACE_HPP_
#define MIMIR_GRAPHS_GRAPH_VERTEX_INTERFACE_HPP_

#include <concepts>
#include <cstdint>
#include <ranges>
#include <span>
#include <vector>

namespace mimir
{

using VertexIndex = uint32_t;

/// @brief Each vertex must be associated with an index.
template<typename T>
concept IsVertex = requires(T a) {
    {
        a.get_index()
    } -> std::convertible_to<VertexIndex>;
};

}

#endif