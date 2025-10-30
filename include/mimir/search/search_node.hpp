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

#ifndef MIMIR_SEARCH_SEARCH_NODE_HPP_
#define MIMIR_SEARCH_SEARCH_NODE_HPP_

#include "mimir/common/declarations.hpp"
#include "mimir/common/segmented_vector.hpp"

#include <tuple>

namespace mimir::search
{

enum SearchNodeStatus : uint8_t
{
    GOAL = 0,
    DEAD_END = 1,
    CLOSED = 2,
    OPEN = 3,
    NEW = 4,
};

template<typename T>
concept IsSearchNode = requires(const T a) {
    { a.parent_state } -> std::convertible_to<Index>;
};

}

#endif