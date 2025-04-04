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

#include "mimir/search/algorithms/strategies/pruning_strategy.hpp"

using namespace mimir::formalism;

namespace mimir::search
{

/* NoPruningStrategyImpl */
bool NoPruningStrategyImpl::test_prune_initial_state(const State state) { return false; }

bool NoPruningStrategyImpl::test_prune_successor_state(const State state, const State succ_state, bool is_new_succ) { return false; }

NoPruningStrategy NoPruningStrategyImpl::create() { return std::make_shared<NoPruningStrategyImpl>(); }

/* DuplicatePruningStrategyImpl */
bool DuplicatePruningStrategyImpl::test_prune_initial_state(const State state) { return false; };

bool DuplicatePruningStrategyImpl::test_prune_successor_state(const State state, const State succ_state, bool is_new_succ) { return !is_new_succ; }

DuplicatePruningStrategy DuplicatePruningStrategyImpl::create() { return std::make_shared<DuplicatePruningStrategyImpl>(); }
}
