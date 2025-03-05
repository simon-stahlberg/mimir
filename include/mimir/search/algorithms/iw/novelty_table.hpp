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

#ifndef MIMIR_SEARCH_ALGORITHMS_IW_NOVELTY_TABLE_HPP_
#define MIMIR_SEARCH_ALGORITHMS_IW_NOVELTY_TABLE_HPP_

#include "mimir/search/algorithms/iw/tuple_index_generators.hpp"
#include "mimir/search/algorithms/iw/tuple_index_mapper.hpp"
#include "mimir/search/algorithms/iw/types.hpp"
#include "mimir/search/declarations.hpp"

#include <concepts>

namespace mimir
{

struct DynamicSize
{
};

struct StaticSize
{
};

template<typename T>
concept StaticOrDynamicSize = std::is_same_v<T, StaticSize> || std::is_same_v<T, DynamicSize>;

/// @brief NoveltyTable encapsulates a table to test novelty of tuples of atoms of size at most arity.
/// It automatically resizes when the atoms do not fit into the table anymore.
template<StaticOrDynamicSize Mode>
class NoveltyTable
{
private:
    TupleIndexMapper m_tuple_index_mapper;

    std::vector<bool> m_table;

    void resize_to_fit(AtomIndex atom_index);
    void resize_to_fit(State state);

    // Preallocated memory that will be modified.
    StateTupleIndexGenerator m_state_tuple_index_generator;
    StatePairTupleIndexGenerator m_state_pair_tuple_index_generator;

public:
    explicit NoveltyTable(size_t arity);
    NoveltyTable(size_t arity, size_t num_atoms);

    void compute_novel_tuple_indices(const State state, TupleIndexList& out_novel_tuple_indices);

    void insert_tuple_indices(const TupleIndexList& tuple_indices);

    bool test_novelty_and_update_table(const State state);

    bool test_novelty_and_update_table(const State state, const State succ_state);

    void reset();

    const TupleIndexMapper& get_tuple_index_mapper() const;
};

}

#endif