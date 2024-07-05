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

#ifndef MIMIR_DATASETS_TRANSITION_INTERFACE_HPP_
#define MIMIR_DATASETS_TRANSITION_INTERFACE_HPP_

#include "mimir/search/action.hpp"
#include "mimir/search/state.hpp"

#include <vector>

namespace mimir
{

using TransitionCost = double;
using TransitionCostList = std::vector<TransitionCost>;
using TransitionIndex = int;
using TransitionIndexList = std::vector<TransitionIndex>;

template<typename T>
concept IsTransition = requires(T a) {
    {
        a.get_index()
    } -> std::same_as<TransitionIndex>;
    {
        a.get_source_state()
    } -> std::same_as<StateIndex>;
    {
        a.get_target_state()
    } -> std::same_as<StateIndex>;
    {
        a.get_cost()
    } -> std::same_as<TransitionCost>;
};

}

#endif
