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
#include "mimir/common/itertools.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/axiom.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_function.hpp"
#include "mimir/formalism/ground_function_expressions.hpp"
#include "mimir/formalism/ground_function_value.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/ground_numeric_constraint.hpp"
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
                         PDDLRepositories repositories,
                         std::optional<fs::path> filepath,
                         Domain domain,
                         std::string name,
                         Requirements requirements,
                         ObjectList objects,
                         ObjectList problem_and_domain_objects,
                         PredicateList<Derived> derived_predicates,
                         PredicateList<Derived> problem_and_domain_derived_predicates,
                         GroundLiteralList<Static> static_initial_literals,
                         GroundLiteralList<Fluent> fluent_initial_literals,
                         GroundFunctionValueList<Static> static_function_values,
                         GroundFunctionValueList<Fluent> fluent_function_values,
                         std::optional<GroundFunctionValue<Auxiliary>> auxiliary_function_value,
                         GroundLiteralList<Static> static_goal_condition,
                         GroundLiteralList<Fluent> fluent_goal_condition,
                         GroundLiteralList<Derived> derived_goal_condition,
                         GroundNumericConstraintList numeric_goal_condition,
                         OptimizationMetric optimization_metric,
                         AxiomList axioms,
                         AxiomList problem_and_domain_axioms) :
    m_index(index),
    m_repositories(std::move(repositories)),
    m_filepath(std::move(filepath)),
    m_domain(std::move(domain)),
    m_name(std::move(name)),
    m_requirements(std::move(requirements)),
    m_objects(std::move(objects)),
    m_problem_and_domain_objects(std::move(problem_and_domain_objects)),
    m_derived_predicates(std::move(derived_predicates)),
    m_problem_and_domain_derived_predicates(std::move(problem_and_domain_derived_predicates)),
    m_static_initial_literals(std::move(static_initial_literals)),
    m_fluent_initial_literals(std::move(fluent_initial_literals)),
    m_static_function_values(std::move(static_function_values)),
    m_fluent_function_values(std::move(fluent_function_values)),
    m_auxiliary_function_value(auxiliary_function_value),
    m_static_goal_condition(std::move(static_goal_condition)),
    m_fluent_goal_condition(std::move(fluent_goal_condition)),
    m_derived_goal_condition(std::move(derived_goal_condition)),
    m_numeric_goal_condition(std::move(numeric_goal_condition)),
    m_optimization_metric(std::move(optimization_metric)),
    m_axioms(std::move(axioms)),
    m_problem_and_domain_axioms(std::move(problem_and_domain_axioms)),
    m_positive_static_initial_atoms(to_ground_atoms(m_static_initial_literals)),
    m_positive_static_initial_atoms_bitset(),
    m_positive_static_initial_atoms_indices(),
    m_positive_static_initial_assignment_set(AssignmentSet<Static>(m_objects.size(), m_domain->get_predicates<Static>())),
    m_static_initial_numeric_assignment_set(NumericAssignmentSet<Static>(m_objects.size(), m_domain->get_functions<Static>())),
    m_positive_fluent_initial_atoms(to_ground_atoms(m_fluent_initial_literals)),
    m_static_function_to_value(),
    m_fluent_function_to_value(),
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
    m_problem_and_domain_axiom_partitioning()
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
    auto static_functions = GroundFunctionList<Static> {};
    for (const auto static_numeric_value : m_static_function_values)
    {
        const auto function = static_numeric_value->get_function();
        const auto index = function->get_index();
        const auto value = static_numeric_value->get_number();
        if (index >= m_static_function_to_value.size())
        {
            static_functions.resize(index + 1, nullptr);
            m_static_function_to_value.resize(index + 1, UNDEFINED_CONTINUOUS_COST);
        }
        m_static_function_to_value[index] = value;
        static_functions[index] = function;
    }
    m_static_initial_numeric_assignment_set.insert_ground_function_values(static_functions, m_static_function_to_value);

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
    for (size_t i = 0; i < m_fluent_function_to_value.size(); ++i)
    {
        if (m_fluent_function_to_value[i] == UNDEFINED_CONTINUOUS_COST)
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

    m_problem_and_domain_axiom_partitioning = compute_axiom_partitioning(m_problem_and_domain_axioms, m_problem_and_domain_derived_predicates);

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

const PDDLRepositories& ProblemImpl::repositories() const { return m_repositories; }

const std::optional<fs::path>& ProblemImpl::get_filepath() const { return m_filepath; }

const Domain& ProblemImpl::get_domain() const { return m_domain; }

const std::string& ProblemImpl::get_name() const { return m_name; }

const Requirements& ProblemImpl::get_requirements() const { return m_requirements; }

const ObjectList& ProblemImpl::get_objects() const { return m_objects; }

const ObjectList& ProblemImpl::get_problem_and_domain_objects() const { return m_problem_and_domain_objects; }

const PredicateList<Derived>& ProblemImpl::get_derived_predicates() const { return m_derived_predicates; }

const PredicateList<Derived>& ProblemImpl::get_problem_and_domain_derived_predicates() const { return m_problem_and_domain_derived_predicates; }

const GroundLiteralList<Static>& ProblemImpl::get_static_initial_literals() const { return m_static_initial_literals; }

const GroundLiteralList<Fluent>& ProblemImpl::get_fluent_initial_literals() const { return m_fluent_initial_literals; }

template<StaticOrFluentTag F>
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
    else
    {
        static_assert(dependent_false<F>::value, "Missing implementation for StaticOrFluentOrAuxiliary.");
    }
}

template const GroundFunctionValueList<Static>& ProblemImpl::get_function_values() const;
template const GroundFunctionValueList<Fluent>& ProblemImpl::get_function_values() const;

const std::optional<GroundFunctionValue<Auxiliary>>& ProblemImpl::get_auxiliary_function_value() const { return m_auxiliary_function_value; }

