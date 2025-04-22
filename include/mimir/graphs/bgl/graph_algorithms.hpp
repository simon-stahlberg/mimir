/*
 * Copyright (C) 2024 Dominik Drexler and Till Hofmann
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

#ifndef MIMIR_GRAPHS_BGL_GRAPH_ALGORITHMS_HPP_
#define MIMIR_GRAPHS_BGL_GRAPH_ALGORITHMS_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/graphs/bgl/graph_adapters.hpp"
#include "mimir/graphs/bgl/graph_traits.hpp"
#include "mimir/graphs/bgl/property_maps.hpp"
#include "mimir/graphs/static_graph_interface.hpp"

#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/property_map/property_map.hpp>
#include <concepts>
#include <limits>
#include <ranges>

namespace mimir::graphs::bgl
{

}

#endif
