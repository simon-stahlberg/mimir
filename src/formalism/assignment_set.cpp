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

#include "mimir/formalism/assignment_set.hpp"

#include "mimir/common/formatter.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/parameter.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/type.hpp"

#include <algorithm>

namespace mimir::formalism
{

PerfectAssignmentHash::PerfectAssignmentHash(const ParameterList& parameters, const ObjectList& objects) : m_num_assignments(0), m_remapping(), m_offsets()
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

size_t PerfectAssignmentHash::get_assignment_rank(const VertexAssignment& assignment) const noexcept
{
    assert(assignment.is_valid());

    const auto o = m_remapping[assignment.index + 1][assignment.object + 1];

    const auto result = m_offsets[assignment.index + 1] + o;

    assert(result < m_num_assignments);

    return result;
}

size_t PerfectAssignmentHash::get_assignment_rank(const EdgeAssignment& assignment) const noexcept
{
    assert(assignment.is_valid());

    const auto o1 = m_remapping[assignment.first_index + 1][assignment.first_object + 1];
    const auto o2 = m_remapping[assignment.second_index + 1][assignment.second_object + 1];

    const auto j1 = m_offsets[assignment.first_index + 1] + o1;
    const auto j2 = m_offsets[assignment.second_index + 1] + o2;

    const auto result = j1 * m_num_assignments + j2;

    assert(result < m_num_assignments * m_num_assignments);

    return result;
}

size_t PerfectAssignmentHash::size() const noexcept { return m_num_assignments * m_num_assignments; }

template<IsStaticOrFluentOrDerivedTag P>
PredicateAssignmentSet<P>::PredicateAssignmentSet(const ObjectList& objects, Predicate<P> predicate) :
    m_predicate(predicate),
    m_hash(PerfectAssignmentHash(predicate->get_parameters(), objects)),
    m_set(m_hash.size(), false)
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

template class PredicateAssignmentSet<StaticTag>;
template class PredicateAssignmentSet<FluentTag>;
template class PredicateAssignmentSet<DerivedTag>;

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

template<IsStaticOrFluentOrDerivedTag P>
GroundAtomList<P>& PredicateAssignmentSets<P>::get_atoms_scratch()
{
    return m_atoms_scratch;
}

template class PredicateAssignmentSets<StaticTag>;
template class PredicateAssignmentSets<FluentTag>;
template class PredicateAssignmentSets<DerivedTag>;

template<IsStaticOrFluentTag F>
FunctionSkeletonAssignmentSet<F>::FunctionSkeletonAssignmentSet(const ObjectList& objects, FunctionSkeleton<F> function_skeleton) :
    m_function_skeleton(function_skeleton),
    m_hash(PerfectAssignmentHash(function_skeleton->get_parameters(), objects)),
    m_set(m_hash.size(), ClosedInterval<ContinuousCost>())
{
}

template<IsStaticOrFluentTag F>
void FunctionSkeletonAssignmentSet<F>::reset() noexcept
{
    std::fill(m_set.begin(), m_set.end(), ClosedInterval<ContinuousCost>());
}

template<IsStaticOrFluentTag F>
void FunctionSkeletonAssignmentSet<F>::insert_ground_function_value(GroundFunction<F> ground_function, ContinuousCost value)
{
    const auto& arity = ground_function->get_arity();
    const auto& arguments = ground_function->get_objects();

    assert(ground_function->get_function_skeleton() == m_function_skeleton);

    auto& empty_assignment_bound = m_set[EmptyAssignment::rank];
    empty_assignment_bound = hull(empty_assignment_bound, ClosedInterval<ContinuousCost>(value, value));

    for (size_t first_index = 0; first_index < arity; ++first_index)
    {
        const auto& first_object = arguments[first_index];

        // Complete vertex.
        auto& single_assignment_bound = m_set[m_hash.get_assignment_rank(VertexAssignment(first_index, first_object->get_index()))];
        single_assignment_bound = hull(single_assignment_bound, ClosedInterval<ContinuousCost>(value, value));

        for (size_t second_index = first_index + 1; second_index < arity; ++second_index)
        {
            const auto& second_object = arguments[second_index];

            // Ordered complete edge.
            auto& double_assignment_bound =
                m_set[m_hash.get_assignment_rank(EdgeAssignment(first_index, first_object->get_index(), second_index, second_object->get_index()))];
            double_assignment_bound = hull(double_assignment_bound, ClosedInterval<ContinuousCost>(value, value));
        }
    }
}

template<IsStaticOrFluentTag F>
ClosedInterval<ContinuousCost> FunctionSkeletonAssignmentSet<F>::operator[](const EmptyAssignment& assignment) const noexcept
{
    return m_set[EmptyAssignment::rank];
}

template<IsStaticOrFluentTag F>
ClosedInterval<ContinuousCost> FunctionSkeletonAssignmentSet<F>::operator[](const VertexAssignment& assignment) const noexcept
{
    return m_set[m_hash.get_assignment_rank(assignment)];
}

template<IsStaticOrFluentTag F>
ClosedInterval<ContinuousCost> FunctionSkeletonAssignmentSet<F>::operator[](const EdgeAssignment& assignment) const noexcept
{
    return m_set[m_hash.get_assignment_rank(assignment)];
}

template<IsStaticOrFluentTag F>
size_t FunctionSkeletonAssignmentSet<F>::size() const noexcept
{
    return m_set.size();
}

template class FunctionSkeletonAssignmentSet<StaticTag>;
template class FunctionSkeletonAssignmentSet<FluentTag>;

template<IsStaticOrFluentTag F>
FunctionSkeletonAssignmentSets<F>::FunctionSkeletonAssignmentSets(const ObjectList& objects, const FunctionSkeletonList<F>& function_skeletons) :
    m_sets(),
    m_functions_scratch()
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

