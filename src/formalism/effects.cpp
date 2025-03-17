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

#include "mimir/formalism/effects.hpp"

#include "formatter.hpp"
#include "mimir/common/collections.hpp"
#include "mimir/common/concepts.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/conjunctive_condition.hpp"
#include "mimir/formalism/function.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/numeric_constraint.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/variable.hpp"

#include <cassert>

namespace mimir::formalism
{

/**
 * EffectNumeric
 */

template<FluentOrAuxiliary F>
NumericEffectImpl<F>::NumericEffectImpl(Index index, loki::AssignOperatorEnum assign_operator, Function<F> function, FunctionExpression function_expression) :
    m_index(index),
    m_assign_operator(assign_operator),
    m_function(function),
    m_function_expression(function_expression)
{
}

template<FluentOrAuxiliary F>
Index NumericEffectImpl<F>::get_index() const
{
    return m_index;
}

template<FluentOrAuxiliary F>
loki::AssignOperatorEnum NumericEffectImpl<F>::get_assign_operator() const
{
    return m_assign_operator;
}

template<FluentOrAuxiliary F>
Function<F> NumericEffectImpl<F>::get_function() const
{
    return m_function;
}

template<FluentOrAuxiliary F>
FunctionExpression NumericEffectImpl<F>::get_function_expression() const
{
    return m_function_expression;
}

template class NumericEffectImpl<Fluent>;
template class NumericEffectImpl<Auxiliary>;

/**
 * Type 1 effect
 */

ConjunctiveEffectImpl::ConjunctiveEffectImpl(Index index,
                                             VariableList parameters,
                                             LiteralList<Fluent> effects,
                                             NumericEffectList<Fluent> fluent_numeric_effects,
                                             std::optional<NumericEffect<Auxiliary>> auxiliary_numeric_effect) :
    m_index(index),
    m_parameters(std::move(parameters)),
    m_literals(std::move(effects)),
    m_fluent_numeric_effects(std::move(fluent_numeric_effects)),
    m_auxiliary_numeric_effect(std::move(auxiliary_numeric_effect))
{
    assert(is_all_unique(m_literals));
    assert(std::is_sorted(m_literals.begin(), m_literals.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(m_fluent_numeric_effects.begin(),
                          m_fluent_numeric_effects.end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
}

Index ConjunctiveEffectImpl::get_index() const { return m_index; }

const VariableList& ConjunctiveEffectImpl::get_parameters() const { return m_parameters; }

const LiteralList<Fluent>& ConjunctiveEffectImpl::get_literals() const { return m_literals; }

const NumericEffectList<Fluent>& ConjunctiveEffectImpl::get_fluent_numeric_effects() const { return m_fluent_numeric_effects; }

std::optional<NumericEffect<Auxiliary>> ConjunctiveEffectImpl::get_auxiliary_numeric_effect() const { return m_auxiliary_numeric_effect; }

/**
 * Type 3 effect
 */

ConditionalEffectImpl::ConditionalEffectImpl(Index index, ConjunctiveCondition conjunctive_condition, ConjunctiveEffect conjunctive_effect) :
    m_index(index),
    m_conjunctive_condition(std::move(conjunctive_condition)),
    m_conjunctive_effect(std::move(conjunctive_effect))
{
    assert(conjunctive_condition);
    assert(conjunctive_effect);
}

Index ConditionalEffectImpl::get_index() const { return m_index; }

ConjunctiveCondition ConditionalEffectImpl::get_conjunctive_condition() const { return m_conjunctive_condition; }

ConjunctiveEffect ConditionalEffectImpl::get_conjunctive_effect() const { return m_conjunctive_effect; }

size_t ConditionalEffectImpl::get_arity() const { return m_conjunctive_condition->get_arity(); }

template<FluentOrAuxiliary F>
std::ostream& operator<<(std::ostream& out, const NumericEffectImpl<F>& element)
{
    write(element, StringFormatter(), out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, const NumericEffectImpl<Fluent>& element);
template std::ostream& operator<<(std::ostream& out, const NumericEffectImpl<Auxiliary>& element);

std::ostream& operator<<(std::ostream& out, const ConjunctiveEffectImpl& element)
{
    write(element, StringFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, const ConditionalEffectImpl& element)
{
    write(element, StringFormatter(), out);
    return out;
}

template<FluentOrAuxiliary F>
std::ostream& operator<<(std::ostream& out, NumericEffect<F> element)
{
    write(*element, AddressFormatter(), out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, NumericEffect<Fluent> element);
template std::ostream& operator<<(std::ostream& out, NumericEffect<Auxiliary> element);

std::ostream& operator<<(std::ostream& out, ConjunctiveEffect element)
{
    write(*element, AddressFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, ConditionalEffect element)
{
    write(*element, AddressFormatter(), out);
    return out;
}

}
