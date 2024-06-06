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

#ifndef MIMIR_SEARCH_ALGORITHMS_IW_HPP_
#define MIMIR_SEARCH_ALGORITHMS_IW_HPP_

#include "mimir/search/algorithms/event_handlers.hpp"
#include "mimir/search/algorithms/interface.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/successor_state_generators.hpp"

namespace mimir
{

/**
 * Large k for IW(k) is generally infeasible.
 * Hence, we use a small constant values within feasible range
 * allowing us to use stack allocated arrays.
 */

const int MAX_ARITY = 4;

/**
 * Type aliases for readability
 */

using AtomIndex = int;
using AtomIndices = std::vector<AtomIndex>;

using TupleIndex = int;
using TupleIndices = std::vector<TupleIndex>;

/// @brief TupleIndexMapper defines a perfect hash function mapping tuples of atom indices to a tuple index.
class TupleIndexMapper
{
private:
    int m_num_atoms;
    int m_arity;

    int m_factors[MAX_ARITY];

public:
    TupleIndexMapper(int num_atoms, int arity);

    TupleIndex to_tuple_index(const AtomIndices& atom_indices) const;

    void to_atom_indices(TupleIndex tuple_index, AtomIndices& out_atom_indices) const;

    std::string tuple_index_to_string(TupleIndex tuple_index) const;

    /**
     * Getters
     */
    int get_num_atoms() const;
    int get_arity() const;
    const int* get_factors() const;
};

/// @brief TupleIndexGenerator encapsulates logic to generate all combinations of tuple indices
/// of size that is at most the arity defined in the TupleIndexMapper.
class SingleStateTupleIndexGenerator
{
private:
    const TupleIndexMapper* m_tuple_index_mapper;
    const AtomIndices* m_atom_indices;

public:
    SingleStateTupleIndexGenerator(const TupleIndexMapper& tuple_index_mapper, const AtomIndices& atom_indices);

    class const_iterator
    {
    private:
        const TupleIndexMapper* m_tuple_index_mapper;
        const AtomIndices* m_atom_indices;

        int m_cur;

        int m_indices[MAX_ARITY];

        void next_tuple_index();

    public:
        using difference_type = int;
        using value_type = TupleIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(const TupleIndexMapper& tuple_index_mapper, const AtomIndices& atom_indices, bool begin);
        [[nodiscard]] value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        [[nodiscard]] bool operator==(const const_iterator& other) const;
        [[nodiscard]] bool operator!=(const const_iterator& other) const;
    };

    const_iterator begin() const;
    const_iterator end() const;
};

/// @brief TupleIndexGenerator encapsulates logic to generate all combinations of tuple indices
/// of size that is at most the arity defined in the TupleIndexMapper.
class CombinedStateTupleIndexGenerator
{
private:
    const TupleIndexMapper* m_tuple_index_mapper;
    const AtomIndices* m_atom_indices;
    const AtomIndices* m_add_atom_indices;

public:
    CombinedStateTupleIndexGenerator(const TupleIndexMapper& tuple_index_mapper, const AtomIndices& atom_indices, const AtomIndices& add_atom_indices);

    class const_iterator
    {
    private:
        /* Data */
        const TupleIndexMapper* m_tuple_index_mapper;
        const AtomIndices* m_a_atom_indices[2];
        int m_a_num_atom_indices[2];

        /* Iterator positions */
        bool m_end_inner;
        int m_cur_inner;
        bool m_end_outter;
        int m_cur_outter;

        /* Iterator positions explicit representation */
        // m_a[i] = 0 => pick from atom_indices, m_a[i] = 1 => pick from add_atom_indices
        int m_a[MAX_ARITY];
        // m_a_geq[i][j] = k =>
        int m_a_geq[2][MAX_ARITY];

        int m_indices[MAX_ARITY];

        bool next_outter_begin();

        void next_tuple_index();

    public:
        using difference_type = int;
        using value_type = TupleIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(const TupleIndexMapper& tuple_index_mapper, const AtomIndices& atom_indices, const AtomIndices& add_atom_indices, bool begin);
        [[nodiscard]] value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        [[nodiscard]] bool operator==(const const_iterator& other) const;
        [[nodiscard]] bool operator!=(const const_iterator& other) const;
    };

    const_iterator begin() const;
    const_iterator end() const;
};

/// @brief DynamicNoveltyTable encapsulates a table to test novelty of tuples of atoms of size at most arity.
/// It automatically resizes when the atoms do not fit into the table anymore.
class DynamicNoveltyTable
{
private:
    TupleIndexMapper m_tuple_index_mapper;

public:
    explicit DynamicNoveltyTable(size_t arity);

    bool test_novelty_and_update_table(const State state);

    bool test_novelty_and_update_table(const State state, const State succ_state);
};

// We could in principle factor out some general functionality in BrFs, derive from BrFS, and override the functionality here.
class IterativeWidthAlgorithm : public IAlgorithm
{
private:
public:
    /// @brief Simplest construction
    IterativeWidthAlgorithm(std::shared_ptr<IDynamicAAG> applicable_action_generator, size_t max_arity);

    /// @brief Complete construction
    IterativeWidthAlgorithm(std::shared_ptr<IDynamicAAG> applicable_action_generator,
                            std::shared_ptr<IDynamicSSG> successor_state_generator,
                            std::shared_ptr<IAlgorithmEventHandler> event_handler,
                            size_t max_arity);

    SearchStatus find_solution(GroundActionList& out_plan) override {}
};
}

#endif
