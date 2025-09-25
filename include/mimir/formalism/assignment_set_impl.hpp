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

#ifndef MIMIR_FORMALISM_ASSIGNMENT_SET_IMPL_HPP_
#define MIMIR_FORMALISM_ASSIGNMENT_SET_IMPL_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/parameter.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/type.hpp"

#include <algorithm>

namespace mimir::formalism
{

inline PerfectAssignmentHash::PerfectAssignmentHash(const ParameterList& parameters, const ObjectList& objects) :
    m_num_assignments(0),
    m_remapping(),
    m_offsets()
{
    const auto num_parameters = parameters.size();
    const auto num_objects = objects.size();

    m_remapping.resize(num_parameters + 1);
    m_offsets.resize(num_parameters + 1);

    m_remapping[0].resize(1, 0);  // 0 is sentinel to map to 0
    m_offsets[0] = m_num_assignments++;

    for (Index i = 0; i < num_parameters; ++i)
    {
        const auto& parameter = parameters[i];
        m_remapping[i + 1].resize(num_objects + 1, 0);  // 0 is sentinel to map to 0
        m_offsets[i + 1] = m_num_assignments++;

        auto new_index = Index { 1 };
        for (const auto& object : objects)
        {
            if (is_subtypeeq(object->get_bases(), parameter->get_bases()))
            {
                m_remapping[i + 1][object->get_index() + 1] = new_index++;
                ++m_num_assignments;
            }
        }
    }
}

inline size_t PerfectAssignmentHash::get_empty_assignment_rank() noexcept { return 0; }

inline size_t PerfectAssignmentHash::get_assignment_rank(const VertexAssignment& assignment) const noexcept
{
    const auto o = m_remapping[assignment.index + 1][assignment.object + 1];

    const auto result = m_offsets[assignment.index + 1] + o;

    assert(result < m_num_assignments);
    assert(result == get_assignment_rank(EdgeAssignment(MAX_INDEX, MAX_INDEX, assignment.index, assignment.object)));

    return result;
}

inline size_t PerfectAssignmentHash::get_assignment_rank(const EdgeAssignment& assignment) const noexcept
{
    const auto o1 = m_remapping[assignment.first_index + 1][assignment.first_object + 1];
    const auto o2 = m_remapping[assignment.second_index + 1][assignment.second_object + 1];

    const auto j1 = m_offsets[assignment.first_index + 1] + o1;
    const auto j2 = m_offsets[assignment.second_index + 1] + o2;

    assert(assignment.is_ordered());
    const auto result = j1 * m_num_assignments + j2;

    assert(result < get_num_assignments());

    return result;
}

inline size_t PerfectAssignmentHash::get_num_assignments() const noexcept { return m_num_assignments * m_num_assignments; }

template<IsStaticOrFluentOrDerivedTag P>
PredicateAssignmentSet<P>::PredicateAssignmentSet(const ObjectList& objects, Predicate<P> predicate) :
    m_predicate(predicate),
    m_hash(PerfectAssignmentHash(predicate->get_parameters(), objects)),
    m_set(m_hash.get_num_assignments(), false)
{
}

template<IsStaticOrFluentOrDerivedTag P>
void PredicateAssignmentSet<P>::reset() noexcept
{
    m_set.reset();
}

template<IsStaticOrFluentOrDerivedTag P>
void PredicateAssignmentSet<P>::insert_ground_atom(GroundAtom<P> ground_atom)
{
    const auto& arity = ground_atom->get_arity();
    const auto& objects = ground_atom->get_objects();

    assert(ground_atom->get_predicate() == m_predicate);

    for (size_t first_index = 0; first_index < arity; ++first_index)
    {
        const auto& first_object = objects[first_index];

        // Complete vertex.
        m_set.set(m_hash.get_assignment_rank(VertexAssignment(first_index, first_object->get_index())));

        // Partial edge with first assignment.
        m_set.set(m_hash.get_assignment_rank(EdgeAssignment(first_index, first_object->get_index(), MAX_INDEX, MAX_INDEX)));
        // Partial edge with second assignment is implicit in VertexAssignment.
        assert(m_set.test(m_hash.get_assignment_rank(VertexAssignment(first_index, first_object->get_index())))
               == m_set.test(m_hash.get_assignment_rank(EdgeAssignment(MAX_INDEX, MAX_INDEX, first_index, first_object->get_index()))));

        for (size_t second_index = first_index + 1; second_index < arity; ++second_index)
        {
            const auto& second_object = objects[second_index];

            // Ordered complete edge.
            m_set.set(m_hash.get_assignment_rank(EdgeAssignment(first_index, first_object->get_index(), second_index, second_object->get_index())));
        }
    }
}

template<IsStaticOrFluentOrDerivedTag P>
bool PredicateAssignmentSet<P>::operator[](const VertexAssignment& assignment) const noexcept
{
    return m_set.test(m_hash.get_assignment_rank(assignment));
}

template<IsStaticOrFluentOrDerivedTag P>
bool PredicateAssignmentSet<P>::operator[](const EdgeAssignment& assignment) const noexcept
{
    return m_set.test(m_hash.get_assignment_rank(assignment));
}

template<IsStaticOrFluentOrDerivedTag P>
size_t PredicateAssignmentSet<P>::size() const noexcept
{
    return m_set.size();
}

template<IsStaticOrFluentOrDerivedTag P>
PredicateAssignmentSets<P>::PredicateAssignmentSets(const ObjectList& objects, const PredicateList<P>& predicates)
{
    /* Validate inputs. */
    for (Index i = 0; i < predicates.size(); ++i)
        assert(predicates[i]->get_index() == i);

    /* Initialize sets. */
    for (const auto& predicate : predicates)
        m_sets.emplace_back(PredicateAssignmentSet<P>(objects, predicate));
}

template<IsStaticOrFluentOrDerivedTag P>
void PredicateAssignmentSets<P>::reset() noexcept
{
    for (auto& set : m_sets)
        set.reset();
}

template<IsStaticOrFluentOrDerivedTag P>
void PredicateAssignmentSets<P>::insert_ground_atoms(const GroundAtomList<P>& ground_atoms)
{
    for (const auto& ground_atom : ground_atoms)
        m_sets[ground_atom->get_predicate()->get_index()].insert_ground_atom(ground_atom);
}

template<IsStaticOrFluentOrDerivedTag P>
void PredicateAssignmentSets<P>::insert_ground_atom(GroundAtom<P> ground_atom)
{
    m_sets[ground_atom->get_predicate()->get_index()].insert_ground_atom(ground_atom);
}

template<IsStaticOrFluentOrDerivedTag P>
const PredicateAssignmentSet<P>& PredicateAssignmentSets<P>::get_set(Predicate<P> predicate) const noexcept
{
    return m_sets[predicate->get_index()];
}

template<IsStaticOrFluentOrDerivedTag P>
size_t PredicateAssignmentSets<P>::size() const noexcept
{
    return std::accumulate(m_sets.begin(), m_sets.end(), size_t { 0 }, [](auto&& lhs, auto&& rhs) { return lhs + rhs.size(); });
}

template<IsStaticOrFluentTag F>
FunctionSkeletonAssignmentSet<F>::FunctionSkeletonAssignmentSet(const ObjectList& objects, FunctionSkeleton<F> function_skeleton) :
    m_function_skeleton(function_skeleton),
    m_hash(PerfectAssignmentHash(function_skeleton->get_parameters(), objects)),
    m_set(m_hash.get_num_assignments(), Bounds<ContinuousCost>::unbounded)
{
}

template<IsStaticOrFluentTag F>
void FunctionSkeletonAssignmentSet<F>::reset() noexcept
{
    std::fill(m_set.begin(), m_set.end(), Bounds<ContinuousCost>::unbounded);
}

template<IsStaticOrFluentTag F>
void FunctionSkeletonAssignmentSet<F>::insert_ground_function_value(GroundFunction<F> ground_function, ContinuousCost value)
{
    const auto& arity = ground_function->get_arity();
    const auto& arguments = ground_function->get_objects();

    assert(ground_function->get_function_skeleton() == m_function_skeleton);

    auto& empty_assignment_bound = m_set[PerfectAssignmentHash::get_empty_assignment_rank()];
    empty_assignment_bound = Bounds(
        (empty_assignment_bound.get_lower() == -std::numeric_limits<ContinuousCost>::infinity()) ? value : std::min(empty_assignment_bound.get_lower(), value),
        (empty_assignment_bound.get_upper() == std::numeric_limits<ContinuousCost>::infinity()) ? value : std::max(empty_assignment_bound.get_upper(), value));

    for (size_t first_index = 0; first_index < arity; ++first_index)
    {
        const auto& first_object = arguments[first_index];

        // Complete vertex.
        auto& single_assignment_bound = m_set[m_hash.get_assignment_rank(VertexAssignment(first_index, first_object->get_index()))];
        single_assignment_bound = Bounds(
            (single_assignment_bound.get_lower() == -std::numeric_limits<ContinuousCost>::infinity()) ? value :
                                                                                                        std::min(single_assignment_bound.get_lower(), value),
            (single_assignment_bound.get_upper() == std::numeric_limits<ContinuousCost>::infinity()) ? value :
                                                                                                       std::max(single_assignment_bound.get_upper(), value));

        // Partial edge with first assignment.
        m_set[m_hash.get_assignment_rank(EdgeAssignment(first_index, first_object->get_index(), MAX_INDEX, MAX_INDEX))] = single_assignment_bound;
        // Partial edge with second assignment is implicit in VertexAssignment.
        assert(m_set[m_hash.get_assignment_rank(VertexAssignment(first_index, first_object->get_index()))]
               == m_set[m_hash.get_assignment_rank(EdgeAssignment(MAX_INDEX, MAX_INDEX, first_index, first_object->get_index()))]);

        for (size_t second_index = first_index + 1; second_index < arity; ++second_index)
        {
            const auto& second_object = arguments[second_index];

            // Ordered complete edge.
            auto& double_assignment_bound =
                m_set[m_hash.get_assignment_rank(EdgeAssignment(first_index, first_object->get_index(), second_index, second_object->get_index()))];
            double_assignment_bound = Bounds((double_assignment_bound.get_lower() == -std::numeric_limits<ContinuousCost>::infinity()) ?
                                                 value :
                                                 std::min(double_assignment_bound.get_lower(), value),
                                             (double_assignment_bound.get_upper() == std::numeric_limits<ContinuousCost>::infinity()) ?
                                                 value :
                                                 std::max(double_assignment_bound.get_upper(), value));
        }
    }
}

template<IsStaticOrFluentTag F>
Bounds<ContinuousCost> FunctionSkeletonAssignmentSet<F>::operator[](const VertexAssignment& assignment) const noexcept
{
    return m_set[m_hash.get_assignment_rank(assignment)];
}

template<IsStaticOrFluentTag F>
Bounds<ContinuousCost> FunctionSkeletonAssignmentSet<F>::operator[](const EdgeAssignment& assignment) const noexcept
{
    return m_set[m_hash.get_assignment_rank(assignment)];
}

template<IsStaticOrFluentTag F>
size_t FunctionSkeletonAssignmentSet<F>::size() const noexcept
{
    return m_set.size();
}

template<IsStaticOrFluentTag F>
FunctionSkeletonAssignmentSets<F>::FunctionSkeletonAssignmentSets(const ObjectList& objects, const FunctionSkeletonList<F>& function_skeletons)
{
    for (const auto& function_skeleton : function_skeletons)
        m_sets.emplace_back(FunctionSkeletonAssignmentSet<F>(objects, function_skeleton));
}

template<IsStaticOrFluentTag F>
void FunctionSkeletonAssignmentSets<F>::reset() noexcept
{
    for (auto& set : m_sets)
        set.reset();
}

template<IsStaticOrFluentTag F>
void FunctionSkeletonAssignmentSets<F>::insert_ground_function_values(const GroundFunctionList<F>& ground_functions, const FlatDoubleList& numeric_values)
{
    /* Validate inputs. */
    assert(ground_functions.size() == numeric_values.size());
    for (Index i = 0; i < ground_functions.size(); ++i)
        assert(ground_functions[i]->get_index() == i);

    /* Initialize bookkeeping. */
    m_ground_function_to_value.clear();
    for (size_t i = 0; i < ground_functions.size(); ++i)
        m_ground_function_to_value.emplace_back(ground_functions[i], numeric_values[i]);
    std::sort(m_ground_function_to_value.begin(), m_ground_function_to_value.end(), [](auto&& lhs, auto&& rhs) { return lhs.second < rhs.second; });

    /* Initialize sets. */
    for (const auto& [function, value] : m_ground_function_to_value)
        m_sets[function->get_function_skeleton()->get_index()].insert_ground_function_value(function, value);
}

template<IsStaticOrFluentTag F>
const FunctionSkeletonAssignmentSet<F>& FunctionSkeletonAssignmentSets<F>::get_set(FunctionSkeleton<F> function_skeleton) const noexcept
{
    return m_sets[function_skeleton->get_index()];
}

template<IsStaticOrFluentTag F>
size_t FunctionSkeletonAssignmentSets<F>::size() const noexcept
{
    return std::accumulate(m_sets.begin(), m_sets.end(), size_t { 0 }, [](auto&& lhs, auto&& rhs) { return lhs + rhs.size(); });
}

}

#endif