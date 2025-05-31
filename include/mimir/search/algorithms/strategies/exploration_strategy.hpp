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

#ifndef MIMIR_SEARCH_ALGORITHMS_STRATEGIES_EXPLORATION_STRATEGY_HPP_
#define MIMIR_SEARCH_ALGORITHMS_STRATEGIES_EXPLORATION_STRATEGY_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir::search
{

/// @brief `IExplorationStrategy` encapsulates logic to perform policy execution during search.
class IExplorationStrategy
{
public:
    virtual ~IExplorationStrategy() = default;

    /// @brief Called when generating a state.
    /// @param state is the state.
    /// @param action is the action.
    /// @param successor_state is the successor state.
    /// @return return true iff the state transition is exploration compatible.
    virtual bool on_generate_state(State state, formalism::GroundAction action, State successor_state) = 0;
};

}

#endif