template<StaticOrFluentOrDerived P>
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
        static_assert(dependent_false<P>::value, "Missing implementation for StaticOrFluentOrDerived.");
    }
}

template const GroundLiteralList<Static>& ProblemImpl::get_goal_condition<Static>() const;
template const GroundLiteralList<Fluent>& ProblemImpl::get_goal_condition<Fluent>() const;
template const GroundLiteralList<Derived>& ProblemImpl::get_goal_condition<Derived>() const;

const GroundNumericConstraintList& ProblemImpl::get_numeric_goal_condition() const { return m_numeric_goal_condition; }

const OptimizationMetric& ProblemImpl::get_optimization_metric() const { return m_optimization_metric; }

const AxiomList& ProblemImpl::get_axioms() const { return m_axioms; }

const AxiomList& ProblemImpl::get_problem_and_domain_axioms() const { return m_problem_and_domain_axioms; }

/**
 * Additional members
 */

/* Initial state */

const GroundAtomList<Static>& ProblemImpl::get_static_initial_atoms() const { return m_positive_static_initial_atoms; }

const FlatBitset& ProblemImpl::get_static_initial_positive_atoms_bitset() const { return m_positive_static_initial_atoms_bitset; }

const FlatIndexList& ProblemImpl::get_static_initial_positive_atoms_indices() const
{
    assert(std::is_sorted(m_positive_static_initial_atoms_indices.begin(), m_positive_static_initial_atoms_indices.end()));
    return m_positive_static_initial_atoms_indices;
}

const AssignmentSet<Static>& ProblemImpl::get_static_assignment_set() const { return m_positive_static_initial_assignment_set; }

const NumericAssignmentSet<Static>& ProblemImpl::get_static_initial_numeric_assignment_set() const { return m_static_initial_numeric_assignment_set; }

const GroundAtomList<Fluent>& ProblemImpl::get_fluent_initial_atoms() const { return m_positive_fluent_initial_atoms; }

template<StaticOrFluentTag F>
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
    else
    {
        static_assert(dependent_false<F>::value, "Missing implementation for StaticOrFluentOrAuxiliary.");
    }
}

template const FlatDoubleList& ProblemImpl::get_function_to_value<Static>() const;
template const FlatDoubleList& ProblemImpl::get_function_to_value<Fluent>() const;

template<StaticOrFluentTag F>
ContinuousCost ProblemImpl::get_function_value(GroundFunction<F> function) const
{
    const auto& function_to_value = get_function_to_value<F>();
    if (function->get_index() >= function_to_value.size())
    {
        return UNDEFINED_CONTINUOUS_COST;
    }
    return function_to_value[function->get_index()];
}

template ContinuousCost ProblemImpl::get_function_value(GroundFunction<Static> function) const;
template ContinuousCost ProblemImpl::get_function_value(GroundFunction<Fluent> function) const;

/* Goal */
bool ProblemImpl::static_goal_holds() const { return m_static_goal_holds; }

bool ProblemImpl::static_literal_holds(const GroundLiteral<Static> literal) const
{
    return (literal->is_negated() != get_static_initial_positive_atoms_bitset().get(literal->get_atom()->get_index()));
}

template<StaticOrFluentOrDerived P>
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
        static_assert(dependent_false<P>::value, "Missing implementation for StaticOrFluentOrDerived.");
    }
}

template const GroundAtomList<Static>& ProblemImpl::get_positive_goal_atoms<Static>() const;
template const GroundAtomList<Fluent>& ProblemImpl::get_positive_goal_atoms<Fluent>() const;
template const GroundAtomList<Derived>& ProblemImpl::get_positive_goal_atoms<Derived>() const;

template<StaticOrFluentOrDerived P>
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
        static_assert(dependent_false<P>::value, "Missing implementation for StaticOrFluentOrDerived.");
    }
}

template const FlatBitset& ProblemImpl::get_positive_goal_atoms_bitset<Static>() const;
template const FlatBitset& ProblemImpl::get_positive_goal_atoms_bitset<Fluent>() const;
template const FlatBitset& ProblemImpl::get_positive_goal_atoms_bitset<Derived>() const;

template<StaticOrFluentOrDerived P>
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
        static_assert(dependent_false<P>::value, "Missing implementation for StaticOrFluentOrDerived.");
    }
}

template const FlatIndexList& ProblemImpl::get_positive_goal_atoms_indices<Static>() const;
template const FlatIndexList& ProblemImpl::get_positive_goal_atoms_indices<Fluent>() const;
template const FlatIndexList& ProblemImpl::get_positive_goal_atoms_indices<Derived>() const;

template<StaticOrFluentOrDerived P>
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
        static_assert(dependent_false<P>::value, "Missing implementation for StaticOrFluentOrDerived.");
    }
}

template const GroundAtomList<Static>& ProblemImpl::get_negative_goal_atoms<Static>() const;
template const GroundAtomList<Fluent>& ProblemImpl::get_negative_goal_atoms<Fluent>() const;
template const GroundAtomList<Derived>& ProblemImpl::get_negative_goal_atoms<Derived>() const;

template<StaticOrFluentOrDerived P>
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
        static_assert(dependent_false<P>::value, "Missing implementation for StaticOrFluentOrDerived.");
    }
}

template const FlatBitset& ProblemImpl::get_negative_goal_atoms_bitset<Static>() const;
template const FlatBitset& ProblemImpl::get_negative_goal_atoms_bitset<Fluent>() const;
template const FlatBitset& ProblemImpl::get_negative_goal_atoms_bitset<Derived>() const;

template<StaticOrFluentOrDerived P>
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
        static_assert(dependent_false<P>::value, "Missing implementation for StaticOrFluentOrDerived.");
    }
}

