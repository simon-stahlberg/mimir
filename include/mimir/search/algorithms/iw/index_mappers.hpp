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

#ifndef MIMIR_SEARCH_ALGORITHMS_IW_INDEX_MAPPERS_HPP_
#define MIMIR_SEARCH_ALGORITHMS_IW_INDEX_MAPPERS_HPP_

#include "mimir/search/algorithms/iw/types.hpp"

#include <array>
#include <string>

namespace mimir
{

/// @brief TupleIndexMapper defines a perfect hash function mapping tuples of atom indices to a tuple index.
class TupleIndexMapper
{
private:
    int m_arity;
    int m_num_atoms;

    std::array<int, MAX_ARITY> m_factors;

    int m_empty_tuple_index;

public:
    TupleIndexMapper(int arity, int num_atoms);

    TupleIndex to_tuple_index(const AtomIndices& atom_indices) const;

    void to_atom_indices(TupleIndex tuple_index, AtomIndices& out_atom_indices) const;

    std::string tuple_index_to_string(TupleIndex tuple_index) const;

    /**
     * Getters
     */
    int get_num_atoms() const;
    int get_arity() const;
    const std::array<int, MAX_ARITY>& get_factors() const;
    int get_max_tuple_index() const;
    TupleIndex get_empty_tuple_index() const;
};

/// @brief FluentAndDerivedMapper encapsulates logic to combine
/// reached fluent and derived atom indices into a common indexing scheme 0,1,...
///
/// This is needed to consider both fluent and derived atoms in the novelty test.
/// Fluent atoms have their own indexing scheme 0,1,...
/// Derived atoms have their own indexing scheme 0,1,...
class FluentAndDerivedMapper
{
private:
    std::vector<int> m_fluent_remap;
    std::vector<int> m_derived_remap;
    int m_num_atoms;

    static const int UNDEFINED;

    void remap_atoms(const State state);

public:
    FluentAndDerivedMapper();

    void remap_and_combine_and_sort(const State state, AtomIndices& out_atoms);
    void remap_and_combine_and_sort(const State state, const State succ_state, AtomIndices& out_atoms, AtomIndices& out_add_atoms);
};

}

#endif
