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

#ifndef MIMIR_SEARCH_ALGORITHMS_IW_TUPLE_INDEX_MAPPER_HPP_
#define MIMIR_SEARCH_ALGORITHMS_IW_TUPLE_INDEX_MAPPER_HPP_

#include "mimir/formalism/repositories.hpp"
#include "mimir/search/algorithms/iw/types.hpp"
#include "mimir/search/declarations.hpp"

#include <array>
#include <string>

namespace mimir::search::iw
{

/// @brief TupleIndexMapper defines a perfect hash function mapping tuples of atom indices to a tuple index.
class TupleIndexMapper
{
private:
    size_t m_arity;
    size_t m_num_atoms;

    std::array<size_t, MAX_ARITY> m_factors;

    TupleIndex m_empty_tuple_index;

public:
    TupleIndexMapper(size_t arity);
    TupleIndexMapper(size_t arity, size_t num_atoms);

    void initialize(size_t arity, size_t num_atoms);

    TupleIndex to_tuple_index(const AtomIndexList& atom_indices) const;

    AtomIndexList to_atom_indices(TupleIndex tuple_index) const;
    void to_atom_indices(TupleIndex tuple_index, AtomIndexList& out_atom_indices) const;

    std::string tuple_index_to_string(TupleIndex tuple_index) const;

    /**
     * Getters
     */
    size_t get_num_atoms() const;
    size_t get_arity() const;
    const std::array<size_t, MAX_ARITY>& get_factors() const;
    TupleIndex get_max_tuple_index() const;
    TupleIndex get_empty_tuple_index() const;
};

using TupleIndexMapperList = std::vector<TupleIndexMapper>;

}

#endif