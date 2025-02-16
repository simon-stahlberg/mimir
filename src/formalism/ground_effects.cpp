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

#include "formatter.hpp"
#include "mimir/common/concepts.hpp"
#include "mimir/common/hash_cista.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/ground_function_expressions.hpp"
#include "mimir/formalism/repositories.hpp"

#include <ostream>
#include <tuple>

namespace mimir
{

/* GroundNumericEffect */
template<FluentOrAuxiliary F>
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

template<FluentOrAuxiliary F>
Index GroundNumericEffectImpl<F>::get_index() const
{
    return m_index;
}

template<FluentOrAuxiliary F>
loki::AssignOperatorEnum GroundNumericEffectImpl<F>::get_assign_operator() const
{
    return m_assign_operator;
}

template<FluentOrAuxiliary F>
const GroundFunction<F>& GroundNumericEffectImpl<F>::get_function() const
{
    return m_function;
}

template<FluentOrAuxiliary F>
const GroundFunctionExpression& GroundNumericEffectImpl<F>::get_function_expression() const
{
    return m_function_expression;
}

template class GroundNumericEffectImpl<Fluent>;
template class GroundNumericEffectImpl<Auxiliary>;

/* GroundConjunctiveEffect */

FlatIndexList& GroundConjunctiveEffect::get_positive_effects() { return m_positive_effects; }

const FlatIndexList& GroundConjunctiveEffect::get_positive_effects() const { return m_positive_effects; }

FlatIndexList& GroundConjunctiveEffect::get_negative_effects() { return m_negative_effects; }

const FlatIndexList& GroundConjunctiveEffect::get_negative_effects() const { return m_negative_effects; }

GroundNumericEffectList<Fluent>& GroundConjunctiveEffect::get_fluent_numeric_effects() { return m_fluent_numeric_effects; }

const GroundNumericEffectList<Fluent>& GroundConjunctiveEffect::get_fluent_numeric_effects() const { return m_fluent_numeric_effects; }

cista::optional<FlatExternalPtr<const GroundNumericEffectImpl<Auxiliary>>>& GroundConjunctiveEffect::get_auxiliary_numeric_effect()
{
    return m_auxiliary_numeric_effect;
}

const cista::optional<FlatExternalPtr<const GroundNumericEffectImpl<Auxiliary>>>& GroundConjunctiveEffect::get_auxiliary_numeric_effect() const
{
    return m_auxiliary_numeric_effect;
}

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

template<FluentOrAuxiliary F>
std::pair<loki::AssignOperatorEnum, ContinuousCost> evaluate(GroundNumericEffect<F> effect, const FlatDoubleList& fluent_numeric_variables)
{
    return { effect->get_assign_operator(), evaluate(effect->get_function_expression(), fluent_numeric_variables) };
}

template std::pair<loki::AssignOperatorEnum, ContinuousCost> evaluate(GroundNumericEffect<Fluent> effect, const FlatDoubleList& fluent_numeric_variables);
template std::pair<loki::AssignOperatorEnum, ContinuousCost> evaluate(GroundNumericEffect<Auxiliary> effect, const FlatDoubleList& fluent_numeric_variables);

/**
 * Pretty printing
 */

template<FluentOrAuxiliary F>
std::ostream& operator<<(std::ostream& out, const GroundNumericEffectImpl<F>& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, const GroundNumericEffectImpl<Fluent>& element);
template std::ostream& operator<<(std::ostream& out, const GroundNumericEffectImpl<Auxiliary>& element);

template<FluentOrAuxiliary F>
std::ostream& operator<<(std::ostream& out, GroundNumericEffect<F> element)
{
    out << *element;
    return out;
}

template std::ostream& operator<<(std::ostream& out, GroundNumericEffect<Fluent> element);
template std::ostream& operator<<(std::ostream& out, GroundNumericEffect<Auxiliary> element);

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundConjunctiveEffect, const PDDLRepositories&>& data)
{
    const auto& [conjunctive_effect, pddl_repositories] = data;

    const auto& positive_literal_indices = conjunctive_effect.get_positive_effects();
    const auto& negative_literal_indices = conjunctive_effect.get_negative_effects();

    auto positive_literals = GroundAtomList<Fluent> {};
    auto negative_literals = GroundAtomList<Fluent> {};
    const auto& fluent_numeric_effects = conjunctive_effect.get_fluent_numeric_effects();
    const auto& auxiliary_numeric_effect = conjunctive_effect.get_auxiliary_numeric_effect();

    pddl_repositories.get_ground_atoms_from_indices<Fluent>(positive_literal_indices, positive_literals);
    pddl_repositories.get_ground_atoms_from_indices<Fluent>(negative_literal_indices, negative_literals);

    os << "delete effects=" << negative_literals << ", " << "add effects=" << positive_literals << ", fluent numeric effects=" << fluent_numeric_effects;
    if (auxiliary_numeric_effect)
    {
        os << ", auxiliary numeric effects=" << auxiliary_numeric_effect.value();
    }
    else
    {
        os << ", no auxiliary numeric effects";
    }

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
