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
#include "mimir/search/dense_state.hpp"
#include "mimir/search/state.hpp"

#include <ostream>
#include <tuple>

namespace mimir
{

/* GroundEffectNumeric */
template<DynamicFunctionTag F>
GroundEffectNumeric<F>::GroundEffectNumeric(loki::AssignOperatorEnum assign_operator,
                                            GroundFunction<F> function,
                                            GroundFunctionExpression function_expression) :
    m_assign_operator(assign_operator),
    m_function(function),
    m_function_expression(function_expression)
{
}

template<DynamicFunctionTag F>
loki::AssignOperatorEnum& GroundEffectNumeric<F>::get_assign_operator()
{
    return m_assign_operator;
}
template<DynamicFunctionTag F>
loki::AssignOperatorEnum GroundEffectNumeric<F>::get_assign_operator() const
{
    return m_assign_operator;
}

template<DynamicFunctionTag F>
FlatExternalPtr<const GroundFunctionImpl<F>>& GroundEffectNumeric<F>::get_function()
{
    return m_function;
}
template<DynamicFunctionTag F>
FlatExternalPtr<const GroundFunctionImpl<F>> GroundEffectNumeric<F>::get_function() const
{
    return m_function;
}

template<DynamicFunctionTag F>
FlatExternalPtr<const GroundFunctionExpressionImpl>& GroundEffectNumeric<F>::get_function_expression()
{
    return m_function_expression;
}
template<DynamicFunctionTag F>
FlatExternalPtr<const GroundFunctionExpressionImpl> GroundEffectNumeric<F>::get_function_expression() const
{
    return m_function_expression;
}

template class GroundEffectNumeric<Fluent>;
template class GroundEffectNumeric<Auxiliary>;

/* GroundEffectStrips */

FlatIndexList& GroundEffectStrips::get_positive_effects() { return m_positive_effects; }

const FlatIndexList& GroundEffectStrips::get_positive_effects() const { return m_positive_effects; }

FlatIndexList& GroundEffectStrips::get_negative_effects() { return m_negative_effects; }

const FlatIndexList& GroundEffectStrips::get_negative_effects() const { return m_negative_effects; }

template<DynamicFunctionTag F>
GroundEffectNumericList<F>& GroundEffectStrips::get_numeric_effects()
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

template GroundEffectNumericList<Fluent>& GroundEffectStrips::get_numeric_effects<Fluent>();
template GroundEffectNumericList<Auxiliary>& GroundEffectStrips::get_numeric_effects<Auxiliary>();

template<DynamicFunctionTag F>
const GroundEffectNumericList<F>& GroundEffectStrips::get_numeric_effects() const
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

template const GroundEffectNumericList<Fluent>& GroundEffectStrips::get_numeric_effects<Fluent>() const;
template const GroundEffectNumericList<Auxiliary>& GroundEffectStrips::get_numeric_effects<Auxiliary>() const;

/* GroundEffectConditional */

/* Precondition */
GroundConjunctiveCondition& GroundEffectConditional::get_strips_precondition() { return m_strips_condition; }

const GroundConjunctiveCondition& GroundEffectConditional::get_strips_precondition() const { return m_strips_condition; }

/* Effect */
GroundEffectStrips& GroundEffectConditional::get_strips_effect() { return m_strips_effect; }

const GroundEffectStrips& GroundEffectConditional::get_strips_effect() const { return m_strips_effect; }

template<PredicateTag P>
bool GroundEffectConditional::is_applicable(const FlatBitset& atoms) const
{
    return m_strips_condition.is_applicable<P>(atoms);
}

template bool GroundEffectConditional::is_applicable<Static>(const FlatBitset& atoms) const;
template bool GroundEffectConditional::is_applicable<Fluent>(const FlatBitset& atoms) const;
template bool GroundEffectConditional::is_applicable<Derived>(const FlatBitset& atoms) const;

bool GroundEffectConditional::is_applicable(const FlatDoubleList& fluent_numeric_variables) const
{
    return m_strips_condition.is_applicable(fluent_numeric_variables);
}

bool GroundEffectConditional::is_dynamically_applicable(const DenseState& dense_state) const
{
    return m_strips_condition.is_dynamically_applicable(dense_state);
}

bool GroundEffectConditional::is_statically_applicable(Problem problem) const
{
    return m_strips_condition.is_statically_applicable(problem->get_static_initial_positive_atoms_bitset());
}

bool GroundEffectConditional::is_applicable(Problem problem, const DenseState& dense_state) const
{
    return m_strips_condition.is_applicable(problem, dense_state);
}

/**
 * Utils
 */

template<DynamicFunctionTag F>
ContinuousCost evaluate(GroundEffectNumeric<F> effect, const FlatDoubleList& fluent_numeric_variables, const FlatDoubleList& auxiliary_numeric_variables)
{
    auto old_value = ContinuousCost(0);
    if constexpr (std::is_same_v<F, Fluent>)
    {
        if (effect.get_function()->get_index() >= fluent_numeric_variables.size())
        {
            throw std::logic_error("evaluate(effect, fluent_numeric_variables, auxiliary_numeric_variables): undefined fluent function value "
                                   + to_string(effect.get_function()) + ".");
        }
        old_value = fluent_numeric_variables.at(effect.get_function()->get_index());
    }
    else if constexpr (std::is_same_v<F, Auxiliary>)
    {
        if (effect.get_function()->get_index() >= auxiliary_numeric_variables.size())
        {
            throw std::logic_error("evaluate(effect, fluent_numeric_variables, auxiliary_numeric_variables): undefined auxiliary function value "
                                   + to_string(effect.get_function()) + ".");
        }
        old_value = auxiliary_numeric_variables.at(effect.get_function()->get_index());
    }

    const auto new_value = evaluate(effect.get_function_expression().get(), fluent_numeric_variables, auxiliary_numeric_variables);

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

template double evaluate(GroundEffectNumeric<Fluent> effect, const FlatDoubleList& fluent_numeric_variables, const FlatDoubleList& auxiliary_numeric_variables);
template double
evaluate(GroundEffectNumeric<Auxiliary> effect, const FlatDoubleList& fluent_numeric_variables, const FlatDoubleList& auxiliary_numeric_variables);

/**
 * Pretty printing
 */

template<DynamicFunctionTag F>
std::ostream& operator<<(std::ostream& os, const GroundEffectNumeric<F>& element)
{
    os << "(" << to_string(element.get_assign_operator()) << " " << element.get_function() << " " << element.get_function_expression() << ")";

    return os;
}

template std::ostream& operator<<(std::ostream& os, const GroundEffectNumeric<Fluent>& data);
template std::ostream& operator<<(std::ostream& os, const GroundEffectNumeric<Auxiliary>& data);

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundEffectStrips, const PDDLRepositories&>& data)
{
    // TODO(numerical): add numeric effects
    const auto& [strips_effect_proxy, pddl_repositories] = data;

    const auto& positive_effect_bitset = strips_effect_proxy.get_positive_effects();
    const auto& negative_effect_bitset = strips_effect_proxy.get_negative_effects();
    const auto& fluent_numeric_effects = strips_effect_proxy.get_numeric_effects<Fluent>();
    const auto& auxiliary_numeric_effects = strips_effect_proxy.get_numeric_effects<Auxiliary>();

    auto positive_simple_effects = GroundAtomList<Fluent> {};
    auto negative_simple_effects = GroundAtomList<Fluent> {};

    pddl_repositories.get_ground_atoms_from_indices<Fluent>(positive_effect_bitset, positive_simple_effects);
    pddl_repositories.get_ground_atoms_from_indices<Fluent>(negative_effect_bitset, negative_simple_effects);

    os << "delete effects=" << negative_simple_effects << ", " << "add effects=" << positive_simple_effects
       << ", fluent numeric effects=" << fluent_numeric_effects << ", auxiliary numeric effects=" << auxiliary_numeric_effects;

    return os;
}

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundEffectConditional, const PDDLRepositories&>& data)
{
    const auto& [cond_effect_proxy, pddl_repositories] = data;

    os << std::make_tuple(cond_effect_proxy.get_strips_precondition(), std::cref(pddl_repositories)) << ", "
       << std::make_tuple(cond_effect_proxy.get_strips_effect(), std::cref(pddl_repositories));

    return os;
}

}
