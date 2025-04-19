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

#ifndef MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
#define MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_

#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/algorithms/utils.hpp"
#include "mimir/search/declarations.hpp"

#include <memory>
#include <optional>
#include <vector>

namespace mimir::search::astar
{

extern SearchResult find_solution(const SearchContext& context,
                                  const Heuristic& heuristic,
                                  State start_state = nullptr,
                                  EventHandler event_handler = nullptr,
                                  GoalStrategy goal_strategy = nullptr,
                                  PruningStrategy pruning_strategy = nullptr);

}

#endif