template const FlatIndexList& ProblemImpl::get_negative_goal_atoms_indices<Static>() const;
template const FlatIndexList& ProblemImpl::get_negative_goal_atoms_indices<Fluent>() const;
template const FlatIndexList& ProblemImpl::get_negative_goal_atoms_indices<Derived>() const;

/* Axioms */

const std::vector<AxiomPartition>& ProblemImpl::get_problem_and_domain_axiom_partitioning() const { return m_problem_and_domain_axiom_partitioning; }

/* Grounding */

// Terms
static void ground_terms(const TermList& terms, const ObjectList& binding, ObjectList& out_terms)
{
    out_terms.clear();

    for (const auto& term : terms)
    {
        std::visit(
            [&](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, Object>)
                {
                    out_terms.emplace_back(arg);
                }
                else if constexpr (std::is_same_v<T, Variable>)
                {
                    assert(arg->get_parameter_index() < binding.size());
                    out_terms.emplace_back(binding[arg->get_parameter_index()]);
                }
            },
            term->get_variant());
    }
}
// Literal

template<StaticOrFluentOrDerived P>
GroundLiteral<P> ProblemImpl::ground(Literal<P> literal, const ObjectList& binding)
{
    /* 1. Access the type specific grounding tables. */
    auto& grounding_tables = boost::hana::at_key(m_grounding_tables, boost::hana::type<GroundLiteral<P>> {});

    /* 2. Access the context-independent literal grounding table */
    const auto is_negated = literal->is_negated();
    const auto predicate_index = literal->get_atom()->get_predicate()->get_index();
    auto& polarity_grounding_tables = grounding_tables[is_negated];
    if (predicate_index >= polarity_grounding_tables.size())
    {
        polarity_grounding_tables.resize(predicate_index + 1);
    }

    auto& grounding_table = polarity_grounding_tables.at(predicate_index);

    /* 3. Check if grounding is cached */

    // We have to fetch the literal-relevant part of the binding first.
    auto grounded_terms = ObjectList {};
    ground_terms(literal->get_atom()->get_terms(), binding, grounded_terms);

    const auto it = grounding_table.find(grounded_terms);
    if (it != grounding_table.end())
    {
        return it->second;
    }

    /* 4. Ground the literal */

    auto grounded_atom = m_repositories.get_or_create_ground_atom(literal->get_atom()->get_predicate(), grounded_terms);
    auto grounded_literal = m_repositories.get_or_create_ground_literal(literal->is_negated(), grounded_atom);

    /* 5. Insert to grounding_table table */

    grounding_table.emplace(std::move(grounded_terms), GroundLiteral<P>(grounded_literal));

    /* 6. Return the resulting ground literal */

    return grounded_literal;
}

template GroundLiteral<Static> ProblemImpl::ground(Literal<Static> literal, const ObjectList& binding);
template GroundLiteral<Fluent> ProblemImpl::ground(Literal<Fluent> literal, const ObjectList& binding);
template GroundLiteral<Derived> ProblemImpl::ground(Literal<Derived> literal, const ObjectList& binding);

template<StaticOrFluentOrDerived P>
void ProblemImpl::ground_and_fill_bitset(const std::vector<Literal<P>>& literals,
                                         FlatBitset& ref_positive_bitset,
                                         FlatBitset& ref_negative_bitset,
                                         const ObjectList& binding)
{
    for (const auto& literal : literals)
    {
        const auto grounded_literal = ground(literal, binding);

        if (grounded_literal->is_negated())
        {
            ref_negative_bitset.set(grounded_literal->get_atom()->get_index());
        }
        else
        {
            ref_positive_bitset.set(grounded_literal->get_atom()->get_index());
        }
    }
}

template void ProblemImpl::ground_and_fill_bitset(const std::vector<Literal<Static>>& literals,
                                                  FlatBitset& ref_positive_bitset,
                                                  FlatBitset& ref_negative_bitset,
                                                  const ObjectList& binding);
template void ProblemImpl::ground_and_fill_bitset(const std::vector<Literal<Fluent>>& literals,
                                                  FlatBitset& ref_positive_bitset,
                                                  FlatBitset& ref_negative_bitset,
                                                  const ObjectList& binding);
template void ProblemImpl::ground_and_fill_bitset(const std::vector<Literal<Derived>>& literals,
                                                  FlatBitset& ref_positive_bitset,
                                                  FlatBitset& ref_negative_bitset,
                                                  const ObjectList& binding);

template<StaticOrFluentOrDerived P>
void ProblemImpl::ground_and_fill_vector(const std::vector<Literal<P>>& literals,
                                         FlatIndexList& ref_positive_indices,
                                         FlatIndexList& ref_negative_indices,
                                         const ObjectList& binding)
{
    for (const auto& literal : literals)
    {
        const auto grounded_literal = ground(literal, binding);

        if (grounded_literal->is_negated())
        {
            ref_negative_indices.push_back(grounded_literal->get_atom()->get_index());
        }
        else
        {
            ref_positive_indices.push_back(grounded_literal->get_atom()->get_index());
        }
    }
    std::sort(ref_positive_indices.begin(), ref_positive_indices.end());
    std::sort(ref_negative_indices.begin(), ref_negative_indices.end());
}

template void ProblemImpl::ground_and_fill_vector(const std::vector<Literal<Static>>& literals,
                                                  FlatIndexList& ref_positive_indices,
                                                  FlatIndexList& ref_negative_indices,
                                                  const ObjectList& binding);
template void ProblemImpl::ground_and_fill_vector(const std::vector<Literal<Fluent>>& literals,
                                                  FlatIndexList& ref_positive_indices,
                                                  FlatIndexList& ref_negative_indices,
                                                  const ObjectList& binding);
