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

#include "mimir/formalism/ground_effects.hpp"

#include "mimir/common/declarations.hpp"
#include "mimir/common/formatter.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/ground_function_expressions.hpp"
#include "mimir/formalism/problem.hpp"

#include <ostream>
#include <tuple>

namespace mimir::formalism
{

/* GroundNumericEffect */
template<IsFluentOrAuxiliaryTag F>
GroundNumericEffectImpl<F>::GroundNumericEffectImpl(Index index,
                                                    loki::AssignOperatorEnum assign_operator,
                                                    GroundFunction<F> function,
                                                    GroundFunctionExpression function_expression) :
    m_index(index),
    m_assign_operator(assign_operator),
    m_function(function),
    m_function_expression(function_expression)
{
}

template<IsFluentOrAuxiliaryTag F>
Index GroundNumericEffectImpl<F>::get_index() const
{
    return m_index;
}

template<IsFluentOrAuxiliaryTag F>
loki::AssignOperatorEnum GroundNumericEffectImpl<F>::get_assign_operator() const
{
    return m_assign_operator;
}

template<IsFluentOrAuxiliaryTag F>
GroundFunction<F> GroundNumericEffectImpl<F>::get_function() const
{
    return m_function;
}

template<IsFluentOrAuxiliaryTag F>
GroundFunctionExpression GroundNumericEffectImpl<F>::get_function_expression() const
{
    return m_function_expression;
}

template class GroundNumericEffectImpl<FluentTag>;
template class GroundNumericEffectImpl<AuxiliaryTag>;

/* GroundConjunctiveEffect */

GroundConjunctiveEffectImpl::GroundConjunctiveEffectImpl(Index index,
                                                         HanaContainer<const FlatIndexList*, PositiveTag, NegativeTag> propositional_effects,
                                                         GroundNumericEffectList<FluentTag> fluent_numeric_effects,
                                                         std::optional<GroundNumericEffect<AuxiliaryTag>> auxiliary_numeric_effect) :
    m_index(index),
    m_propositional_effects(propositional_effects),
    m_fluent_numeric_effects(std::move(fluent_numeric_effects)),
    m_auxiliary_numeric_effect(std::move(auxiliary_numeric_effect))
{
    assert(get_compressed_propositional_effects<PositiveTag>().is_compressed());
    assert(get_compressed_propositional_effects<NegativeTag>().is_compressed());

    assert(std::is_sorted(get_compressed_propositional_effects<PositiveTag>().compressed_begin(),
                          get_compressed_propositional_effects<PositiveTag>().compressed_end()));
    assert(std::is_sorted(get_compressed_propositional_effects<NegativeTag>().compressed_begin(),
                          get_compressed_propositional_effects<NegativeTag>().compressed_end()));
    assert(std::is_sorted(get_fluent_numeric_effects().begin(),
                          get_fluent_numeric_effects().end(),
                          [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); }));
}

Index GroundConjunctiveEffectImpl::get_index() const { return m_index; }

const GroundNumericEffectList<FluentTag>& GroundConjunctiveEffectImpl::get_fluent_numeric_effects() const { return m_fluent_numeric_effects; }

const std::optional<GroundNumericEffect<AuxiliaryTag>>& GroundConjunctiveEffectImpl::get_auxiliary_numeric_effect() const { return m_auxiliary_numeric_effect; }

/* GroundConditionalEffect */

GroundConditionalEffectImpl::GroundConditionalEffectImpl(Index index,
                                                         GroundConjunctiveCondition conjunctive_condition,
                                                         GroundConjunctiveEffect conjunctive_effect) :
    m_index(index),
    m_conjunctive_condition(conjunctive_condition),
    m_conjunctive_effect(conjunctive_effect)
{
}

Index GroundConditionalEffectImpl::get_index() const { return m_index; }

GroundConjunctiveCondition GroundConditionalEffectImpl::get_conjunctive_condition() const { return m_conjunctive_condition; }

GroundConjunctiveEffect GroundConditionalEffectImpl::get_conjunctive_effect() const { return m_conjunctive_effect; }

/**
 * Utils
 */

template<IsFluentOrAuxiliaryTag F>
std::pair<loki::AssignOperatorEnum, ContinuousCost>
evaluate(GroundNumericEffect<F> effect, const FlatDoubleList& static_numeric_variables, const FlatDoubleList& fluent_numeric_variables)
{
    return { effect->get_assign_operator(), evaluate(effect->get_function_expression(), static_numeric_variables, fluent_numeric_variables) };
}

template std::pair<loki::AssignOperatorEnum, ContinuousCost>
evaluate(GroundNumericEffect<FluentTag> effect, const FlatDoubleList& static_numeric_variables, const FlatDoubleList& fluent_numeric_variables);
template std::pair<loki::AssignOperatorEnum, ContinuousCost>
evaluate(GroundNumericEffect<AuxiliaryTag> effect, const FlatDoubleList& static_numeric_variables, const FlatDoubleList& fluent_numeric_variables);
}
