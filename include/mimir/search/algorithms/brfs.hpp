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

#ifndef MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_
#define MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/algorithms/utils.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/state.hpp"

#include <memory>
#include <optional>
#include <vector>

namespace mimir::search::brfs
{
struct Options
{
    std::optional<State> start_state = std::nullopt;
    EventHandler event_handler = nullptr;
    GoalStrategy goal_strategy = nullptr;
    PruningStrategy pruning_strategy = nullptr;
    bool stop_if_goal = true;
    uint32_t max_num_states = std::numeric_limits<uint32_t>::max();
    uint32_t max_time_in_ms = std::numeric_limits<uint32_t>::max();

    Options() = default;
};

extern SearchResult find_solution(const SearchContext& context, const Options& options = Options());

}

#endif