template void ProblemImpl::ground_and_fill_vector(const std::vector<Literal<Derived>>& literals,
                                                  FlatIndexList& ref_positive_indices,
                                                  FlatIndexList& ref_negative_indices,
                                                  const ObjectList& binding);

// Function

template<StaticOrFluentOrAuxiliary F>
GroundFunction<F> ProblemImpl::ground(Function<F> function, const ObjectList& binding)
{
    /* 1. Access the type specific grounding tables. */
    auto& grounding_tables = boost::hana::at_key(m_grounding_tables, boost::hana::type<GroundFunction<F>> {});

    /* 2. Access the context-independent function grounding table */
    const auto function_skeleton_index = function->get_function_skeleton()->get_index();
    if (function_skeleton_index >= grounding_tables.size())
    {
        grounding_tables.resize(function_skeleton_index + 1);
    }

    auto& grounding_table = grounding_tables.at(function_skeleton_index);

    /* 3. Check if grounding is cached */

    // We have to fetch the literal-relevant part of the binding first.
    // Note: this is important and saves a lot of memory.
    auto grounded_terms = ObjectList {};
    ground_terms(function->get_terms(), binding, grounded_terms);

    const auto it = grounding_table.find(grounded_terms);
    if (it != grounding_table.end())
    {
        return it->second;
    }

    /* 4. Ground the function */

    const auto grounded_function = m_repositories.get_or_create_ground_function(function->get_function_skeleton(), grounded_terms);

    /* 5. Insert to grounding_table table */

    grounding_table.emplace(std::move(grounded_terms), GroundFunction<F>(grounded_function));

    /* 6. Return the resulting ground literal */

    return grounded_function;
}

template GroundFunction<Static> ProblemImpl::ground(Function<Static> function, const ObjectList& binding);
template GroundFunction<Fluent> ProblemImpl::ground(Function<Fluent> function, const ObjectList& binding);
template GroundFunction<Auxiliary> ProblemImpl::ground(Function<Auxiliary> function, const ObjectList& binding);

// FunctionExpression

GroundFunctionExpression ProblemImpl::ground(FunctionExpression fexpr, Problem problem, const ObjectList& binding)
{
    /* 1. Access the type specific grounding tables. */
    auto& grounding_tables = boost::hana::at_key(m_grounding_tables, boost::hana::type<GroundFunctionExpression> {});

    /* 2. Access the context-specific fexpr grounding table
     */

    const auto fexpr_index = fexpr->get_index();
    if (fexpr_index >= grounding_tables.size())
    {
        grounding_tables.resize(fexpr_index + 1);
    }

    auto& grounding_table = grounding_tables.at(fexpr_index);

    /* 3. Check if grounding is cached */

    const auto it = grounding_table.find(binding);
    if (it != grounding_table.end())
    {
        return it->second;
    }

    /* 4. Ground the function expression */
    const auto grounded_fexpr = std::visit(
        [&](auto&& arg) -> GroundFunctionExpression
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, FunctionExpressionNumber>)
            {
                return m_repositories.get_or_create_ground_function_expression(
                    m_repositories.get_or_create_ground_function_expression_number(arg->get_number()));
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionBinaryOperator>)
            {
                const auto op = arg->get_binary_operator();
                const auto ground_lhs = ground(arg->get_left_function_expression(), problem, binding);
                const auto ground_rhs = ground(arg->get_right_function_expression(), problem, binding);

                if (std::holds_alternative<GroundFunctionExpressionNumber>(ground_lhs->get_variant())
                    && std::holds_alternative<GroundFunctionExpressionNumber>(ground_rhs->get_variant()))
                {
                    return m_repositories.get_or_create_ground_function_expression(m_repositories.get_or_create_ground_function_expression_number(
                        evaluate_binary(op,
                                        std::get<GroundFunctionExpressionNumber>(ground_lhs->get_variant())->get_number(),
                                        std::get<GroundFunctionExpressionNumber>(ground_rhs->get_variant())->get_number())));
                }

                return m_repositories.get_or_create_ground_function_expression(
                    m_repositories.get_or_create_ground_function_expression_binary_operator(op, ground_lhs, ground_rhs));
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionMultiOperator>)
            {
                const auto op = arg->get_multi_operator();
                auto fexpr_numbers = GroundFunctionExpressionList {};
                auto fexpr_others = GroundFunctionExpressionList {};
                for (const auto& child_fexpr : arg->get_function_expressions())
                {
                    const auto ground_child_fexpr = ground(child_fexpr, problem, binding);
                    std::holds_alternative<GroundFunctionExpressionNumber>(ground_child_fexpr->get_variant()) ? fexpr_numbers.push_back(ground_child_fexpr) :
                                                                                                                fexpr_others.push_back(ground_child_fexpr);
                }

                if (!fexpr_numbers.empty())
                {
                    const auto value =
                        std::accumulate(std::next(fexpr_numbers.begin()),  // Start from the second expression
                                        fexpr_numbers.end(),
                                        std::get<GroundFunctionExpressionNumber>(fexpr_numbers.front()->get_variant())->get_number(),  // Initial bounds
                                        [op](const auto& value, const auto& child_expr) {
                                            return evaluate_multi(op, value, std::get<GroundFunctionExpressionNumber>(child_expr->get_variant())->get_number());
                                        });

                    fexpr_others.push_back(
                        m_repositories.get_or_create_ground_function_expression(m_repositories.get_or_create_ground_function_expression_number(value)));
                }

                return m_repositories.get_or_create_ground_function_expression(
                    m_repositories.get_or_create_ground_function_expression_multi_operator(op, fexpr_others));
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionMinus>)
            {
                const auto ground_fexpr = ground(arg->get_function_expression(), problem, binding);

                return std::holds_alternative<GroundFunctionExpressionNumber>(ground_fexpr->get_variant()) ?
                           m_repositories.get_or_create_ground_function_expression(m_repositories.get_or_create_ground_function_expression_number(
                               -std::get<GroundFunctionExpressionNumber>(ground_fexpr->get_variant())->get_number())) :
                           ground_fexpr;
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionFunction<Static>>)
            {
                return m_repositories.get_or_create_ground_function_expression(
                    m_repositories.get_or_create_ground_function_expression_number(problem->get_function_value<Static>(ground(arg->get_function(), binding))));
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionFunction<Fluent>>)
            {
                return m_repositories.get_or_create_ground_function_expression(
                    m_repositories.get_or_create_ground_function_expression_function<Fluent>(ground(arg->get_function(), binding)));
            }
            else
            {
                static_assert(dependent_false<T>::value,
                              "NumericConstraintGrounder::ground(fexpr, binding): Missing implementation for GroundFunctionExpression type.");
            }
        },
        fexpr->get_variant());

    /* 5. Insert to grounding_table table */

    grounding_table.emplace(binding, GroundFunctionExpression(grounded_fexpr));

    /* 6. Return the resulting ground literal */

    return grounded_fexpr;
}

