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
                                                   LiteralLists<StaticTag, FluentTag, DerivedTag> literals,
                                                   GroundLiteralLists<StaticTag, FluentTag, DerivedTag> nullary_ground_literals,
                                                   NumericConstraintList numeric_constraints) :
    m_index(index),
    m_parameters(std::move(parameters)),
    m_literals(std::move(literals)),
    m_nullary_ground_literals(std::move(nullary_ground_literals)),
    m_numeric_constraints(std::move(numeric_constraints))
{
    assert(is_all_unique(get_parameters()));
    assert(is_all_unique(get_literals<StaticTag>()));
    assert(is_all_unique(get_literals<FluentTag>()));
    assert(is_all_unique(get_literals<DerivedTag>()));
    assert(is_all_unique(get_nullary_ground_literals<StaticTag>()));
    assert(is_all_unique(get_nullary_ground_literals<FluentTag>()));
    assert(is_all_unique(get_nullary_ground_literals<DerivedTag>()));
    assert(is_all_unique(get_numeric_constraints()));

    assert(std::is_sorted(get_literals<StaticTag>().begin(),
                          get_literals<StaticTag>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_literals<FluentTag>().begin(),
                          get_literals<FluentTag>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_literals<DerivedTag>().begin(),
                          get_literals<DerivedTag>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_nullary_ground_literals<StaticTag>().begin(),
                          get_nullary_ground_literals<StaticTag>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_nullary_ground_literals<FluentTag>().begin(),
                          get_nullary_ground_literals<FluentTag>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_nullary_ground_literals<DerivedTag>().begin(),
                          get_nullary_ground_literals<DerivedTag>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_numeric_constraints().begin(),
                          get_numeric_constraints().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
}

Index ConjunctiveConditionImpl::get_index() const { return m_index; }

const VariableList& ConjunctiveConditionImpl::get_parameters() const { return m_parameters; }

template<IsStaticOrFluentOrDerivedTag P>
const LiteralList<P>& ConjunctiveConditionImpl::get_literals() const
{
    return boost::hana::at_key(m_literals, boost::hana::type<P> {});
}

template const LiteralList<StaticTag>& ConjunctiveConditionImpl::get_literals<StaticTag>() const;
template const LiteralList<FluentTag>& ConjunctiveConditionImpl::get_literals<FluentTag>() const;
template const LiteralList<DerivedTag>& ConjunctiveConditionImpl::get_literals<DerivedTag>() const;

const LiteralLists<StaticTag, FluentTag, DerivedTag>& ConjunctiveConditionImpl::get_hana_literals() const { return m_literals; }

template<IsStaticOrFluentOrDerivedTag P>
const GroundLiteralList<P>& ConjunctiveConditionImpl::get_nullary_ground_literals() const
{
    return boost::hana::at_key(m_nullary_ground_literals, boost::hana::type<P> {});
}

template const GroundLiteralList<StaticTag>& ConjunctiveConditionImpl::get_nullary_ground_literals<StaticTag>() const;
template const GroundLiteralList<FluentTag>& ConjunctiveConditionImpl::get_nullary_ground_literals<FluentTag>() const;
template const GroundLiteralList<DerivedTag>& ConjunctiveConditionImpl::get_nullary_ground_literals<DerivedTag>() const;

const GroundLiteralLists<StaticTag, FluentTag, DerivedTag>& ConjunctiveConditionImpl::get_hana_nullary_ground_literals() const
{
    return m_nullary_ground_literals;
}

const NumericConstraintList& ConjunctiveConditionImpl::get_numeric_constraints() const { return m_numeric_constraints; }

size_t ConjunctiveConditionImpl::get_arity() const { return m_parameters.size(); }

std::ostream& operator<<(std::ostream& out, const ConjunctiveConditionImpl& element)
{
    write(element, StringFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, ConjunctiveCondition element)
{
    write(*element, AddressFormatter(), out);
    return out;
}
}