    /* Initialize sets. */
    for (size_t i = 0; i < ground_functions.size(); ++i)
        m_sets[ground_functions[i]->get_function_skeleton()->get_index()].insert_ground_function_value(ground_functions[i], numeric_values[i]);
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

template<IsStaticOrFluentTag F>
GroundFunctionList<F>& FunctionSkeletonAssignmentSets<F>::get_functions_scratch()
{
    return m_functions_scratch;
}

template class FunctionSkeletonAssignmentSets<StaticTag>;
template class FunctionSkeletonAssignmentSets<FluentTag>;

StaticAssignmentSets::StaticAssignmentSets() {}

StaticAssignmentSets::StaticAssignmentSets(const ProblemImpl& problem) :
    static_predicate_assignment_sets(problem.get_problem_and_domain_objects(), problem.get_domain()->get_predicates<StaticTag>()),
    static_function_skeleton_assignment_sets(problem.get_problem_and_domain_objects(), problem.get_domain()->get_function_skeletons<StaticTag>())
{
    static_predicate_assignment_sets.insert_ground_atoms(problem.get_static_initial_atoms());

    static_function_skeleton_assignment_sets.insert_ground_function_values(problem.get_static_initial_functions(),
                                                                           problem.get_initial_function_to_value<StaticTag>());
}

DynamicAssignmentSets::DynamicAssignmentSets() {}

DynamicAssignmentSets::DynamicAssignmentSets(const ProblemImpl& problem) :
    fluent_predicate_assignment_sets(problem.get_problem_and_domain_objects(), problem.get_domain()->get_predicates<FluentTag>()),
    derived_predicate_assignment_sets(problem.get_problem_and_domain_objects(), problem.get_problem_and_domain_derived_predicates()),
    fluent_function_skeleton_assignment_sets(problem.get_problem_and_domain_objects(), problem.get_domain()->get_function_skeletons<FluentTag>())
{
}

}
