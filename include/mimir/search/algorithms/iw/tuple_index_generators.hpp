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

#include "mimir/search/algorithms/iw/tuple_index_mapper.hpp"
#include "mimir/search/algorithms/iw/types.hpp"
#include "mimir/search/declarations.hpp"

#include <array>
#include <memory>

namespace mimir::search::iw
{

/// @brief StateTupleIndexGenerator encapsulates iterator logic to generate
/// all combinations of tuple indices of size at most arity.
///
/// The underlying algorithm is an adaption of std::next_permutation
/// with constant amortized cost to compute the next tuple index.
class StateTupleIndexGenerator
{
private:
    const TupleIndexMapper* tuple_index_mapper;

    // Preallocated memory for reuse
    AtomIndexList atom_indices;

    friend class const_iterator;

public:
    explicit StateTupleIndexGenerator(const TupleIndexMapper* tuple_index_mapper);

    class const_iterator
    {
    private:
        /* External data */
        const TupleIndexMapper* m_tuple_index_mapper;
        const AtomIndexList* m_atoms;

        /* Internal data */
        std::array<size_t, MAX_ARITY> m_indices;
        bool m_end;
        int m_cur;

        std::optional<size_t> find_rightmost_incrementable_index();

        void advance();

        /**
         * Getters for less verbose access.
         */

        const TupleIndexMapper& get_tuple_index_mapper() const;
        const AtomIndexList& get_atoms() const;

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = TupleIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(StateTupleIndexGenerator* data, bool begin);
        value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
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
    const TupleIndexMapper* tuple_index_mapper;

    // Preallocated memory for reuse
    std::array<std::vector<size_t>, 2> a_index_jumper;
    std::array<AtomIndexList, 2> a_atom_indices;

    friend class const_iterator;

public:
    explicit StatePairTupleIndexGenerator(const TupleIndexMapper* tuple_index_mapper);

    class const_iterator
    {
    private:
        /* External data */
        const TupleIndexMapper* m_tuple_index_mapper;
        const std::array<AtomIndexList, 2>* m_a_atoms;
        std::array<std::vector<size_t>, 2>* m_a_jumpers;

        /* Internal data */
        std::array<size_t, MAX_ARITY> m_indices;
        std::array<bool, MAX_ARITY> m_a;
        int m_cur_outter;
        int m_cur_inner;
        bool m_end_outter;
        bool m_end_inner;

        // O(N)
        void initialize_jumpers();
        // O(K)
        std::optional<size_t> find_rightmost_incrementable_index();
        // O(1)
        std::optional<size_t> find_next_index(size_t i);
        // O(K*2^K)
        bool advance_outter();
        // O(1) amortized for O(N^K) iterations + O(K*2^K) for advance_outter.
        void advance_inner();

        bool try_create_first_inner_tuple();
        bool try_create_next_inner_tuple(size_t i);

        /**
         * Getters for less verbose access.
         */

        const TupleIndexMapper& get_tuple_index_mapper() const;
        const std::array<AtomIndexList, 2>& get_atoms() const;
        std::array<std::vector<size_t>, 2>& get_jumpers() const;

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = TupleIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(StatePairTupleIndexGenerator* sptig, bool begin);
        value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
    };

    const_iterator begin(const State state, const State succ_state);
    const_iterator begin(const AtomIndexList& atom_indices, const AtomIndexList& add_atom_indices);  // for testing only
    const_iterator end() const;
};

}

#endif