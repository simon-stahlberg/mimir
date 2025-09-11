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

#ifndef MIMIR_FORMALISM_ASSIGNMENT_SET2_HPP_
#define MIMIR_FORMALISM_ASSIGNMENT_SET2_HPP_

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

template<IsStaticOrFluentOrDerivedTag P>
class PredicateAssignmentSet
{
private:
    Problem m_problem;
    Predicate m_predicate;

    size_t m_num_vertex_assignments;                 ///< The number of vertices in the predicate
    std::vector<std::vector<uint32_t>> m_remapping;  ///< The remapping of o in O to o in O(p, i)
    std::vector<size_t> m_offsets;                   ///< The offsets of i
    std::vector<bool> m_set;

public:
    PredicateAssignmentSet(Problem problem, Predicate predicate) :
        m_problem(problem),
        m_predicate(predicate),
        m_num_vertex_assignments(0),
        m_remapping(),
        m_offsets(),
        m_set()
    {
        // TODO: initialize m_remapping, m_offsets, m_set
    }

    size_t get_assignment_rank(VertexAssignment assignment) const
    {
        const auto o = m_remapping[assignment.index + 1][assignment.object + 1];

        const auto result = m_offsets[assignment.index + 1] + o;

        return result;
    }

    size_t get_assignment_rank(EdgeAssignment assignment) const
    {
        const auto o1 = m_remapping[assignment.first_index + 1][assignment.first_object + 1];
        const auto o2 = m_remapping[assignment.second_index + 1][assignment.second_object + 1];

        const auto j1 = m_offsets[assignment.first_index + 1] + o1;
        const auto j2 = m_offsets[assignment.second_index + 1] + o2;

        const auto result = j1 * m_num_vertex_assignments + j2;

        return result;
    }

    size_t get_num_assignments() const { return m_num_vertex_assignments * m_num_vertex_assignments; }

    void reset();

    void insert_ground_atom(GroundAtom<P> ground_atom);
};

template<IsStaticOrFluentOrDerivedTag P>
class PredicateAssignmentSets
{
private:
    std::vector<PredicateAssignmentSet<P>> m_sets;

public:
    PredicateAssignmentSets() = default;

    PredicateAssignmentSets(Problem problem, const PredicateList<P>& predicates);

    void reset();

    void insert_ground_atoms(const GroundAtomList<P>& ground_atoms);

    void insert_ground_atom(GroundAtom<P> ground_atom);

    const PredicateAssignmentSet<P>& get_set(Predicate predicate) const { return m_sets[predicate->get_index()]; }
};

template<IsStaticOrFluentTag F>
class FunctionSkeletonAssignmentSet
{
private:
    Problem m_problem;
    FunctionSkeleton m_function_skeleton;

    size_t m_num_vertex_assignments;                 ///< The number of vertices in the predicate
    std::vector<std::vector<uint32_t>> m_remapping;  ///< The remapping of o in O to o in O(p, i)
    std::vector<size_t> m_offsets;                   ///< The offsets of i
    std::vector<Bounds<ContinuousCost>> m_set;

public:
    FunctionSkeletonAssignmentSet() = default;

    FunctionSkeletonAssignmentSet(Problem problem, FunctionSkeleton function_skeleton) : m_problem(problem), m_function_skeleton(function_skeleton) {}

    size_t get_assignment_rank(VertexAssignment assignment) const
    {
        const auto o = m_remapping[assignment.index + 1][assignment.object + 1];

        const auto result = m_offsets[assignment.index + 1] + o;

        return result;
    }

    size_t get_assignment_rank(EdgeAssignment assignment) const
    {
        const auto o1 = m_remapping[assignment.first_index + 1][assignment.first_object + 1];
        const auto o2 = m_remapping[assignment.second_index + 1][assignment.second_object + 1];

        const auto j1 = m_offsets[assignment.first_index + 1] + o1;
        const auto j2 = m_offsets[assignment.second_index + 1] + o2;

        const auto result = j1 * m_num_vertex_assignments + j2;

        return result;
    }

    size_t get_num_assignments() const { return m_num_vertex_assignments * m_num_vertex_assignments; }

    void reset();

    void insert_ground_function_value(GroundFunction<F> ground_function, ContinuousCost value);
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
    FunctionSkeletonAssignmentSets();

    FunctionSkeletonAssignmentSets(Problem problem, const FunctionSkeletonList<F>& function_skeletons);

    void reset();

    void insert_ground_function_values(const GroundFunctionList<F>& ground_functions, const FlatDoubleList& numeric_values);

    const FunctionSkeletonAssignmentSet<F>& get_set(FunctionSkeleton<F> function_skeleton) const { return m_sets[function_skeleton->get_index()]; }
};

}

#endif
