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

#include "mimir/formalism/conjunctive_condition.hpp"

#include "formatter.hpp"
#include "mimir/common/collections.hpp"
#include "mimir/common/concepts.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/numeric_constraint.hpp"
#include "mimir/formalism/variable.hpp"

namespace mimir
{
ConjunctiveConditionImpl::ConjunctiveConditionImpl(Index index,
                                                   VariableList parameters,
                                                   LiteralList<Static> static_conditions,
                                                   LiteralList<Fluent> fluent_conditions,
                                                   LiteralList<Derived> derived_conditions,
                                                   GroundLiteralList<Static> nullary_static_conditions,
                                                   GroundLiteralList<Fluent> nullary_fluent_conditions,
                                                   GroundLiteralList<Derived> nullary_derived_conditions,
                                                   NumericConstraintList numeric_constraints,
                                                   GroundNumericConstraintList nullary_numeric_constraints) :
    m_index(index),
    m_parameters(std::move(parameters)),
    m_static_conditions(std::move(static_conditions)),
    m_fluent_conditions(std::move(fluent_conditions)),
    m_derived_conditions(std::move(derived_conditions)),
    m_nullary_static_conditions(std::move(nullary_static_conditions)),
    m_nullary_fluent_conditions(std::move(nullary_fluent_conditions)),
    m_nullary_derived_conditions(std::move(nullary_derived_conditions)),
    m_numeric_constraints(std::move(numeric_constraints)),
    m_nullary_numeric_constraints(std::move(nullary_numeric_constraints))
{
    assert(is_all_unique(m_parameters));
    assert(is_all_unique(m_static_conditions));
    assert(is_all_unique(m_fluent_conditions));
    assert(is_all_unique(m_derived_conditions));
    assert(is_all_unique(m_nullary_static_conditions));
    assert(is_all_unique(m_nullary_fluent_conditions));
    assert(is_all_unique(m_nullary_derived_conditions));
    assert(is_all_unique(m_numeric_constraints));
    assert(is_all_unique(m_nullary_numeric_constraints));

    assert(
        std::is_sorted(m_static_conditions.begin(), m_static_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(
        std::is_sorted(m_fluent_conditions.begin(), m_fluent_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(
        std::is_sorted(m_derived_conditions.begin(), m_derived_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(m_nullary_static_conditions.begin(),
                          m_nullary_static_conditions.end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(m_nullary_fluent_conditions.begin(),
                          m_nullary_fluent_conditions.end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(m_nullary_derived_conditions.begin(),
                          m_nullary_derived_conditions.end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(m_numeric_constraints.begin(),
                          m_numeric_constraints.end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(m_nullary_numeric_constraints.begin(),
                          m_nullary_numeric_constraints.end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
}

Index ConjunctiveConditionImpl::get_index() const { return m_index; }

const VariableList& ConjunctiveConditionImpl::get_parameters() const { return m_parameters; }

template<StaticOrFluentOrDerived P>
const LiteralList<P>& ConjunctiveConditionImpl::get_literals() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_static_conditions;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_fluent_conditions;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_derived_conditions;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for StaticOrFluentOrDerived.");
    }
}

template const LiteralList<Static>& ConjunctiveConditionImpl::get_literals<Static>() const;
template const LiteralList<Fluent>& ConjunctiveConditionImpl::get_literals<Fluent>() const;
template const LiteralList<Derived>& ConjunctiveConditionImpl::get_literals<Derived>() const;

template<StaticOrFluentOrDerived P>
const GroundLiteralList<P>& ConjunctiveConditionImpl::get_nullary_ground_literals() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_nullary_static_conditions;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_nullary_fluent_conditions;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_nullary_derived_conditions;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for StaticOrFluentOrDerived.");
    }
}

template const GroundLiteralList<Static>& ConjunctiveConditionImpl::get_nullary_ground_literals<Static>() const;
template const GroundLiteralList<Fluent>& ConjunctiveConditionImpl::get_nullary_ground_literals<Fluent>() const;
template const GroundLiteralList<Derived>& ConjunctiveConditionImpl::get_nullary_ground_literals<Derived>() const;

const NumericConstraintList& ConjunctiveConditionImpl::get_numeric_constraints() const { return m_numeric_constraints; }

const GroundNumericConstraintList& ConjunctiveConditionImpl::get_nullary_numeric_constraints() const { return m_nullary_numeric_constraints; }

size_t ConjunctiveConditionImpl::get_arity() const { return m_parameters.size(); }

std::ostream& operator<<(std::ostream& out, const ConjunctiveConditionImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, ConjunctiveCondition element)
{
    out << *element;
    return out;
}
}
