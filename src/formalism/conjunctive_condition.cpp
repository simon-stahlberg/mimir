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
#include "mimir/formalism/ground_numeric_constraint.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/numeric_constraint.hpp"
#include "mimir/formalism/variable.hpp"

namespace mimir::formalism
{
ConjunctiveConditionImpl::ConjunctiveConditionImpl(Index index,
                                                   VariableList parameters,
                                                   LiteralLists<Static, Fluent, Derived> literals,
                                                   GroundLiteralLists<Static, Fluent, Derived> nullary_ground_literals,
                                                   NumericConstraintList numeric_constraints) :
    m_index(index),
    m_parameters(std::move(parameters)),
    m_literals(std::move(literals)),
    m_nullary_ground_literals(std::move(nullary_ground_literals)),
    m_numeric_constraints(std::move(numeric_constraints))
{
    assert(is_all_unique(get_parameters()));
    assert(is_all_unique(get_literals<Static>()));
    assert(is_all_unique(get_literals<Fluent>()));
    assert(is_all_unique(get_literals<Derived>()));
    assert(is_all_unique(get_nullary_ground_literals<Static>()));
    assert(is_all_unique(get_nullary_ground_literals<Fluent>()));
    assert(is_all_unique(get_nullary_ground_literals<Derived>()));
    assert(is_all_unique(get_numeric_constraints()));

    assert(std::is_sorted(get_literals<Static>().begin(),
                          get_literals<Static>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_literals<Fluent>().begin(),
                          get_literals<Fluent>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_literals<Derived>().begin(),
                          get_literals<Derived>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_nullary_ground_literals<Static>().begin(),
                          get_nullary_ground_literals<Static>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_nullary_ground_literals<Fluent>().begin(),
                          get_nullary_ground_literals<Fluent>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_nullary_ground_literals<Derived>().begin(),
                          get_nullary_ground_literals<Derived>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_numeric_constraints().begin(),
                          get_numeric_constraints().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
}

Index ConjunctiveConditionImpl::get_index() const { return m_index; }

const VariableList& ConjunctiveConditionImpl::get_parameters() const { return m_parameters; }

template<StaticOrFluentOrDerived P>
const LiteralList<P>& ConjunctiveConditionImpl::get_literals() const
{
    return boost::hana::at_key(m_literals, boost::hana::type<P> {});
}

template const LiteralList<Static>& ConjunctiveConditionImpl::get_literals<Static>() const;
template const LiteralList<Fluent>& ConjunctiveConditionImpl::get_literals<Fluent>() const;
template const LiteralList<Derived>& ConjunctiveConditionImpl::get_literals<Derived>() const;

const LiteralLists<Static, Fluent, Derived>& ConjunctiveConditionImpl::get_hana_literals() const { return m_literals; }

template<StaticOrFluentOrDerived P>
const GroundLiteralList<P>& ConjunctiveConditionImpl::get_nullary_ground_literals() const
{
    return boost::hana::at_key(m_nullary_ground_literals, boost::hana::type<P> {});
}

template const GroundLiteralList<Static>& ConjunctiveConditionImpl::get_nullary_ground_literals<Static>() const;
template const GroundLiteralList<Fluent>& ConjunctiveConditionImpl::get_nullary_ground_literals<Fluent>() const;
template const GroundLiteralList<Derived>& ConjunctiveConditionImpl::get_nullary_ground_literals<Derived>() const;

const GroundLiteralLists<Static, Fluent, Derived>& ConjunctiveConditionImpl::get_hana_nullary_ground_literals() const { return m_nullary_ground_literals; }

const NumericConstraintList& ConjunctiveConditionImpl::get_numeric_constraints() const { return m_numeric_constraints; }

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
