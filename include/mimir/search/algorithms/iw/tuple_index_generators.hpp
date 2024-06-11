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

#ifndef MIMIR_SEARCH_ALGORITHMS_IW_TUPLE_INDEX_GENERATORS_HPP_
#define MIMIR_SEARCH_ALGORITHMS_IW_TUPLE_INDEX_GENERATORS_HPP_

#include "mimir/search/algorithms/iw/index_mappers.hpp"
#include "mimir/search/algorithms/iw/types.hpp"
#include "mimir/search/state.hpp"

#include <array>
#include <memory>

namespace mimir
{

/// @brief StateTupleIndexGenerator encapsulates iterator logic to generate
/// all combinations of tuple indices of size at most arity.
///
/// The underlying algorithm is an adaption of std::next_permutation
/// with constant amortized cost to compute the next tuple index.
class StateTupleIndexGenerator
{
private:
    std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper;
    std::shared_ptr<TupleIndexMapper> tuple_index_mapper;

    // Preallocated memory for reuse
    AtomIndexList atom_indices;

    friend class const_iterator;

public:
    StateTupleIndexGenerator(std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper, std::shared_ptr<TupleIndexMapper> tuple_index_mapper);

    class const_iterator
    {
    private:
        /* External data */
        const TupleIndexMapper* m_tuple_index_mapper;
        const AtomIndexList* m_atom_indices;

        /* Internal data */
        std::array<int, MAX_ARITY> m_indices;
        bool m_end;
        int m_cur;

        void advance();

    public:
        using difference_type = int;
        using value_type = TupleIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(StateTupleIndexGenerator* data, bool begin);
        [[nodiscard]] value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        [[nodiscard]] bool operator==(const const_iterator& other) const;
        [[nodiscard]] bool operator!=(const const_iterator& other) const;
    };

    const_iterator begin(const State state);
    const_iterator begin(const AtomIndexList& atom_indices);  // for testing only
    const_iterator end() const;
};

/// @brief StatePairTupleIndexGenerator encapsulates iterator logic to generate
/// all combinations of tuple indices of size at most the arity
/// such that at least one atom index is coming from add_atom_indices
/// and the others from atom_indices.
///
/// The underlying algorithm is an adaption of std::next_permutation
/// with constant amortized cost to compute the next tuple index.
class StatePairTupleIndexGenerator
{
private:
    std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper;
    std::shared_ptr<TupleIndexMapper> tuple_index_mapper;

    // Preallocated memory for reuse
    std::array<std::vector<int>, 2> a_index_jumper;
    std::array<AtomIndexList, 2> a_atom_indices;

    friend class const_iterator;

public:
    StatePairTupleIndexGenerator(std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper, std::shared_ptr<TupleIndexMapper> tuple_index_mapper);

    class const_iterator
    {
    private:
        /* External data */
        const TupleIndexMapper* m_tuple_index_mapper;
        const std::array<AtomIndexList, 2>* m_a_atom_indices;
        std::array<std::vector<int>, 2>* m_a_index_jumper;

        /* Internal data */
        std::array<int, MAX_ARITY> m_indices;
        std::array<bool, MAX_ARITY> m_a;
        int m_cur_outter;
        int m_cur_inner;
        bool m_end_outter;
        bool m_end_inner;

        static const int UNDEFINED;

        // O(N)
        void initialize_index_jumper();
        // O(K)
        int find_rightmost_incrementable_index();
        // O(1)
        int find_new_index(int i);
        // O(K*2^K)
        bool advance_outter();
        // Outter iteration O(K*2^K), inner iteration amortized O(1) for O(N^K) iterations
        void advance_inner();

    public:
        using difference_type = int;
        using value_type = TupleIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(StatePairTupleIndexGenerator* sptig, bool begin);
        [[nodiscard]] value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        [[nodiscard]] bool operator==(const const_iterator& other) const;
        [[nodiscard]] bool operator!=(const const_iterator& other) const;
    };

    const_iterator begin(const State state, const State succ_state);
    const_iterator begin(const AtomIndexList& atom_indices, const AtomIndexList& add_atom_indices);  // for testing only
    const_iterator end() const;
};

}

#endif
