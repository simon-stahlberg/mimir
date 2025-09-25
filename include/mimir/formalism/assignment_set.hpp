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
#include "mimir/formalism/assignment.hpp"
#include "mimir/formalism/declarations.hpp"

#include <boost/dynamic_bitset.hpp>
#include <cassert>
#include <limits>
#include <tuple>
#include <vector>

namespace mimir::formalism
{

struct PerfectAssignmentHash
{
    size_t m_num_assignments;                        ///< The number of type legal [i/o] including a sentinel for each i
    std::vector<std::vector<uint32_t>> m_remapping;  ///< The remapping of o in O to index for each type legal [i/o]
    std::vector<uint32_t> m_offsets;                 ///< The offsets of i

    inline PerfectAssignmentHash(const ParameterList& parameters, const ObjectList& objects);

    static inline size_t get_empty_assignment_rank() noexcept;

    inline size_t get_assignment_rank(const VertexAssignment& assignment) const noexcept;

    inline size_t get_assignment_rank(const EdgeAssignment& assignment) const noexcept;

    inline size_t get_num_assignments() const noexcept;
};

template<IsStaticOrFluentOrDerivedTag P>
class PredicateAssignmentSet
{
private:
    Predicate<P> m_predicate;

    PerfectAssignmentHash m_hash;
    boost::dynamic_bitset<> m_set;

public:
    PredicateAssignmentSet(const ObjectList& objects, Predicate<P> predicate);

    inline void reset() noexcept;

    void insert_ground_atom(GroundAtom<P> ground_atom);

    inline bool operator[](const VertexAssignment& assignment) const noexcept;
    inline bool operator[](const EdgeAssignment& assignment) const noexcept;

    inline size_t size() const noexcept;
};

template<IsStaticOrFluentOrDerivedTag P>
class PredicateAssignmentSets
{
private:
    std::vector<PredicateAssignmentSet<P>> m_sets;

public:
    PredicateAssignmentSets() = default;

    PredicateAssignmentSets(const ObjectList& objects, const PredicateList<P>& predicates);

    inline void reset() noexcept;

    void insert_ground_atoms(const GroundAtomList<P>& ground_atoms);

    void insert_ground_atom(GroundAtom<P> ground_atom);

    inline const PredicateAssignmentSet<P>& get_set(Predicate<P> predicate) const noexcept;

    inline size_t size() const noexcept;
};

template<IsStaticOrFluentTag F>
class FunctionSkeletonAssignmentSet
{
private:
    FunctionSkeleton<F> m_function_skeleton;

    PerfectAssignmentHash m_hash;
    std::vector<Bounds<ContinuousCost>> m_set;

public:
    FunctionSkeletonAssignmentSet() = default;

    FunctionSkeletonAssignmentSet(const ObjectList& objects, FunctionSkeleton<F> function_skeleton);

    inline void reset() noexcept;

    void insert_ground_function_value(GroundFunction<F> ground_function, ContinuousCost value);

    inline Bounds<ContinuousCost> operator[](const VertexAssignment& assignment) const noexcept;
    inline Bounds<ContinuousCost> operator[](const EdgeAssignment& assignment) const noexcept;

    inline size_t size() const noexcept;
};

template<IsStaticOrFluentTag F>
class FunctionSkeletonAssignmentSets
{
private:
    std::vector<FunctionSkeletonAssignmentSet<F>> m_sets;

    // This lets us easily compute the bounds for partial substitutions by sorting the vector by the cost,
    // followed by computing lower and upper bounds using minimization and maximization.
    std::vector<std::pair<GroundFunction<F>, ContinuousCost>> m_ground_function_to_value;

public:
    FunctionSkeletonAssignmentSets() = default;

    FunctionSkeletonAssignmentSets(const ObjectList& objects, const FunctionSkeletonList<F>& function_skeletons);

    inline void reset() noexcept;

    void insert_ground_function_values(const GroundFunctionList<F>& ground_functions, const FlatDoubleList& numeric_values);

    inline const FunctionSkeletonAssignmentSet<F>& get_set(FunctionSkeleton<F> function_skeleton) const noexcept;

    inline size_t size() const noexcept;
};

}

#include "mimir/formalism/assignment_set_impl.hpp"

#endif