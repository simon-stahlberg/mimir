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

#include "mimir/common/printers.hpp"
#include "mimir/formalism/function_skeleton.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_function.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/term.hpp"
#include "mimir/formalism/variable.hpp"

namespace mimir::formalism
{

/**
 * AssignmentSet
 */

template<IsStaticOrFluentOrDerivedTag P>
AssignmentSet<P>::AssignmentSet() : m_num_objects(0), m_per_predicate_assignment_set()
{
}

template<IsStaticOrFluentOrDerivedTag P>
AssignmentSet<P>::AssignmentSet(size_t num_objects, const PredicateList<P>& predicates) : m_num_objects(num_objects), m_per_predicate_assignment_set()
{
    /* Allocate */
    auto max_predicate_index = Index(0);
    for (const auto& predicate : predicates)
    {
        max_predicate_index = std::max(max_predicate_index, predicate->get_index());
    }
    m_per_predicate_assignment_set.resize(max_predicate_index + 1);

    for (const auto& predicate : predicates)
    {
        auto& assignment_set = m_per_predicate_assignment_set.at(predicate->get_index());
        assignment_set.resize(num_assignments(predicate->get_arity(), m_num_objects));
    }

    /* Initialize */
    reset();
}

template<IsStaticOrFluentOrDerivedTag P>
void AssignmentSet<P>::reset()
{
    for (auto& assignment_set : m_per_predicate_assignment_set)
    {
        std::fill(assignment_set.begin(), assignment_set.end(), false);
    }
}

template<IsStaticOrFluentOrDerivedTag P>
void AssignmentSet<P>::insert_ground_atoms(const GroundAtomList<P>& ground_atoms)
{
    for (const auto& ground_atom : ground_atoms)
    {
        const auto& arity = ground_atom->get_arity();
        const auto& predicate = ground_atom->get_predicate();
        const auto& arguments = ground_atom->get_objects();
        auto& assignment_set = m_per_predicate_assignment_set.at(predicate->get_index());

        for (size_t first_index = 0; first_index < arity; ++first_index)
        {
            const auto& first_object = arguments[first_index];
            assignment_set[get_assignment_rank(VertexAssignment(first_index, first_object->get_index()), arity, m_num_objects)] = true;

            for (size_t second_index = first_index + 1; second_index < arity; ++second_index)
            {
                const auto& second_object = arguments[second_index];
                assignment_set[get_assignment_rank(EdgeAssignment(first_index, first_object->get_index(), second_index, second_object->get_index()),
                                                   arity,
                                                   m_num_objects)] = true;
            }
        }
    }
}

template<IsStaticOrFluentOrDerivedTag P>
void AssignmentSet<P>::insert_ground_atom(GroundAtom<P> ground_atom)
{
    const auto& arity = ground_atom->get_arity();
    const auto& predicate = ground_atom->get_predicate();
    const auto& arguments = ground_atom->get_objects();
    auto& assignment_set = m_per_predicate_assignment_set.at(predicate->get_index());

    for (size_t first_index = 0; first_index < arity; ++first_index)
    {
        const auto& first_object = arguments[first_index];
        assignment_set[get_assignment_rank(VertexAssignment(first_index, first_object->get_index()), arity, m_num_objects)] = true;

        for (size_t second_index = first_index + 1; second_index < arity; ++second_index)
        {
            const auto& second_object = arguments[second_index];
            assignment_set[get_assignment_rank(EdgeAssignment(first_index, first_object->get_index(), second_index, second_object->get_index()),
                                               arity,
                                               m_num_objects)] = true;
        }
    }
}

template class AssignmentSet<StaticTag>;
template class AssignmentSet<FluentTag>;
template class AssignmentSet<DerivedTag>;

/**
 * NumericAssignmentSet
 */

template<IsStaticOrFluentTag F>
NumericAssignmentSet<F>::NumericAssignmentSet() : m_num_objects(0), m_per_function_skeleton_bounds_set()
{
}

template<IsStaticOrFluentTag F>
NumericAssignmentSet<F>::NumericAssignmentSet(size_t num_objects, const FunctionSkeletonList<F>& function_skeletons) :
    m_num_objects(num_objects),
    m_per_function_skeleton_bounds_set()
{
    /* Allocate */
    auto max_function_skeleton_index = Index(0);
    for (const auto& function_skeleton : function_skeletons)
    {
        max_function_skeleton_index = std::max(max_function_skeleton_index, function_skeleton->get_index());
    }
    m_per_function_skeleton_bounds_set.resize(max_function_skeleton_index + 1);

    for (const auto& function_skeleton : function_skeletons)
    {
        auto& assignment_set = m_per_function_skeleton_bounds_set.at(function_skeleton->get_index());
        assignment_set.resize(num_assignments(function_skeleton->get_arity(), m_num_objects));
    }

    /* Initialize */
    reset();
}

template<IsStaticOrFluentTag F>
void NumericAssignmentSet<F>::reset()
{
    for (auto& assignment_set : m_per_function_skeleton_bounds_set)
    {
        std::fill(assignment_set.begin(), assignment_set.end(), Bounds<ContinuousCost>::unbounded);
    }
}

template<IsStaticOrFluentTag F>
void NumericAssignmentSet<F>::insert_ground_function_values(const GroundFunctionList<F>& ground_fluent_functions, const FlatDoubleList& fluent_numeric_values)
{
    /* Validate inputs. */
    assert(ground_fluent_functions.size() == fluent_numeric_values.size());
    for (size_t i = 0; i < ground_fluent_functions.size(); ++i)
    {
        assert(ground_fluent_functions[i]);
        assert(ground_fluent_functions[i]->get_index() == i);
    }

    /* Initialize bookkeeping. */
    m_ground_function_to_value.clear();
    for (size_t i = 0; i < ground_fluent_functions.size(); ++i)
    {
        m_ground_function_to_value.emplace_back(ground_fluent_functions[i], fluent_numeric_values[i]);
    }
    std::sort(m_ground_function_to_value.begin(), m_ground_function_to_value.end(), [](auto&& lhs, auto&& rhs) { return lhs.second < rhs.second; });

    /* Compute lower and upper bounds. */
    for (const auto& [function, value] : m_ground_function_to_value)
    {
        const auto& arity = function->get_arity();
        const auto& function_skeleton = function->get_function_skeleton();
        const auto& arguments = function->get_objects();
        auto& assignment_set = m_per_function_skeleton_bounds_set.at(function_skeleton->get_index());

        auto& empty_assignment_bound = assignment_set[get_empty_assignment_rank()];
        empty_assignment_bound = Bounds(
            (empty_assignment_bound.get_lower() == -std::numeric_limits<ContinuousCost>::infinity()) ? value : std::min(empty_assignment_bound.get_lower(), value), 
            (empty_assignment_bound.get_upper() == std::numeric_limits<ContinuousCost>::infinity()) ? value : std::max(empty_assignment_bound.get_upper(), value));

        for (size_t first_index = 0; first_index < arity; ++first_index)
        {
            const auto& first_object = arguments[first_index];
            auto& single_assignment_bound = assignment_set[get_assignment_rank(VertexAssignment(first_index, first_object->get_index()), arity, m_num_objects)];
            single_assignment_bound = Bounds(
                (single_assignment_bound.get_lower() == -std::numeric_limits<ContinuousCost>::infinity()) ? value : std::min(single_assignment_bound.get_lower(), value),
                (single_assignment_bound.get_upper() == std::numeric_limits<ContinuousCost>::infinity()) ? value : std::max(single_assignment_bound.get_upper(), value));

            for (size_t second_index = first_index + 1; second_index < arity; ++second_index)
            {
                const auto& second_object = arguments[second_index];
                auto& double_assignment_bound =
                    assignment_set[get_assignment_rank(EdgeAssignment(first_index, first_object->get_index(), second_index, second_object->get_index()),
                                                       arity,
                                                       m_num_objects)];
                double_assignment_bound = Bounds(
                    (single_assignment_bound.get_lower() == -std::numeric_limits<ContinuousCost>::infinity()) ? value : std::min(double_assignment_bound.get_lower(), value),
                    (single_assignment_bound.get_upper() == std::numeric_limits<ContinuousCost>::infinity()) ? value : std::max(double_assignment_bound.get_upper(), value));
            }
        }
    }
}

template class NumericAssignmentSet<StaticTag>;
template class NumericAssignmentSet<FluentTag>;

}