// NumericConstraint
GroundNumericConstraint ProblemImpl::ground(NumericConstraint numeric_constraint, Problem problem, const ObjectList& binding)
{
    return m_repositories.get_or_create_ground_numeric_constraint(numeric_constraint->get_binary_comparator(),
                                                                  ground(numeric_constraint->get_left_function_expression(), problem, binding),
                                                                  ground(numeric_constraint->get_right_function_expression(), problem, binding));
}

// NumericEffect
template<FluentOrAuxiliary F>
GroundNumericEffect<F> ProblemImpl::ground(NumericEffect<F> numeric_effect, Problem problem, const ObjectList& binding)
{
    return m_repositories.get_or_create_ground_numeric_effect(numeric_effect->get_assign_operator(),
                                                              ground(numeric_effect->get_function(), binding),
                                                              ground(numeric_effect->get_function_expression(), problem, binding));
}

template GroundNumericEffect<Fluent> ProblemImpl::ground(NumericEffect<Fluent> numeric_effect, Problem problem, const ObjectList& binding);
template GroundNumericEffect<Auxiliary> ProblemImpl::ground(NumericEffect<Auxiliary> numeric_effect, Problem problem, const ObjectList& binding);

// Action

void ProblemImpl::ground_and_fill_vector(const NumericConstraintList& numeric_constraints,
                                         Problem problem,
                                         const ObjectList& binding,
                                         FlatExternalPtrList<const GroundNumericConstraintImpl>& ref_numeric_constraints)
{
    for (const auto& condition : numeric_constraints)
    {
        ref_numeric_constraints.push_back(ground(condition, problem, binding));
    }
}

void ProblemImpl::ground_and_fill_vector(const NumericEffectList<Fluent>& numeric_effects,
                                         Problem problem,
                                         const ObjectList& binding,
                                         GroundNumericEffectList<Fluent>& ref_numeric_effects)
{
    for (const auto& effect : numeric_effects)
    {
        ref_numeric_effects.push_back(ground(effect, problem, binding));
    }
}

void ProblemImpl::ground_and_fill_optional(const std::optional<NumericEffect<Auxiliary>>& numeric_effect,
                                           Problem problem,
                                           const ObjectList& binding,
                                           cista::optional<FlatExternalPtr<const GroundNumericEffectImpl<Auxiliary>>>& ref_numeric_effect)
{
    if (numeric_effect.has_value())
    {
        assert(!ref_numeric_effect.has_value());
        ref_numeric_effect = ground(numeric_effect.value(), problem, binding);
    }
}

