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

#ifndef MIMIR_SEARCH_ALGORITHMS_STRATEGIES_LOOKAHEAD_STRATEGY_HPP_
#define MIMIR_SEARCH_ALGORITHMS_STRATEGIES_LOOKAHEAD_STRATEGY_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir::search
{

/// @brief `ILookaheadStrategy` encapsulates logic to perform greedy lookahead during search.
class ILookaheadStrategy
{
public:
    virtual ~ILookaheadStrategy() = default;

    /// @brief Pass search execution to the lookahead strategy
    /// @param state is the seed state of the lookahead.
    /// @return
    virtual bool execute_lookahead(State state) = 0;
};

/// @brief `LookaheadStrategyBase` implements a common base class to perform greedy lookahead rollouts during search.
/// @tparam Derived is the type of the concrete derived class.
template<typename Derived>
class LookaheadStrategyBase
{
};

}

#endif