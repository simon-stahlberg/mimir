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

#include "mimir/common/closed_interval.hpp"
#include "mimir/common/declarations.hpp"
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

    PerfectAssignmentHash(const ParameterList& parameters, const ObjectList& objects);

    size_t get_assignment_rank(const VertexAssignment& assignment) const noexcept;

    size_t get_assignment_rank(const EdgeAssignment& assignment) const noexcept;

    size_t size() const noexcept;
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

    void reset() noexcept;

    void insert_ground_atom(GroundAtom<P> ground_atom);

    bool operator[](const VertexAssignment& assignment) const noexcept;
    bool operator[](const EdgeAssignment& assignment) const noexcept;

    size_t size() const noexcept;
};

template<IsStaticOrFluentOrDerivedTag P>
class PredicateAssignmentSets
{
private:
    std::vector<PredicateAssignmentSet<P>> m_sets;

    GroundAtomList<P> m_atoms_scratch;

public:
    PredicateAssignmentSets() = default;

    PredicateAssignmentSets(const ObjectList& objects, const PredicateList<P>& predicates);

    void reset() noexcept;

    void insert_ground_atoms(const GroundAtomList<P>& ground_atoms);

    void insert_ground_atom(GroundAtom<P> ground_atom);

    const PredicateAssignmentSet<P>& get_set(Predicate<P> predicate) const noexcept;

    size_t size() const noexcept;

    GroundAtomList<P>& get_atoms_scratch();
};

template<IsStaticOrFluentTag F>
class FunctionSkeletonAssignmentSet
{
private:
    FunctionSkeleton<F> m_function_skeleton;

    PerfectAssignmentHash m_hash;
    std::vector<ClosedInterval<ContinuousCost>> m_set;

public:
    FunctionSkeletonAssignmentSet() = default;

    FunctionSkeletonAssignmentSet(const ObjectList& objects, FunctionSkeleton<F> function_skeleton);

    void reset() noexcept;

    void insert_ground_function_value(GroundFunction<F> ground_function, ContinuousCost value);

    ClosedInterval<ContinuousCost> operator[](const EmptyAssignment& assignment) const noexcept;
    ClosedInterval<ContinuousCost> operator[](const VertexAssignment& assignment) const noexcept;
    ClosedInterval<ContinuousCost> operator[](const EdgeAssignment& assignment) const noexcept;

    size_t size() const noexcept;
};

template<IsStaticOrFluentTag F>
class FunctionSkeletonAssignmentSets
{
private:
    std::vector<FunctionSkeletonAssignmentSet<F>> m_sets;

    GroundFunctionList<F> m_functions_scratch;

public:
    FunctionSkeletonAssignmentSets() = default;

    FunctionSkeletonAssignmentSets(const ObjectList& objects, const FunctionSkeletonList<F>& function_skeletons);

    void reset() noexcept;

    void insert_ground_function_values(const GroundFunctionList<F>& ground_functions, const FlatDoubleList& numeric_values);

    const FunctionSkeletonAssignmentSet<F>& get_set(FunctionSkeleton<F> function_skeleton) const noexcept;

    size_t size() const noexcept;

    GroundFunctionList<F>& get_functions_scratch();
};

struct StaticAssignmentSets
{
    PredicateAssignmentSets<StaticTag> static_predicate_assignment_sets;
    FunctionSkeletonAssignmentSets<StaticTag> static_function_skeleton_assignment_sets;

    StaticAssignmentSets();
    StaticAssignmentSets(const ProblemImpl& problem);
};

struct DynamicAssignmentSets
{
    PredicateAssignmentSets<FluentTag> fluent_predicate_assignment_sets;
    PredicateAssignmentSets<DerivedTag> derived_predicate_assignment_sets;

    FunctionSkeletonAssignmentSets<FluentTag> fluent_function_skeleton_assignment_sets;

    DynamicAssignmentSets();
    DynamicAssignmentSets(const ProblemImpl& problem);
};

}

#endif