GroundAction ProblemImpl::ground(Action action,
                                 Problem problem,
                                 ObjectList binding,
                                 const std::vector<std::vector<IndexList>>& candidate_conditional_effect_objects_by_parameter_index)
{
    /* 1. Check if grounding is cached */

    auto& [action_builder, grounding_table] = m_per_action_datas[action];

    auto it = grounding_table.find(binding);
    if (it != grounding_table.end())
    {
        return it->second;
    }

    /* 2. Ground the action */

    /* Header */

    action_builder.get_index() = m_ground_actions.size();
    action_builder.get_action_index() = action->get_index();
    auto& objects = action_builder.get_objects();
    objects.clear();
    for (const auto& obj : binding)
    {
        objects.push_back(obj->get_index());
    }
    objects.compress();

    /* Conjunctive precondition */
    {
        auto& conjunctive_condition = action_builder.get_conjunctive_condition();
        auto& positive_fluent_precondition = conjunctive_condition.get_positive_precondition<Fluent>();
        auto& negative_fluent_precondition = conjunctive_condition.get_negative_precondition<Fluent>();
        auto& positive_static_precondition = conjunctive_condition.get_positive_precondition<Static>();
        auto& negative_static_precondition = conjunctive_condition.get_negative_precondition<Static>();
        auto& positive_derived_precondition = conjunctive_condition.get_positive_precondition<Derived>();
        auto& negative_derived_precondition = conjunctive_condition.get_negative_precondition<Derived>();
        auto& numeric_constraints = conjunctive_condition.get_numeric_constraints();
        positive_fluent_precondition.clear();
        negative_fluent_precondition.clear();
        positive_static_precondition.clear();
        negative_static_precondition.clear();
        positive_derived_precondition.clear();
        negative_derived_precondition.clear();
        numeric_constraints.clear();
        ground_and_fill_vector(action->get_conjunctive_condition()->get_literals<Fluent>(),
                               positive_fluent_precondition,
                               negative_fluent_precondition,
                               binding);
        ground_and_fill_vector(action->get_conjunctive_condition()->get_literals<Static>(),
                               positive_static_precondition,
                               negative_static_precondition,
                               binding);
        ground_and_fill_vector(action->get_conjunctive_condition()->get_literals<Derived>(),
                               positive_derived_precondition,
                               negative_derived_precondition,
                               binding);
        positive_fluent_precondition.compress();
        negative_fluent_precondition.compress();
        positive_static_precondition.compress();
        negative_static_precondition.compress();
        positive_derived_precondition.compress();
        negative_derived_precondition.compress();
        ground_and_fill_vector(action->get_conjunctive_condition()->get_numeric_constraints(), problem, binding, numeric_constraints);

        /* Conjunctive propositional effects */
        auto& conjunctive_effect = action_builder.get_conjunctive_effect();
        auto& positive_effect = conjunctive_effect.get_positive_effects();
        auto& negative_effect = conjunctive_effect.get_negative_effects();
        positive_effect.clear();
        negative_effect.clear();
        ground_and_fill_vector(action->get_conjunctive_effect()->get_literals(), positive_effect, negative_effect, binding);
        positive_effect.compress();
        negative_effect.compress();

        /* Conjunctive numerical effects */
        auto& fluent_numerical_effects = conjunctive_effect.get_fluent_numeric_effects();
        auto& auxiliary_numerical_effect = conjunctive_effect.get_auxiliary_numeric_effect();
        fluent_numerical_effects.clear();
        auxiliary_numerical_effect = std::nullopt;
        ground_and_fill_vector(action->get_conjunctive_effect()->get_fluent_numeric_effects(), problem, binding, fluent_numerical_effects);
        ground_and_fill_optional(action->get_conjunctive_effect()->get_auxiliary_numeric_effect(), problem, binding, auxiliary_numerical_effect);
    }

    /* Conditional effects */
    // Fetch data
    auto& cond_effects = action_builder.get_conditional_effects();

    // We have copy the binding to extend it with objects for quantified effect variables
    // and at the same time we need to use the original binding as cache key.
    auto binding_cond_effect = binding;

    const auto num_lifted_cond_effects = action->get_conditional_effects().size();
    if (num_lifted_cond_effects > 0)
    {
        size_t j = 0;  ///< position in cond_effects

        for (size_t i = 0; i < num_lifted_cond_effects; ++i)
        {
            const auto& lifted_cond_effect = action->get_conditional_effects().at(i);

            if (lifted_cond_effect->get_arity() > 0)
            {
                const auto& objects_by_parameter_index = candidate_conditional_effect_objects_by_parameter_index.at(i);

                // Resize binding to have additional space for all variables in quantified effect.
                binding_cond_effect.resize(binding.size() + lifted_cond_effect->get_arity());

                for (const auto& binding_cond : create_cartesian_product_generator(objects_by_parameter_index))
                {
                    // Create resulting conditional effect binding.
                    for (size_t pos = 0; pos < lifted_cond_effect->get_arity(); ++pos)
                    {
                        binding_cond_effect[binding.size() + pos] = m_repositories.get_object(binding_cond[pos]);
                    }

                    // Ensure that there is enough space in the action specific builder.
                    // We never clear cond_effects to avoid frequent allocation and deallocation.
                    if (j == cond_effects.size())
                    {
                        cond_effects.resize(j + 1);
                    }
                    auto& cond_effect_j = cond_effects.at(j++);
                    auto& cond_conjunctive_condition_j = cond_effect_j.get_conjunctive_condition();
                    auto& cond_positive_fluent_precondition_j = cond_conjunctive_condition_j.get_positive_precondition<Fluent>();
                    auto& cond_negative_fluent_precondition_j = cond_conjunctive_condition_j.get_negative_precondition<Fluent>();
                    auto& cond_positive_static_precondition_j = cond_conjunctive_condition_j.get_positive_precondition<Static>();
                    auto& cond_negative_static_precondition_j = cond_conjunctive_condition_j.get_negative_precondition<Static>();
                    auto& cond_positive_derived_precondition_j = cond_conjunctive_condition_j.get_positive_precondition<Derived>();
                    auto& cond_negative_derived_precondition_j = cond_conjunctive_condition_j.get_negative_precondition<Derived>();
                    auto& cond_numeric_constraints_j = cond_conjunctive_condition_j.get_numeric_constraints();
                    auto& cond_conjunctive_effect_j = cond_effect_j.get_conjunctive_effect();
                    auto& cond_positive_effect_j = cond_conjunctive_effect_j.get_positive_effects();
                    auto& cond_negative_effect_j = cond_conjunctive_effect_j.get_negative_effects();
                    cond_positive_fluent_precondition_j.clear();
                    cond_negative_fluent_precondition_j.clear();
                    cond_positive_static_precondition_j.clear();
                    cond_negative_static_precondition_j.clear();
                    cond_positive_derived_precondition_j.clear();
                    cond_negative_derived_precondition_j.clear();
                    cond_numeric_constraints_j.clear();
                    cond_positive_effect_j.clear();
                    cond_negative_effect_j.clear();

                    /* Propositional precondition */
                    ground_and_fill_vector(lifted_cond_effect->get_conjunctive_condition()->get_literals<Fluent>(),
                                           cond_positive_fluent_precondition_j,
                                           cond_negative_fluent_precondition_j,
                                           binding_cond_effect);
                    ground_and_fill_vector(lifted_cond_effect->get_conjunctive_condition()->get_literals<Static>(),
                                           cond_positive_static_precondition_j,
                                           cond_negative_static_precondition_j,
                                           binding_cond_effect);
                    ground_and_fill_vector(lifted_cond_effect->get_conjunctive_condition()->get_literals<Derived>(),
                                           cond_positive_derived_precondition_j,
                                           cond_negative_derived_precondition_j,
                                           binding_cond_effect);
                    cond_positive_fluent_precondition_j.compress();
                    cond_negative_fluent_precondition_j.compress();
                    cond_positive_static_precondition_j.compress();
                    cond_negative_static_precondition_j.compress();
                    cond_positive_derived_precondition_j.compress();
                    cond_negative_derived_precondition_j.compress();
                    ground_and_fill_vector(lifted_cond_effect->get_conjunctive_condition()->get_numeric_constraints(),
                                           problem,
                                           binding_cond_effect,
                                           cond_numeric_constraints_j);

                    /* Propositional effect */
                    ground_and_fill_vector(lifted_cond_effect->get_conjunctive_effect()->get_literals(),
                                           cond_positive_effect_j,
                                           cond_negative_effect_j,
                                           binding_cond_effect);

                    /* Numeric effect */
                    auto& cond_fluent_numerical_effects_j = cond_conjunctive_effect_j.get_fluent_numeric_effects();
                    auto& cond_auxiliary_numerical_effect_j = cond_conjunctive_effect_j.get_auxiliary_numeric_effect();
                    cond_fluent_numerical_effects_j.clear();
                    cond_auxiliary_numerical_effect_j = std::nullopt;

                    ground_and_fill_vector(lifted_cond_effect->get_conjunctive_effect()->get_fluent_numeric_effects(),
                                           problem,
                                           binding_cond_effect,
                                           cond_fluent_numerical_effects_j);
                    ground_and_fill_optional(lifted_cond_effect->get_conjunctive_effect()->get_auxiliary_numeric_effect(),
                                             problem,
                                             binding_cond_effect,
                                             cond_auxiliary_numerical_effect_j);
                }
            }
            else
            {
                // Ensure that there is enough space in the action specific builder.
                // We never clear cond_effects to avoid frequent allocation and deallocation.
                if (j == cond_effects.size())
                {
                    cond_effects.resize(j + 1);
                }
                auto& cond_effect = cond_effects.at(j++);
                auto& cond_conjunctive_condition = cond_effect.get_conjunctive_condition();
                auto& cond_positive_fluent_precondition = cond_conjunctive_condition.get_positive_precondition<Fluent>();
                auto& cond_negative_fluent_precondition = cond_conjunctive_condition.get_negative_precondition<Fluent>();
                auto& cond_positive_static_precondition = cond_conjunctive_condition.get_positive_precondition<Static>();
                auto& cond_negative_static_precondition = cond_conjunctive_condition.get_negative_precondition<Static>();
                auto& cond_positive_derived_precondition = cond_conjunctive_condition.get_positive_precondition<Derived>();
                auto& cond_negative_derived_precondition = cond_conjunctive_condition.get_negative_precondition<Derived>();
                auto& cond_numeric_constraints = cond_conjunctive_condition.get_numeric_constraints();
                auto& cond_conjunctive_effect = cond_effect.get_conjunctive_effect();
                auto& cond_positive_effect = cond_conjunctive_effect.get_positive_effects();
                auto& cond_negative_effect = cond_conjunctive_effect.get_negative_effects();
                cond_positive_fluent_precondition.clear();
                cond_negative_fluent_precondition.clear();
                cond_positive_static_precondition.clear();
                cond_negative_static_precondition.clear();
                cond_positive_derived_precondition.clear();
                cond_negative_derived_precondition.clear();
                cond_numeric_constraints.clear();
                cond_positive_effect.clear();
                cond_negative_effect.clear();

                /* Propositional precondition */
                ground_and_fill_vector(lifted_cond_effect->get_conjunctive_condition()->get_literals<Fluent>(),
                                       cond_positive_fluent_precondition,
                                       cond_negative_fluent_precondition,
                                       binding);
                ground_and_fill_vector(lifted_cond_effect->get_conjunctive_condition()->get_literals<Static>(),
                                       cond_positive_static_precondition,
                                       cond_negative_static_precondition,
                                       binding);
                ground_and_fill_vector(lifted_cond_effect->get_conjunctive_condition()->get_literals<Derived>(),
                                       cond_positive_derived_precondition,
                                       cond_negative_derived_precondition,
                                       binding);
                cond_positive_fluent_precondition.compress();
                cond_negative_fluent_precondition.compress();
                cond_positive_static_precondition.compress();
                cond_negative_static_precondition.compress();
                cond_positive_derived_precondition.compress();
                cond_negative_derived_precondition.compress();
                ground_and_fill_vector(lifted_cond_effect->get_conjunctive_condition()->get_numeric_constraints(), problem, binding, cond_numeric_constraints);

                /* Propositional effect */
                ground_and_fill_vector(lifted_cond_effect->get_conjunctive_effect()->get_literals(), cond_positive_effect, cond_negative_effect, binding);

                /* Numeric effect*/
                auto& cond_fluent_numerical_effects_j = cond_conjunctive_effect.get_fluent_numeric_effects();
                auto& cond_auxiliary_numerical_effects_j = cond_conjunctive_effect.get_auxiliary_numeric_effect();
                cond_fluent_numerical_effects_j.clear();
                cond_auxiliary_numerical_effects_j = std::nullopt;

                ground_and_fill_vector(lifted_cond_effect->get_conjunctive_effect()->get_fluent_numeric_effects(),
                                       problem,
                                       binding,
                                       cond_fluent_numerical_effects_j);
                ground_and_fill_optional(lifted_cond_effect->get_conjunctive_effect()->get_auxiliary_numeric_effect(),
                                         problem,
                                         binding,
                                         cond_auxiliary_numerical_effects_j);
            }
        }
    }

    const auto [iter, inserted] = m_ground_actions.insert(action_builder);
    const auto grounded_action = iter->get();
    if (inserted)
    {
        m_ground_actions_by_index.push_back(grounded_action);
    }

    /* 3. Insert to groundings table */

    grounding_table.emplace(std::move(binding), GroundAction(grounded_action));

    /* 4. Return the resulting ground action */

    return grounded_action;
}

