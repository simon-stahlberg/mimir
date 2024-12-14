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

#ifndef MIMIR_FORMALISM_ASSIGNMENT_SET_HPP_
#define MIMIR_FORMALISM_ASSIGNMENT_SET_HPP_

#include "mimir/formalism/assignment_set_utils.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/literal.hpp"

#include <cassert>
#include <limits>
#include <tuple>
#include <vector>

namespace mimir
{

/// @brief AssignmentSet is a helper class representing a set of functions
/// f : Predicates x Params(A) x Object x Params(A) x Object -> {true, false} where
///   1. f(p,i,o,j,o') = true iff there exists an atom p(...,o_i,...,o'_j)
///   2. f(p,i,o,-1,-1) = true iff there exists an atom p(...,o_i,...)
/// with respective meanings
///   1. the assignment [i/o], [j/o'] is consistent
///   2. the assignment [i/o] is consistent
///
/// We say that an assignment set is static if all atoms it considers are static.
template<PredicateTag P>
class AssignmentSet
{
private:
    size_t m_num_objects;

    // The underlying function
    std::vector<std::vector<bool>> per_predicate_assignment_set;

public:
    /// @brief Construct from a given set of ground atoms.
    AssignmentSet(size_t num_objects, const PredicateList<P>& predicates);

    /// @brief Clears all ground atoms from the assignment set.
    void clear();

    /// @brief Insert ground atoms into the assignment set.
    void insert_ground_atoms(const GroundAtomList<P>& ground_atoms);

    /// @brief Insert a ground atom into the assignment set.
    void insert_ground_atom(GroundAtom<P> ground_atom);

    /**
     * Getters
     */

    size_t get_num_objects() const { return m_num_objects; }
    const std::vector<std::vector<bool>>& get_per_predicate_assignment_set() const { return per_predicate_assignment_set; }
};

}

#endif
