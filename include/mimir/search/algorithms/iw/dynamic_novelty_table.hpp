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

#ifndef MIMIR_SEARCH_ALGORITHMS_IW_DYNAMIC_NOVELTY_TABLE_HPP_
#define MIMIR_SEARCH_ALGORITHMS_IW_DYNAMIC_NOVELTY_TABLE_HPP_

#include "mimir/search/algorithms/iw/index_mappers.hpp"
#include "mimir/search/algorithms/iw/tuple_index_generators.hpp"
#include "mimir/search/algorithms/iw/types.hpp"
#include "mimir/search/state.hpp"

namespace mimir
{

/// @brief DynamicNoveltyTable encapsulates a table to test novelty of tuples of atoms of size at most arity.
/// It automatically resizes when the atoms do not fit into the table anymore.
class DynamicNoveltyTable
{
private:
    std::shared_ptr<FluentAndDerivedMapper> m_atom_index_mapper;
    std::shared_ptr<TupleIndexMapper> m_tuple_index_mapper;

    std::vector<bool> m_table;

    void resize_to_fit(int atom_index);

    // Preallocated memory that will be modified.
    StateTupleIndexGenerator m_state_tuple_index_generator;
    StatePairTupleIndexGenerator m_state_pair_tuple_index_generator;

public:
    DynamicNoveltyTable(std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper, std::shared_ptr<TupleIndexMapper> tuple_index_mapper);

    void compute_novel_tuple_indices(const State state, TupleIndexList& out_novel_tuple_indices);

    void insert_tuple_indices(const TupleIndexList& tuple_indices);

    bool test_novelty_and_update_table(const State state);

    bool test_novelty_and_update_table(const State state, const State succ_state);

    void reset();
};

}

#endif
