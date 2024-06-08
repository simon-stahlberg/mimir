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

#ifndef MIMIR_SEARCH_ALGORITHMS_IW_PRUNING_STRATEGY_HPP_
#define MIMIR_SEARCH_ALGORITHMS_IW_PRUNING_STRATEGY_HPP_

#include "mimir/search/algorithms/iw/dynamic_novelty_table.hpp"
#include "mimir/search/algorithms/iw/index_mappers.hpp"
#include "mimir/search/algorithms/iw/types.hpp"
#include "mimir/search/algorithms/strategies/pruning_strategy.hpp"

#include <memory>
#include <unordered_set>

namespace mimir
{
class ArityZeroNoveltyPruning : public IPruningStrategy
{
private:
    State m_initial_state;

public:
    explicit ArityZeroNoveltyPruning(State initial_state);

    bool test_prune_initial_state(const State state) override;
    bool test_prune_successor_state(const State state, const State succ_state, bool is_new_succ) override;
};

class ArityKNoveltyPruning : public IPruningStrategy
{
private:
    DynamicNoveltyTable m_novelty_table;

    std::unordered_set<int> m_generated_states;

public:
    explicit ArityKNoveltyPruning(int arity, int num_atoms, std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper);

    bool test_prune_initial_state(const State state) override;
    bool test_prune_successor_state(const State state, const State succ_state, bool is_new_succ) override;
};
}

#endif
