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

#ifndef MIMIR_SEARCH_ALGORITHMS_SIW_HPP_
#define MIMIR_SEARCH_ALGORITHMS_SIW_HPP_

#include "mimir/search/algorithms/iw.hpp"
#include "mimir/search/algorithms/iw/types.hpp"

namespace mimir::search::siw
{

extern SearchResult find_solution(const SearchContext& context,
                                  State start_state = nullptr,
                                  size_t max_arity = iw::MAX_ARITY - 1,
                                  EventHandler siw_event_handler = nullptr,
                                  iw::EventHandler iw_event_handler = nullptr,
                                  brfs::EventHandler brfs_event_handler = nullptr,
                                  GoalStrategy goal_strategy = nullptr);
}

#endif
