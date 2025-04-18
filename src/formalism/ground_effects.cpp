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
#include "mimir/common/hash.hpp"
#include "mimir/common/printers.hpp"
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
                                                         const FlatIndexList* positive_effects,
                                                         const FlatIndexList* negative_effects,
                                                         GroundNumericEffectList<FluentTag> fluent_numeric_effects,
                                                         std::optional<GroundNumericEffect<AuxiliaryTag>> auxiliary_numeric_effect) :
    m_index(index),
    m_positive_effects(positive_effects),
    m_negative_effects(negative_effects),
    m_fluent_numeric_effects(std::move(fluent_numeric_effects)),
    m_auxiliary_numeric_effect(std::move(auxiliary_numeric_effect))
{
    assert(m_positive_effects->is_compressed());
    assert(m_negative_effects->is_compressed());

    assert(std::is_sorted(m_positive_effects->compressed_begin(), m_positive_effects->compressed_end()));
    assert(std::is_sorted(m_negative_effects->compressed_begin(), m_negative_effects->compressed_end()));
    assert(std::is_sorted(m_fluent_numeric_effects.begin(),
                          m_fluent_numeric_effects.end(),
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

/**
 * Pretty printing
 */

template<IsFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, const GroundNumericEffectImpl<F>& element)
{
    write(element, StringFormatter(), out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, const GroundNumericEffectImpl<FluentTag>& element);
template std::ostream& operator<<(std::ostream& out, const GroundNumericEffectImpl<AuxiliaryTag>& element);

template<IsFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, GroundNumericEffect<F> element)
{
    write(*element, AddressFormatter(), out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, GroundNumericEffect<FluentTag> element);
template std::ostream& operator<<(std::ostream& out, GroundNumericEffect<AuxiliaryTag> element);

}

namespace mimir
{

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<formalism::GroundConjunctiveEffect, const formalism::ProblemImpl&>& data)
{
    const auto& [conjunctive_effect, problem] = data;

    const auto positive_literal_indices = conjunctive_effect->get_positive_effects();
    const auto negative_literal_indices = conjunctive_effect->get_negative_effects();

    auto positive_literals = formalism::GroundAtomList<formalism::FluentTag> {};
    auto negative_literals = formalism::GroundAtomList<formalism::FluentTag> {};
    const auto& fluent_numeric_effects = conjunctive_effect->get_fluent_numeric_effects();
    const auto& auxiliary_numeric_effect = conjunctive_effect->get_auxiliary_numeric_effect();

    problem.get_repositories().get_ground_atoms_from_indices<formalism::FluentTag>(positive_literal_indices, positive_literals);
    problem.get_repositories().get_ground_atoms_from_indices<formalism::FluentTag>(negative_literal_indices, negative_literals);

    os << "delete effects=";
    mimir::operator<<(os, negative_literals);
    os << ", " << "add effects=";
    mimir::operator<<(os, positive_literals);
    os << ", fluent numeric effects=";
    mimir::operator<<(os, fluent_numeric_effects);
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
std::ostream& operator<<(std::ostream& os, const std::tuple<formalism::GroundConditionalEffect, const formalism::ProblemImpl&>& data)
{
    const auto& [cond_effect_proxy, problem] = data;

    os << std::make_tuple(cond_effect_proxy->get_conjunctive_condition(), std::cref(problem)) << ", "
       << std::make_tuple(cond_effect_proxy->get_conjunctive_effect(), std::cref(problem));

    return os;
}
}