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
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/metric.hpp"
#include "mimir/formalism/numeric_fluent.hpp"
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
                         NumericFluentList numeric_fluents,
                         GroundLiteralList<Static> static_goal_condition,
                         GroundLiteralList<Fluent> fluent_goal_condition,
                         GroundLiteralList<Derived> derived_goal_condition,
                         std::optional<OptimizationMetric> optimization_metric,
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
    m_numeric_fluents(std::move(numeric_fluents)),
    m_static_goal_condition(std::move(static_goal_condition)),
    m_fluent_goal_condition(std::move(fluent_goal_condition)),
    m_derived_goal_condition(std::move(derived_goal_condition)),
    m_optimization_metric(std::move(optimization_metric)),
    m_axioms(std::move(axioms)),
    m_problem_and_domain_derived_predicates(),
    m_positive_static_initial_atoms(to_ground_atoms(m_static_initial_literals)),
    m_positive_static_initial_atoms_bitset(),
    m_positive_static_initial_atoms_indices(),
    m_positive_static_initial_assignment_set(AssignmentSet<Static>(m_objects.size(), m_domain->get_predicates<Static>())),
    m_positive_fluent_initial_atoms(to_ground_atoms(m_fluent_initial_literals)),
    m_ground_function_to_value(),
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
    assert(is_all_unique(m_numeric_fluents));
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
    assert(std::is_sorted(m_numeric_fluents.begin(), m_numeric_fluents.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(m_static_goal_condition.begin(),
                          m_static_goal_condition.end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(m_fluent_goal_condition.begin(),
                          m_fluent_goal_condition.end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(m_derived_goal_condition.begin(),
                          m_derived_goal_condition.end(),
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

    for (const auto numeric_fluent : get_numeric_fluents())
    {
        m_ground_function_to_value.emplace(numeric_fluent->get_function(), numeric_fluent->get_number());
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

const NumericFluentList& ProblemImpl::get_numeric_fluents() const { return m_numeric_fluents; }

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

const std::optional<OptimizationMetric>& ProblemImpl::get_optimization_metric() const { return m_optimization_metric; }

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

const GroundFunctionMap<ContinuousCost>& ProblemImpl::get_ground_function_to_value() const { return m_ground_function_to_value; }

ContinuousCost ProblemImpl::get_ground_function_value(GroundFunction function) const
{
    auto it = m_ground_function_to_value.find(function);
    if (it != m_ground_function_to_value.end())
    {
        return it->second;
    }
    throw std::runtime_error("ProblemImpl::get_ground_function_value: missing value for ground function: " + to_string(function));
}

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
