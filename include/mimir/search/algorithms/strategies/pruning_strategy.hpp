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

#include "mimir/formalism/formalism.hpp"
#include "mimir/search/state.hpp"

namespace mimir
{

/**
 * IPruningStrategy encapsulates logic to test whether a generated state must be pruned.
 */
class IPruningStrategy
{
public:
    virtual ~IPruningStrategy() = default;

    virtual bool test_prune_initial_state(const State state) = 0;
    virtual bool test_prune_successor_state(const State state, const State succ_state, bool is_new_succ) = 0;
};

class DuplicateStatePruning : public IPruningStrategy
{
public:
    bool test_prune_initial_state(const State state) override;
    bool test_prune_successor_state(const State state, const State succ_state, bool is_new_succ) override;
};
}

#endif
