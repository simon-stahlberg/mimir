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

#include "mimir/common/bounds.hpp"
#include "mimir/common/types.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/assignment_set_utils.hpp"
#include "mimir/formalism/declarations.hpp"

#include <cassert>
#include <limits>
#include <tuple>
#include <vector>

namespace mimir::formalism
{

/// @brief `AssignmentSet` is a helper class representing a set of functions
/// f : Predicates x Params(A) x Object x Params(A) x Object -> {true, false} where
///   1. f(p,i,o,j,o') = true iff there exists an atom p(...,o_i,...,o'_j,...)
///   2. f(p,i,o,-1,-1) = true iff there exists an atom p(...,o_i,...)
/// with respective meanings
///   1. the assignment [i/o], [j/o'] is consistent
///   2. the assignment [i/o] is consistent
///
/// We say that an assignment set is static if all atoms it considers are static.
template<IsStaticOrFluentOrDerivedTag P>
class AssignmentSet
{
private:
    size_t m_num_objects;

    // The underlying function
    std::vector<std::vector<bool>> m_per_predicate_assignment_set;

public:
    AssignmentSet();

    /// @brief Construct from a given set of ground atoms.
    AssignmentSet(size_t num_objects, const PredicateList<P>& predicates);

    /// @brief Clears all ground atoms from the assignment set.
    void reset();

    /// @brief Insert ground atoms into the assignment set.
    void insert_ground_atoms(const GroundAtomList<P>& ground_atoms);

    /// @brief Insert a ground atom into the assignment set.
    void insert_ground_atom(GroundAtom<P> ground_atom);

    /**
     * Getters
     */

    size_t get_num_objects() const { return m_num_objects; }
    const std::vector<std::vector<bool>>& get_per_predicate_assignment_set() const { return m_per_predicate_assignment_set; }
};

/// @brief `NumericAssignmentSet` is a helper class representing a set of functions
/// f : FunctionSkeleton x Params(A) x Object x Params(A) x Object -> Bounds<ContinuousCost> where
///   1. f(p,i,o,j,o') = [l,r] iff min_{...} h(...,o_i,...,o'_j,...) = l and max_{...} h(...,o_i,...,o'_j,...) = r
///   2. f(p,i,o,-1,-1) = [l,r] iff min_{...} h(...,o_i,...) = l and max_{...} h(...,o_i,...) = r
///   3. for all others, we have f(p,i,o,j,o') = [inf,-inf] and f(p,i,o,-1,-1) = [inf,-inf]
/// with respective meanings
///   1. the assignment [i/o], [j/o'] results in partial function evaluation [l,r]
///   2. the assignment [i/o] results in partial function evaluation [l,r]
/// Using the `NumericAssignmentSet` we can efficiently evaluate numeric constraints partially.

template<IsStaticOrFluentTag F>
class NumericAssignmentSet
{
private:
    size_t m_num_objects;
    std::vector<std::vector<Bounds<ContinuousCost>>> m_per_function_skeleton_bounds_set;

    /* temporaries for reuse */

    // This lets us easily compute the bounds for partial substitutions by sorting the vector by the cost,
    // followed by computing lower and upper bounds using minimization and maximization.
    std::vector<std::pair<GroundFunction<F>, ContinuousCost>> m_ground_function_to_value;

public:
    NumericAssignmentSet();

    /// @brief Construct from a given set of ground atoms.
    NumericAssignmentSet(size_t num_objects, const FunctionSkeletonList<F>& function_skeletons);

    /// @brief Resets all function skeleton bounds to unrestricted, i.e., [inf,-inf].
    void reset();

    /// @brief Insert fluent ground function values into the assignment set.
    void insert_ground_function_values(const GroundFunctionList<F>& ground_functions, const FlatDoubleList& numeric_values);

    /**
     * Getters
     */

    size_t get_num_objects() const { return m_num_objects; }
    const std::vector<std::vector<Bounds<ContinuousCost>>>& get_per_function_skeleton_bounds_set() const { return m_per_function_skeleton_bounds_set; }
};

}

#endif
