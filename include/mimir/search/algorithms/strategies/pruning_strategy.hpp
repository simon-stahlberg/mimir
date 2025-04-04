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

#ifndef MIMIR_SEARCH_ALGORITHMS_STRATEGIES_PRUNING_STRATEGY_HPP_
#define MIMIR_SEARCH_ALGORITHMS_STRATEGIES_PRUNING_STRATEGY_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir::search
{

/// @brief `IPruningStrategy` encapsulates logic to test whether a newly generated state must be pruned or not.
class IPruningStrategy
{
public:
    virtual ~IPruningStrategy() = default;

    virtual bool test_prune_initial_state(State state) = 0;
    virtual bool test_prune_successor_state(State state, State succ_state, bool is_new_succ) = 0;
};

/// @brief `NoPruningStrategyImpl` never prunes a newly generated state.
class NoPruningStrategyImpl : public IPruningStrategy
{
public:
    bool test_prune_initial_state(State state) override;
    bool test_prune_successor_state(State state, State succ_state, bool is_new_succ) override;

    static NoPruningStrategy create();
};

/// @brief `DuplicatePruningStrategyImpl` prunes a newly generated state if it was already generated before.
class DuplicatePruningStrategyImpl : public IPruningStrategy
{
public:
    bool test_prune_initial_state(State state) override;
    bool test_prune_successor_state(State state, State succ_state, bool is_new_succ) override;

    static DuplicatePruningStrategy create();
};
}

#endif