const GroundActionList& ProblemImpl::get_ground_actions() const { return m_ground_actions_by_index; }

GroundAction ProblemImpl::get_ground_action(Index action_index) const { return m_ground_actions_by_index.at(action_index); }

size_t ProblemImpl::get_num_ground_actions() const { return m_ground_actions_by_index.size(); }

size_t ProblemImpl::get_estimated_memory_usage_in_bytes_for_actions() const
{
    const auto usage1 = m_ground_actions.get_estimated_memory_usage_in_bytes();
    const auto usage2 = m_ground_actions_by_index.capacity() * sizeof(GroundAction);
    auto usage3 = size_t(0);

    for (const auto& [action, action_data] : m_per_action_datas)
    {
        const auto& [action_builder, grounding_table] = action_data;

        usage3 += get_memory_usage_in_bytes(grounding_table);
    }

    return usage1 + usage2 + usage3;
}

// Axiom

GroundAxiom ProblemImpl::ground(Axiom axiom, Problem problem, ObjectList binding)
{
    /* 1. Check if grounding is cached */

    auto& [axiom_builder, grounding_table] = m_per_axiom_data[axiom];
    auto it = grounding_table.find(binding);
    if (it != grounding_table.end())
    {
        return it->second;
    }

    /* 2. Ground the axiom */

    /* Header */

    axiom_builder.get_index() = m_ground_axioms.size();
    axiom_builder.get_axiom() = axiom->get_index();
    auto& objects = axiom_builder.get_object_indices();
    objects.clear();
    for (const auto& obj : binding)
    {
        objects.push_back(obj->get_index());
    }
    objects.compress();

    /* Precondition */
    auto& conjunctive_condition = axiom_builder.get_conjunctive_condition();
    auto& positive_fluent_precondition = conjunctive_condition.get_positive_precondition<Fluent>();
    auto& negative_fluent_precondition = conjunctive_condition.get_negative_precondition<Fluent>();
    auto& positive_static_precondition = conjunctive_condition.get_positive_precondition<Static>();
    auto& negative_static_precondition = conjunctive_condition.get_negative_precondition<Static>();
    auto& positive_derived_precondition = conjunctive_condition.get_positive_precondition<Derived>();
    auto& negative_derived_precondition = conjunctive_condition.get_negative_precondition<Derived>();
    auto& numeric_constraints = conjunctive_condition.get_numeric_constraints();
    positive_fluent_precondition.clear();
    negative_fluent_precondition.clear();
    positive_static_precondition.clear();
    negative_static_precondition.clear();
    positive_derived_precondition.clear();
    negative_derived_precondition.clear();
    numeric_constraints.clear();
    ground_and_fill_vector(axiom->get_conjunctive_condition()->get_literals<Fluent>(), positive_fluent_precondition, negative_fluent_precondition, binding);
    ground_and_fill_vector(axiom->get_conjunctive_condition()->get_literals<Static>(), positive_static_precondition, negative_static_precondition, binding);
    ground_and_fill_vector(axiom->get_conjunctive_condition()->get_literals<Derived>(), positive_derived_precondition, negative_derived_precondition, binding);
    positive_fluent_precondition.compress();
    negative_fluent_precondition.compress();
    positive_static_precondition.compress();
    negative_static_precondition.compress();
    positive_derived_precondition.compress();
    negative_derived_precondition.compress();
    ground_and_fill_vector(axiom->get_conjunctive_condition()->get_numeric_constraints(), problem, binding, numeric_constraints);

    /* Effect */

    // The effect literal might only use the first few objects of the complete binding
    // Therefore, we can prevent the literal grounding table from unnecessarily growing
    // by restricting the binding to only the relevant part
    const auto effect_literal_arity = axiom->get_literal()->get_atom()->get_arity();
    const auto is_complete_binding_relevant_for_head = (binding.size() == effect_literal_arity);
    const auto grounded_literal = is_complete_binding_relevant_for_head ?
                                      ground(axiom->get_literal(), binding) :
                                      ground(axiom->get_literal(), ObjectList(binding.begin(), binding.begin() + effect_literal_arity));
    assert(!grounded_literal->is_negated());
    axiom_builder.get_derived_effect().is_negated = false;
    axiom_builder.get_derived_effect().atom_index = grounded_literal->get_atom()->get_index();

    const auto [iter, inserted] = m_ground_axioms.insert(axiom_builder);
    const auto grounded_axiom = iter->get();

    if (inserted)
    {
        m_ground_axioms_by_index.push_back(grounded_axiom);
    }

    /* 3. Insert to groundings table */

    grounding_table.emplace(std::move(binding), GroundAxiom(grounded_axiom));

    /* 4. Return the resulting ground axiom */

    return grounded_axiom;
}

const GroundAxiomList& ProblemImpl::get_ground_axioms() const { return m_ground_axioms_by_index; }

GroundAxiom ProblemImpl::get_ground_axiom(Index axiom_index) const { return m_ground_axioms_by_index.at(axiom_index); }

size_t ProblemImpl::get_num_ground_axioms() const { return m_ground_axioms_by_index.size(); }

size_t ProblemImpl::get_estimated_memory_usage_in_bytes_for_axioms() const
{
    const auto usage1 = m_ground_axioms.get_estimated_memory_usage_in_bytes();
    const auto usage2 = m_ground_axioms_by_index.capacity() * sizeof(GroundAxiom);
    auto usage3 = size_t(0);
    // TODO: add memory usage of m_per_axiom_data
    for (const auto& [axiom, action_data] : m_per_axiom_data)
    {
        const auto& [axiom_builder, grounding_table] = action_data;
        // TODO: add memory usage of axiom_builder
        usage3 += get_memory_usage_in_bytes(grounding_table);
    }

    return usage1 + usage2 + usage3;
}

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
