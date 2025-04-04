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

#ifndef MIMIR_DATASETS_STATE_SPACE_OPTIONS_HPP_
#define MIMIR_DATASETS_STATE_SPACE_OPTIONS_HPP_

#include <cstdint>
#include <limits>

namespace mimir::datasets::state_space
{
struct Options
{
    bool sort_ascending_by_num_states;
    bool symmetry_pruning;
    bool remove_if_unsolvable;
    uint32_t max_num_states;
    uint32_t timeout_ms;

    Options() :
        sort_ascending_by_num_states(true),
        symmetry_pruning(false),
        remove_if_unsolvable(true),
        max_num_states(std::numeric_limits<uint32_t>::max()),
        timeout_ms(std::numeric_limits<uint32_t>::max())
    {
    }
};
}

#endif
