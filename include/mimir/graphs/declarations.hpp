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

#ifndef MIMIR_GRAPHS_DECLARATIONS_HPP_
#define MIMIR_GRAPHS_DECLARATIONS_HPP_

#include <cstdint>
#include <limits>
#include <vector>

namespace mimir
{

using VertexIndex = uint32_t;
using VertexIndexList = std::vector<VertexIndex>;

using EdgeIndex = uint32_t;
using EdgeIndexList = std::vector<EdgeIndex>;
using EdgeCost = double;

using Distance = double;
using DistanceList = std::vector<Distance>;
using DistanceMatrix = std::vector<DistanceList>;
inline const Distance DISTANCE_INFINITY = std::numeric_limits<Distance>::max();

using Degree = uint32_t;
using DegreeList = std::vector<Degree>;

using Color = uint32_t;
using ColorList = std::vector<Color>;

}

#endif
