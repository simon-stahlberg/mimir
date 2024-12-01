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
#include "mimir/common/hash.hpp"
#include "mimir/formalism/function.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/variable.hpp"

#include <cassert>

namespace mimir
{

/**
 * Type 1 effect
 */

EffectStripsImpl::EffectStripsImpl(Index index, LiteralList<Fluent> effects, FunctionExpression function_expression) :
    m_index(index),
    m_effects(std::move(effects)),
    m_function_expression(std::move(function_expression))
{
    assert(is_all_unique(m_effects));
    assert(std::is_sorted(m_effects.begin(), m_effects.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
}

Index EffectStripsImpl::get_index() const { return m_index; }

const LiteralList<Fluent>& EffectStripsImpl::get_effects() const { return m_effects; }

const FunctionExpression& EffectStripsImpl::get_function_expression() const { return m_function_expression; }

/**
 * Type 3 effect
 */

EffectConditionalImpl::EffectConditionalImpl(Index index,
                                             VariableList quantified_variables,
                                             LiteralList<Static> static_conditions,
                                             LiteralList<Fluent> fluent_conditions,
                                             LiteralList<Derived> derived_conditions,
                                             LiteralList<Fluent> effects,
                                             FunctionExpression function_expression) :
    m_index(index),
    m_quantified_variables(std::move(quantified_variables)),
    m_static_conditions(std::move(static_conditions)),
    m_fluent_conditions(std::move(fluent_conditions)),
    m_derived_conditions(std::move(derived_conditions)),
    m_effects(std::move(effects)),
    m_function_expression(std::move(function_expression))
{
    assert(is_all_unique(m_quantified_variables));
    assert(is_all_unique(m_static_conditions));
    assert(is_all_unique(m_fluent_conditions));
    assert(is_all_unique(m_derived_conditions));
    assert(is_all_unique(m_effects));
    assert(
        std::is_sorted(m_static_conditions.begin(), m_static_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(
        std::is_sorted(m_fluent_conditions.begin(), m_fluent_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(
        std::is_sorted(m_derived_conditions.begin(), m_derived_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(m_effects.begin(), m_effects.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
}

Index EffectConditionalImpl::get_index() const { return m_index; }

const VariableList& EffectConditionalImpl::get_parameters() const { return m_quantified_variables; }

template<PredicateTag P>
const LiteralList<P>& EffectConditionalImpl::get_conditions() const
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

template const LiteralList<Static>& EffectConditionalImpl::get_conditions<Static>() const;
template const LiteralList<Fluent>& EffectConditionalImpl::get_conditions<Fluent>() const;
template const LiteralList<Derived>& EffectConditionalImpl::get_conditions<Derived>() const;

const LiteralList<Fluent>& EffectConditionalImpl::get_effects() const { return m_effects; }

const FunctionExpression& EffectConditionalImpl::get_function_expression() const { return m_function_expression; }

size_t EffectConditionalImpl::get_arity() const { return m_quantified_variables.size(); }

std::ostream& operator<<(std::ostream& out, const EffectStripsImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, const EffectConditionalImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, EffectStrips element)
{
    out << *element;
    return out;
}

std::ostream& operator<<(std::ostream& out, EffectConditional element)
{
    out << *element;
    return out;
}

}
