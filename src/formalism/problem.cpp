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

#include "mimir/formalism/problem.hpp"

#include "formatter.hpp"
#include "mimir/common/collections.hpp"
#include "mimir/common/concepts.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/axiom.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_function.hpp"
#include "mimir/formalism/ground_function_expressions.hpp"
#include "mimir/formalism/ground_function_value.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/metric.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/requirements.hpp"
#include "mimir/formalism/utils.hpp"

#include <cassert>
#include <iostream>

using namespace std;

namespace mimir
{
ProblemImpl::ProblemImpl(Index index,
                         std::optional<fs::path> filepath,
                         Domain domain,
                         std::string name,
                         Requirements requirements,
                         ObjectList objects,
                         PredicateList<Derived> derived_predicates,
                         GroundLiteralList<Static> static_initial_literals,
                         GroundLiteralList<Fluent> fluent_initial_literals,
                         GroundFunctionValueList<Static> static_function_values,
                         GroundFunctionValueList<Fluent> fluent_function_values,
                         GroundFunctionValueList<Auxiliary> auxiliary_function_values,
                         GroundLiteralList<Static> static_goal_condition,
                         GroundLiteralList<Fluent> fluent_goal_condition,
                         GroundLiteralList<Derived> derived_goal_condition,
                         GroundNumericConstraintList numeric_goal_condition,
                         OptimizationMetric optimization_metric,
                         AxiomList axioms) :
    m_index(index),
    m_filepath(std::move(filepath)),
    m_domain(std::move(domain)),
    m_name(std::move(name)),
    m_requirements(std::move(requirements)),
    m_objects(std::move(objects)),
    m_derived_predicates(std::move(derived_predicates)),
    m_static_initial_literals(std::move(static_initial_literals)),
    m_fluent_initial_literals(std::move(fluent_initial_literals)),
    m_static_function_values(std::move(static_function_values)),
    m_fluent_function_values(std::move(fluent_function_values)),
    m_auxiliary_function_values(std::move(auxiliary_function_values)),
    m_static_goal_condition(std::move(static_goal_condition)),
    m_fluent_goal_condition(std::move(fluent_goal_condition)),
    m_derived_goal_condition(std::move(derived_goal_condition)),
    m_numeric_goal_condition(std::move(numeric_goal_condition)),
    m_optimization_metric(std::move(optimization_metric)),
    m_axioms(std::move(axioms)),
    m_problem_and_domain_derived_predicates(),
    m_positive_static_initial_atoms(to_ground_atoms(m_static_initial_literals)),
    m_positive_static_initial_atoms_bitset(),
    m_positive_static_initial_atoms_indices(),
    m_positive_static_initial_assignment_set(AssignmentSet<Static>(m_objects.size(), m_domain->get_predicates<Static>())),
    m_positive_fluent_initial_atoms(to_ground_atoms(m_fluent_initial_literals)),
    m_static_function_to_value(),
    m_fluent_function_to_value(),
    m_auxiliary_function_to_value(),
    m_static_goal_holds(false),
    m_positive_static_goal_atoms(filter_ground_atoms(m_static_goal_condition, true)),
    m_positive_fluent_goal_atoms(filter_ground_atoms(m_fluent_goal_condition, true)),
    m_positive_derived_goal_atoms(filter_ground_atoms(m_derived_goal_condition, true)),
    m_positive_static_goal_atoms_bitset(),
    m_positive_fluent_goal_atoms_bitset(),
    m_positive_derived_goal_atoms_bitset(),
    m_positive_static_goal_atoms_indices(),
    m_positive_fluent_goal_atoms_indices(),
    m_positive_derived_goal_atoms_indices(),
    m_negative_static_goal_atoms(filter_ground_atoms(m_static_goal_condition, false)),
    m_negative_fluent_goal_atoms(filter_ground_atoms(m_fluent_goal_condition, false)),
    m_negative_derived_goal_atoms(filter_ground_atoms(m_derived_goal_condition, false)),
    m_negative_static_goal_atoms_bitset(),
    m_negative_fluent_goal_atoms_bitset(),
    m_negative_derived_goal_atoms_bitset(),
    m_negative_static_goal_atoms_indices(),
    m_negative_fluent_goal_atoms_indices(),
    m_negative_derived_goal_atoms_indices(),
    m_problem_and_domain_axioms()
{
    assert(is_all_unique(m_objects));
    assert(is_all_unique(m_derived_predicates));
    assert(is_all_unique(m_static_initial_literals));
    assert(is_all_unique(m_fluent_initial_literals));
    assert(is_all_unique(m_static_function_values));
    assert(is_all_unique(m_static_goal_condition));
    assert(is_all_unique(m_fluent_goal_condition));
    assert(is_all_unique(m_derived_goal_condition));
    assert(is_all_unique(m_axioms));
    assert(std::is_sorted(m_objects.begin(), m_objects.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(
        std::is_sorted(m_derived_predicates.begin(), m_derived_predicates.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(m_static_initial_literals.begin(),
                          m_static_initial_literals.end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(m_fluent_initial_literals.begin(),
                          m_fluent_initial_literals.end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(m_static_function_values.begin(),
                          m_static_function_values.end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(m_fluent_function_values.begin(),
                          m_fluent_function_values.end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(m_auxiliary_function_values.begin(),
                          m_auxiliary_function_values.end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(m_static_goal_condition.begin(),
                          m_static_goal_condition.end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(m_fluent_goal_condition.begin(),
                          m_fluent_goal_condition.end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(m_derived_goal_condition.begin(),
                          m_derived_goal_condition.end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(m_numeric_goal_condition.begin(),
                          m_numeric_goal_condition.end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(m_axioms.begin(), m_axioms.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));

    /* Additional */

    m_problem_and_domain_derived_predicates = m_domain->get_predicates<Derived>();
    m_problem_and_domain_derived_predicates.insert(m_problem_and_domain_derived_predicates.end(),
                                                   get_derived_predicates().begin(),
                                                   get_derived_predicates().end());
    assert(is_all_unique(m_problem_and_domain_derived_predicates));

    /* Initial state */

    for (const auto& literal : get_static_initial_literals())
    {
        if (!literal->is_negated())
        {
            m_positive_static_initial_atoms_bitset.set(literal->get_atom()->get_index());
        }
    }

    std::transform(m_positive_static_initial_atoms_bitset.begin(),
                   m_positive_static_initial_atoms_bitset.end(),
                   std::back_inserter(m_positive_static_initial_atoms_indices),
                   [](unsigned long val) { return static_cast<Index>(val); });

    m_positive_static_initial_assignment_set.insert_ground_atoms(m_positive_static_initial_atoms);

    // As the ground functions in the goal might not necessarily be defined, we fill the gaps with undefined.
    // In principle, we could compress and define those values during search when applying an action that assigns it.
    for (const auto static_numeric_value : m_static_function_values)
    {
        const auto index = static_numeric_value->get_function()->get_index();
        const auto value = static_numeric_value->get_number();
        if (index >= m_static_function_to_value.size())
        {
            m_static_function_to_value.resize(index + 1, UNDEFINED_CONTINUOUS_COST);
        }
        m_static_function_to_value[index] = value;
    }
    for (const auto fluent_numeric_value : m_fluent_function_values)
    {
        const auto index = fluent_numeric_value->get_function()->get_index();
        const auto value = fluent_numeric_value->get_number();
        if (index >= m_fluent_function_to_value.size())
        {
            m_fluent_function_to_value.resize(index + 1, UNDEFINED_CONTINUOUS_COST);
        }
        m_fluent_function_to_value[index] = value;
    }
    for (const auto auxiliary_numeric_value : m_auxiliary_function_values)
    {
        const auto index = auxiliary_numeric_value->get_function()->get_index();
        const auto value = auxiliary_numeric_value->get_number();
        if (index >= m_auxiliary_function_to_value.size())
        {
            m_auxiliary_function_to_value.resize(index + 1, UNDEFINED_CONTINUOUS_COST);
        }
        m_auxiliary_function_to_value[index] = value;
    }
    for (size_t i = 0; i < m_fluent_function_to_value.size(); ++i)
    {
        if (m_fluent_function_to_value[i] == UNDEFINED_CONTINUOUS_COST)
        {
            std::cout << "Problem::Problem(...): m_fluent_function_to_value contains undefined values. We could optimize by compressing the indexing."
                      << std::endl;
        }
    }
    for (size_t i = 0; i < m_auxiliary_function_to_value.size(); ++i)
    {
        if (m_auxiliary_function_to_value[i] == UNDEFINED_CONTINUOUS_COST)
        {
            std::cout << "Problem::Problem(...): m_fluent_function_to_value contains undefined values. We could optimize by compressing the indexing."
                      << std::endl;
        }
    }

    /* Goal */

    // Determine whether the static goal holds
    m_static_goal_holds = true;
    for (const auto& literal : get_goal_condition<Static>())
    {
        if (literal->is_negated() == m_positive_static_initial_atoms_bitset.get(literal->get_atom()->get_index()))
        {
            m_static_goal_holds = false;
        }
    }

    for (const auto& literal : get_goal_condition<Static>())
    {
        literal->is_negated() ? m_negative_static_goal_atoms_bitset.set(literal->get_atom()->get_index()) :
                                m_positive_static_goal_atoms_bitset.set(literal->get_atom()->get_index());
    }
    std::transform(m_negative_static_goal_atoms_bitset.begin(),
                   m_negative_static_goal_atoms_bitset.end(),
                   std::back_inserter(m_negative_static_goal_atoms_indices),
                   [](unsigned long val) { return static_cast<Index>(val); });
    std::transform(m_positive_static_goal_atoms_bitset.begin(),
                   m_positive_static_goal_atoms_bitset.end(),
                   std::back_inserter(m_positive_static_goal_atoms_indices),
                   [](unsigned long val) { return static_cast<Index>(val); });

    for (const auto& literal : get_goal_condition<Fluent>())
    {
        literal->is_negated() ? m_negative_fluent_goal_atoms_bitset.set(literal->get_atom()->get_index()) :
                                m_positive_fluent_goal_atoms_bitset.set(literal->get_atom()->get_index());
    }
    std::transform(m_negative_fluent_goal_atoms_bitset.begin(),
                   m_negative_fluent_goal_atoms_bitset.end(),
                   std::back_inserter(m_negative_fluent_goal_atoms_indices),
                   [](unsigned long val) { return static_cast<Index>(val); });
    std::transform(m_positive_fluent_goal_atoms_bitset.begin(),
                   m_positive_fluent_goal_atoms_bitset.end(),
                   std::back_inserter(m_positive_fluent_goal_atoms_indices),
                   [](unsigned long val) { return static_cast<Index>(val); });

    for (const auto& literal : get_goal_condition<Derived>())
    {
        literal->is_negated() ? m_negative_derived_goal_atoms_bitset.set(literal->get_atom()->get_index()) :
                                m_positive_derived_goal_atoms_bitset.set(literal->get_atom()->get_index());
    }
    std::transform(m_negative_derived_goal_atoms_bitset.begin(),
                   m_negative_derived_goal_atoms_bitset.end(),
                   std::back_inserter(m_negative_derived_goal_atoms_indices),
                   [](unsigned long val) { return static_cast<Index>(val); });
    std::transform(m_positive_derived_goal_atoms_bitset.begin(),
                   m_positive_derived_goal_atoms_bitset.end(),
                   std::back_inserter(m_positive_derived_goal_atoms_indices),
                   [](unsigned long val) { return static_cast<Index>(val); });

    /* Axioms */

    m_problem_and_domain_axioms = m_domain->get_axioms();
    m_problem_and_domain_axioms.insert(m_problem_and_domain_axioms.end(), m_axioms.begin(), m_axioms.end());
    assert(is_all_unique(m_problem_and_domain_axioms));

    /**
     * Error checking
     */

    for (const auto& literal : get_fluent_initial_literals())
    {
        if (literal->is_negated())
        {
            throw std::runtime_error("Negative literals in the initial state is not supported.");
        }
    }

    for (const auto& literal : get_static_initial_literals())
    {
        if (literal->is_negated())
        {
            throw std::runtime_error("Negative literals in the initial state is not supported.");
        }
    }

    for (const auto& axiom : get_problem_and_domain_axioms())
    {
        if (axiom->get_literal()->is_negated())
        {
            throw std::runtime_error("Negative literals in axiom heads is not supported.");
        }
    }
}

Index ProblemImpl::get_index() const { return m_index; }

const std::optional<fs::path>& ProblemImpl::get_filepath() const { return m_filepath; }

const Domain& ProblemImpl::get_domain() const { return m_domain; }

const std::string& ProblemImpl::get_name() const { return m_name; }

const Requirements& ProblemImpl::get_requirements() const { return m_requirements; }

const ObjectList& ProblemImpl::get_objects() const { return m_objects; }

const PredicateList<Derived>& ProblemImpl::get_derived_predicates() const { return m_derived_predicates; }

const GroundLiteralList<Static>& ProblemImpl::get_static_initial_literals() const { return m_static_initial_literals; }

const GroundLiteralList<Fluent>& ProblemImpl::get_fluent_initial_literals() const { return m_fluent_initial_literals; }

template<FunctionTag F>
const GroundFunctionValueList<F>& ProblemImpl::get_function_values() const
{
    if constexpr (std::is_same_v<F, Static>)
    {
        return m_static_function_values;
    }
    else if constexpr (std::is_same_v<F, Fluent>)
    {
        return m_fluent_function_values;
    }
    else if constexpr (std::is_same_v<F, Auxiliary>)
    {
        return m_auxiliary_function_values;
    }
    else
    {
        static_assert(dependent_false<F>::value, "Missing implementation for FunctionTag.");
    }
}

template const GroundFunctionValueList<Static>& ProblemImpl::get_function_values() const;
template const GroundFunctionValueList<Fluent>& ProblemImpl::get_function_values() const;
template const GroundFunctionValueList<Auxiliary>& ProblemImpl::get_function_values() const;

template<PredicateTag P>
const GroundLiteralList<P>& ProblemImpl::get_goal_condition() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_static_goal_condition;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_fluent_goal_condition;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_derived_goal_condition;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template const GroundLiteralList<Static>& ProblemImpl::get_goal_condition<Static>() const;
template const GroundLiteralList<Fluent>& ProblemImpl::get_goal_condition<Fluent>() const;
template const GroundLiteralList<Derived>& ProblemImpl::get_goal_condition<Derived>() const;

const GroundNumericConstraintList& ProblemImpl::get_numeric_goal_condition() const { return m_numeric_goal_condition; }

const OptimizationMetric& ProblemImpl::get_optimization_metric() const { return m_optimization_metric; }

const AxiomList& ProblemImpl::get_axioms() const { return m_axioms; }

/**
 * Additional members
 */

/* Predicates */
const PredicateList<Derived>& ProblemImpl::get_problem_and_domain_derived_predicates() const { return m_problem_and_domain_derived_predicates; }

/* Initial state */

const GroundAtomList<Static>& ProblemImpl::get_static_initial_atoms() const { return m_positive_static_initial_atoms; }

const FlatBitset& ProblemImpl::get_static_initial_positive_atoms_bitset() const { return m_positive_static_initial_atoms_bitset; }

const FlatIndexList& ProblemImpl::get_static_initial_positive_atoms_indices() const
{
    assert(std::is_sorted(m_positive_static_initial_atoms_indices.begin(), m_positive_static_initial_atoms_indices.end()));
    return m_positive_static_initial_atoms_indices;
}

const AssignmentSet<Static>& ProblemImpl::get_static_assignment_set() const { return m_positive_static_initial_assignment_set; }

const GroundAtomList<Fluent>& ProblemImpl::get_fluent_initial_atoms() const { return m_positive_fluent_initial_atoms; }

template<FunctionTag F>
const FlatDoubleList& ProblemImpl::get_function_to_value() const
{
    if constexpr (std::is_same_v<F, Static>)
    {
        return m_static_function_to_value;
    }
    else if constexpr (std::is_same_v<F, Fluent>)
    {
        return m_fluent_function_to_value;
    }
    else if constexpr (std::is_same_v<F, Auxiliary>)
    {
        return m_auxiliary_function_to_value;
    }
    else
    {
        static_assert(dependent_false<F>::value, "Missing implementation for FunctionTag.");
    }
}

template const FlatDoubleList& ProblemImpl::get_function_to_value<Static>() const;
template const FlatDoubleList& ProblemImpl::get_function_to_value<Fluent>() const;
template const FlatDoubleList& ProblemImpl::get_function_to_value<Auxiliary>() const;

template<FunctionTag F>
ContinuousCost ProblemImpl::get_function_value(GroundFunction<F> function) const
{
    const auto& function_to_value = get_function_to_value<F>();
    if (function->get_index() > function_to_value.size())
    {
        return UNDEFINED_CONTINUOUS_COST;
    }
    return function_to_value[function->get_index()];
}

template ContinuousCost ProblemImpl::get_function_value(GroundFunction<Static> function) const;
template ContinuousCost ProblemImpl::get_function_value(GroundFunction<Fluent> function) const;
template ContinuousCost ProblemImpl::get_function_value(GroundFunction<Auxiliary> function) const;

/* Goal */
bool ProblemImpl::static_goal_holds() const { return m_static_goal_holds; }

bool ProblemImpl::static_literal_holds(const GroundLiteral<Static> literal) const
{
    return (literal->is_negated() != get_static_initial_positive_atoms_bitset().get(literal->get_atom()->get_index()));
}

template<PredicateTag P>
const GroundAtomList<P>& ProblemImpl::get_positive_goal_atoms() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_positive_static_goal_atoms;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_positive_fluent_goal_atoms;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_positive_derived_goal_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template const GroundAtomList<Static>& ProblemImpl::get_positive_goal_atoms<Static>() const;
template const GroundAtomList<Fluent>& ProblemImpl::get_positive_goal_atoms<Fluent>() const;
template const GroundAtomList<Derived>& ProblemImpl::get_positive_goal_atoms<Derived>() const;

template<PredicateTag P>
const FlatBitset& ProblemImpl::get_positive_goal_atoms_bitset() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_positive_static_goal_atoms_bitset;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_positive_fluent_goal_atoms_bitset;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_positive_derived_goal_atoms_bitset;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template const FlatBitset& ProblemImpl::get_positive_goal_atoms_bitset<Static>() const;
template const FlatBitset& ProblemImpl::get_positive_goal_atoms_bitset<Fluent>() const;
template const FlatBitset& ProblemImpl::get_positive_goal_atoms_bitset<Derived>() const;

template<PredicateTag P>
const FlatIndexList& ProblemImpl::get_positive_goal_atoms_indices() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_positive_static_goal_atoms_indices;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_positive_fluent_goal_atoms_indices;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_positive_derived_goal_atoms_indices;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template const FlatIndexList& ProblemImpl::get_positive_goal_atoms_indices<Static>() const;
template const FlatIndexList& ProblemImpl::get_positive_goal_atoms_indices<Fluent>() const;
template const FlatIndexList& ProblemImpl::get_positive_goal_atoms_indices<Derived>() const;

template<PredicateTag P>
const GroundAtomList<P>& ProblemImpl::get_negative_goal_atoms() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_negative_static_goal_atoms;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_negative_fluent_goal_atoms;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_negative_derived_goal_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template const GroundAtomList<Static>& ProblemImpl::get_negative_goal_atoms<Static>() const;
template const GroundAtomList<Fluent>& ProblemImpl::get_negative_goal_atoms<Fluent>() const;
template const GroundAtomList<Derived>& ProblemImpl::get_negative_goal_atoms<Derived>() const;

template<PredicateTag P>
const FlatBitset& ProblemImpl::get_negative_goal_atoms_bitset() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_negative_static_goal_atoms_bitset;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_negative_fluent_goal_atoms_bitset;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_negative_derived_goal_atoms_bitset;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template const FlatBitset& ProblemImpl::get_negative_goal_atoms_bitset<Static>() const;
template const FlatBitset& ProblemImpl::get_negative_goal_atoms_bitset<Fluent>() const;
template const FlatBitset& ProblemImpl::get_negative_goal_atoms_bitset<Derived>() const;

template<PredicateTag P>
const FlatIndexList& ProblemImpl::get_negative_goal_atoms_indices() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_negative_static_goal_atoms_indices;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_negative_fluent_goal_atoms_indices;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_negative_derived_goal_atoms_indices;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template const FlatIndexList& ProblemImpl::get_negative_goal_atoms_indices<Static>() const;
template const FlatIndexList& ProblemImpl::get_negative_goal_atoms_indices<Fluent>() const;
template const FlatIndexList& ProblemImpl::get_negative_goal_atoms_indices<Derived>() const;

/* Axioms */
const AxiomList& ProblemImpl::get_problem_and_domain_axioms() const { return m_problem_and_domain_axioms; }

/* Printing */
std::ostream& operator<<(std::ostream& out, const ProblemImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, Problem element)
{
    out << *element;
    return out;
}

}
