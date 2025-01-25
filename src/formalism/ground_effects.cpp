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

#include "mimir/common/concepts.hpp"
#include "mimir/common/hash_cista.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/ground_function_expressions.hpp"
#include "mimir/formalism/repositories.hpp"

#include <ostream>
#include <tuple>

namespace mimir
{

/* GroundNumericEffect */
template<DynamicFunctionTag F>
GroundNumericEffect<F>::GroundNumericEffect(loki::AssignOperatorEnum assign_operator,
                                            GroundFunction<F> function,
                                            GroundFunctionExpression function_expression) :
    m_assign_operator(assign_operator),
    m_function(function),
    m_function_expression(function_expression)
{
}

template<DynamicFunctionTag F>
loki::AssignOperatorEnum& GroundNumericEffect<F>::get_assign_operator()
{
    return m_assign_operator;
}
template<DynamicFunctionTag F>
loki::AssignOperatorEnum GroundNumericEffect<F>::get_assign_operator() const
{
    return m_assign_operator;
}

template<DynamicFunctionTag F>
FlatExternalPtr<const GroundFunctionImpl<F>>& GroundNumericEffect<F>::get_function()
{
    return m_function;
}
template<DynamicFunctionTag F>
FlatExternalPtr<const GroundFunctionImpl<F>> GroundNumericEffect<F>::get_function() const
{
    return m_function;
}

template<DynamicFunctionTag F>
FlatExternalPtr<const GroundFunctionExpressionImpl>& GroundNumericEffect<F>::get_function_expression()
{
    return m_function_expression;
}
template<DynamicFunctionTag F>
FlatExternalPtr<const GroundFunctionExpressionImpl> GroundNumericEffect<F>::get_function_expression() const
{
    return m_function_expression;
}

template class GroundNumericEffect<Fluent>;
template class GroundNumericEffect<Auxiliary>;

/* GroundConjunctiveEffect */

FlatIndexList& GroundConjunctiveEffect::get_positive_effects() { return m_positive_effects; }

const FlatIndexList& GroundConjunctiveEffect::get_positive_effects() const { return m_positive_effects; }

FlatIndexList& GroundConjunctiveEffect::get_negative_effects() { return m_negative_effects; }

const FlatIndexList& GroundConjunctiveEffect::get_negative_effects() const { return m_negative_effects; }

template<DynamicFunctionTag F>
GroundNumericEffectList<F>& GroundConjunctiveEffect::get_numeric_effects()
{
    if constexpr (std::is_same_v<F, Fluent>)
    {
        return m_fluent_numeric_effects;
    }
    else if constexpr (std::is_same_v<F, Auxiliary>)
    {
        return m_auxiliary_numeric_effects;
    }
    else
    {
        static_assert(dependent_false<F>::value, "Missing implementation for DynamicFunctionTag.");
    }
}

template GroundNumericEffectList<Fluent>& GroundConjunctiveEffect::get_numeric_effects<Fluent>();
template GroundNumericEffectList<Auxiliary>& GroundConjunctiveEffect::get_numeric_effects<Auxiliary>();

template<DynamicFunctionTag F>
const GroundNumericEffectList<F>& GroundConjunctiveEffect::get_numeric_effects() const
{
    if constexpr (std::is_same_v<F, Fluent>)
    {
        return m_fluent_numeric_effects;
    }
    else if constexpr (std::is_same_v<F, Auxiliary>)
    {
        return m_auxiliary_numeric_effects;
    }
    else
    {
        static_assert(dependent_false<F>::value, "Missing implementation for DynamicFunctionTag.");
    }
}

template const GroundNumericEffectList<Fluent>& GroundConjunctiveEffect::get_numeric_effects<Fluent>() const;
template const GroundNumericEffectList<Auxiliary>& GroundConjunctiveEffect::get_numeric_effects<Auxiliary>() const;

/* GroundConditionalEffect */

/* Precondition */
GroundConjunctiveCondition& GroundConditionalEffect::get_conjunctive_condition() { return m_conjunctive_condition; }

const GroundConjunctiveCondition& GroundConditionalEffect::get_conjunctive_condition() const { return m_conjunctive_condition; }

/* Effect */
GroundConjunctiveEffect& GroundConditionalEffect::get_conjunctive_effect() { return m_conjunctive_effect; }

const GroundConjunctiveEffect& GroundConditionalEffect::get_conjunctive_effect() const { return m_conjunctive_effect; }

/**
 * Utils
 */

template<DynamicFunctionTag F>
ContinuousCost evaluate(GroundNumericEffect<F> effect, const FlatDoubleList& fluent_numeric_variables, const FlatDoubleList& auxiliary_numeric_variables)
{
    auto old_value = ContinuousCost(0);
    if constexpr (std::is_same_v<F, Fluent>)
    {
        if (effect.get_function()->get_index() >= fluent_numeric_variables.size())
        {
            return UNDEFINED_CONTINUOUS_COST;
        }
        old_value = fluent_numeric_variables.at(effect.get_function()->get_index());
    }
    else if constexpr (std::is_same_v<F, Auxiliary>)
    {
        if (effect.get_function()->get_index() >= auxiliary_numeric_variables.size())
        {
            return UNDEFINED_CONTINUOUS_COST;
        }
        old_value = auxiliary_numeric_variables.at(effect.get_function()->get_index());
    }

    const auto new_value = evaluate(effect.get_function_expression().get(), fluent_numeric_variables, auxiliary_numeric_variables);

    if (new_value == UNDEFINED_CONTINUOUS_COST)
    {
        return UNDEFINED_CONTINUOUS_COST;
    }

    switch (effect.get_assign_operator())
    {
        case loki::AssignOperatorEnum::ASSIGN:
        {
            return new_value;
        }
        case loki::AssignOperatorEnum::DECREASE:
        {
            return old_value - new_value;
        }
        case loki::AssignOperatorEnum::INCREASE:
        {
            return old_value + new_value;
        }
        case loki::AssignOperatorEnum::SCALE_DOWN:
        {
            return old_value / new_value;
        }
        case loki::AssignOperatorEnum::SCALE_UP:
        {
            return old_value * new_value;
        }
        default:
        {
            throw std::logic_error("evaluate(effect, fluent_numeric_variables, auxiliary_numeric_variables): Unexpected loki::AssignOperatorEnum.");
        }
    }
}

template double evaluate(GroundNumericEffect<Fluent> effect, const FlatDoubleList& fluent_numeric_variables, const FlatDoubleList& auxiliary_numeric_variables);
template double
evaluate(GroundNumericEffect<Auxiliary> effect, const FlatDoubleList& fluent_numeric_variables, const FlatDoubleList& auxiliary_numeric_variables);

/**
 * Pretty printing
 */

template<DynamicFunctionTag F>
std::ostream& operator<<(std::ostream& os, const GroundNumericEffect<F>& element)
{
    os << "(" << to_string(element.get_assign_operator()) << " " << element.get_function() << " " << element.get_function_expression() << ")";

    return os;
}

template std::ostream& operator<<(std::ostream& os, const GroundNumericEffect<Fluent>& data);
template std::ostream& operator<<(std::ostream& os, const GroundNumericEffect<Auxiliary>& data);

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundConjunctiveEffect, const PDDLRepositories&>& data)
{
    // TODO(numerical): add numeric effects
    const auto& [conjunctive_effect, pddl_repositories] = data;

    const auto& positive_effect_bitset = conjunctive_effect.get_positive_effects();
    const auto& negative_effect_bitset = conjunctive_effect.get_negative_effects();
    const auto& fluent_numeric_effects = conjunctive_effect.get_numeric_effects<Fluent>();
    const auto& auxiliary_numeric_effects = conjunctive_effect.get_numeric_effects<Auxiliary>();

    auto positive_simple_effects = GroundAtomList<Fluent> {};
    auto negative_simple_effects = GroundAtomList<Fluent> {};

    pddl_repositories.get_ground_atoms_from_indices<Fluent>(positive_effect_bitset, positive_simple_effects);
    pddl_repositories.get_ground_atoms_from_indices<Fluent>(negative_effect_bitset, negative_simple_effects);

    os << "delete effects=" << negative_simple_effects << ", " << "add effects=" << positive_simple_effects
       << ", fluent numeric effects=" << fluent_numeric_effects << ", auxiliary numeric effects=" << auxiliary_numeric_effects;

    return os;
}

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundConditionalEffect, const PDDLRepositories&>& data)
{
    const auto& [cond_effect_proxy, pddl_repositories] = data;

    os << std::make_tuple(cond_effect_proxy.get_conjunctive_condition(), std::cref(pddl_repositories)) << ", "
       << std::make_tuple(cond_effect_proxy.get_conjunctive_effect(), std::cref(pddl_repositories));

    return os;
}

}
