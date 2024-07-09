/*
 * Copyright (C) 2024 Dominik Drexler and Till Hofmann
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

#ifndef MIMIR_DATASETS_SCC_ABSTRACTION_HPP_
#define MIMIR_DATASETS_SCC_ABSTRACTION_HPP_

#include "mimir/graphs/object_graph.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/successor_state_generator.hpp"

#include <cstdint>
#include <limits>
namespace mimir
{
class SccAbstractState
{
private:
    StateIndex m_index;
    std::span<State> m_states;
    ObjectGraphStaticPruningStrategy m_pruning_strategy;

public:
    SccAbstractState(StateIndex index, std::span<State> states, ObjectGraphStaticPruningStrategy pruning_strategy) :
        m_index(index),
        m_states(states),
        m_pruning_strategy(pruning_strategy)
    {
    }
};

class SccAbstraction
{
public:
    static std::optional<SccAbstraction> create(Problem problem,
                                                bool remove_if_unsolvable = true,
                                                uint32_t max_num_states = std::numeric_limits<uint32_t>::max(),
                                                uint32_t timeout_ms = std::numeric_limits<uint32_t>::max());
};
}

#endif
