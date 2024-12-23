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

#include "mimir/formalism/existentially_quantified_conjunctive_condition.hpp"

#include "formatter.hpp"
#include "mimir/common/collections.hpp"
#include "mimir/common/concepts.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/variable.hpp"

namespace mimir
{
ExistentiallyQuantifiedConjunctiveConditionImpl::ExistentiallyQuantifiedConjunctiveConditionImpl(Index index,
                                                                                                 VariableList parameters,
                                                                                                 LiteralList<Static> static_conditions,
                                                                                                 LiteralList<Fluent> fluent_conditions,
                                                                                                 LiteralList<Derived> derived_conditions,
                                                                                                 GroundLiteralList<Static> nullary_static_conditions,
                                                                                                 GroundLiteralList<Fluent> nullary_fluent_conditions,
                                                                                                 GroundLiteralList<Derived> nullary_derived_conditions) :
    m_index(index),
    m_parameters(std::move(parameters)),
    m_static_conditions(std::move(static_conditions)),
    m_fluent_conditions(std::move(fluent_conditions)),
    m_derived_conditions(std::move(derived_conditions)),
    m_nullary_static_conditions(std::move(nullary_static_conditions)),
    m_nullary_fluent_conditions(std::move(nullary_fluent_conditions)),
    m_nullary_derived_conditions(std::move(nullary_derived_conditions))
{
    assert(is_all_unique(m_parameters));
    assert(is_all_unique(m_static_conditions));
    assert(is_all_unique(m_fluent_conditions));
    assert(is_all_unique(m_derived_conditions));

    assert(
        std::is_sorted(m_static_conditions.begin(), m_static_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(
        std::is_sorted(m_fluent_conditions.begin(), m_fluent_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(
        std::is_sorted(m_derived_conditions.begin(), m_derived_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
}

Index ExistentiallyQuantifiedConjunctiveConditionImpl::get_index() const { return m_index; }

const VariableList& ExistentiallyQuantifiedConjunctiveConditionImpl::get_parameters() const { return m_parameters; }

template<PredicateTag P>
const LiteralList<P>& ExistentiallyQuantifiedConjunctiveConditionImpl::get_literals() const
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
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template const LiteralList<Static>& ExistentiallyQuantifiedConjunctiveConditionImpl::get_literals<Static>() const;
template const LiteralList<Fluent>& ExistentiallyQuantifiedConjunctiveConditionImpl::get_literals<Fluent>() const;
template const LiteralList<Derived>& ExistentiallyQuantifiedConjunctiveConditionImpl::get_literals<Derived>() const;

template<PredicateTag P>
const GroundLiteralList<P>& ExistentiallyQuantifiedConjunctiveConditionImpl::get_nullary_ground_literals() const
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
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template const GroundLiteralList<Static>& ExistentiallyQuantifiedConjunctiveConditionImpl::get_nullary_ground_literals<Static>() const;
template const GroundLiteralList<Fluent>& ExistentiallyQuantifiedConjunctiveConditionImpl::get_nullary_ground_literals<Fluent>() const;
template const GroundLiteralList<Derived>& ExistentiallyQuantifiedConjunctiveConditionImpl::get_nullary_ground_literals<Derived>() const;

size_t ExistentiallyQuantifiedConjunctiveConditionImpl::get_arity() const { return m_parameters.size(); }

std::ostream& operator<<(std::ostream& out, const ExistentiallyQuantifiedConjunctiveConditionImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, ExistentiallyQuantifiedConjunctiveCondition element)
{
    out << *element;
    return out;
}
}
