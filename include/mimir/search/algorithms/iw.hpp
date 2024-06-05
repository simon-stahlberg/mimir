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

    std::vector<int> m_factors;

public:
    TupleIndexMapper(size_t num_atoms, size_t arity);

    TupleIndex to_tuple_index(const AtomIndices& atom_indices);

    void to_atom_indices(const TupleIndex tuple_index, AtomIndices& out_atom_indices);
};

/// @brief TupleIndexGenerator encapsulates logic to generate all combinations of tuple indices
/// of size that is at most the arity defined in the TupleIndexMapper.
class TupleIndexGenerator
{
private:
    const TupleIndexMapper& m_tuple_index_mapper;
    const AtomIndices& m_atom_indices;
    const AtomIndices& m_add_atom_indices;

public:
    TupleIndexGenerator(const TupleIndexMapper& tuple_index_mapper, const AtomIndices& atom_indices);
    TupleIndexGenerator(const TupleIndexMapper& tuple_index_mapper, const AtomIndices& atom_indices, const AtomIndices& add_atom_indices);

    // TODO: add begin and end iterators
};

/// @brief DynamicNoveltyTable encapsulates a table to test novelty of tuples of atoms of size at most arity.
class DynamicNoveltyTable
{
private:
    TupleIndexMapper m_tuple_index_mapper;

public:
    explicit DynamicNoveltyTable(size_t arity);

    TupleIndices compute_novel_tuple_indices(const AtomIndices& atom_indices) const;

    TupleIndices compute_novel_tuple_indices(const AtomIndices& atom_indices, const AtomIndices& add_atom_indices) const;
};

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
