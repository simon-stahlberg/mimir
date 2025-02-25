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

#ifndef MIMIR_SEARCH_ALGORITHMS_IW_HPP_
#define MIMIR_SEARCH_ALGORITHMS_IW_HPP_

#include "mimir/search/algorithms/brfs.hpp"

namespace mimir
{

extern SearchResult find_solution_iw(const SearchContext& context,
                                     std::optional<State> start_state = std::nullopt,
                                     std::optional<size_t> max_arity = std::nullopt,
                                     std::optional<IWAlgorithmEventHandler> iw_event_handler = std::nullopt,
                                     std::optional<BrFSAlgorithmEventHandler> brfs_event_handler = std::nullopt,
                                     std::optional<GoalStrategy> goal_strategy = std::nullopt);
}

#endif