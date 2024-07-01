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

#include "mimir/common/collections.hpp"
#include "mimir/formalism/function.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/variable.hpp"

#include <cassert>
#include <loki/loki.hpp>

namespace mimir
{

/**
 * Type 1 effect
 */

EffectSimpleImpl::EffectSimpleImpl(int identifier, Literal<Fluent> effect) : Base(identifier), m_effect(std::move(effect)) {}

bool EffectSimpleImpl::is_structurally_equivalent_to_impl(const EffectSimpleImpl& other) const
{
    if (this != &other)
    {
        return m_effect == other.m_effect;
    }
    return true;
}
size_t EffectSimpleImpl::hash_impl() const { return loki::hash_combine(m_effect); }

void EffectSimpleImpl::str_impl(std::ostream& out, const loki::FormattingOptions& options) const { out << *m_effect; }

const Literal<Fluent>& EffectSimpleImpl::get_effect() const { return m_effect; }

/**
 * Type 2 effect
 */

EffectConditionalImpl::EffectConditionalImpl(int identifier,
                                             LiteralList<Static> static_conditions,
                                             LiteralList<Fluent> fluent_conditions,
                                             LiteralList<Derived> derived_conditions,
                                             Literal<Fluent> effect) :
    Base(identifier),
    m_static_conditions(std::move(static_conditions)),
    m_fluent_conditions(std::move(fluent_conditions)),
    m_derived_conditions(std::move(derived_conditions)),
    m_effect(std::move(effect))
{
    assert(is_all_unique(m_static_conditions));
    assert(is_all_unique(m_fluent_conditions));
    assert(is_all_unique(m_derived_conditions));
}

bool EffectConditionalImpl::is_structurally_equivalent_to_impl(const EffectConditionalImpl& other) const
{
    if (this != &other)
    {
        return (loki::get_sorted_vector(m_static_conditions) == loki::get_sorted_vector(other.m_static_conditions))
               && (loki::get_sorted_vector(m_fluent_conditions) == loki::get_sorted_vector(other.m_fluent_conditions))
               && (loki::get_sorted_vector(m_derived_conditions) == loki::get_sorted_vector(other.m_derived_conditions)) && m_effect == other.m_effect;
    }
    return true;
}
size_t EffectConditionalImpl::hash_impl() const
{
    return loki::hash_combine(loki::hash_container(loki::get_sorted_vector(m_static_conditions)),
                              loki::hash_container(loki::get_sorted_vector(m_fluent_conditions)),
                              loki::hash_container(loki::get_sorted_vector(m_derived_conditions)),
                              m_effect);
}

void EffectConditionalImpl::str_impl(std::ostream& out, const loki::FormattingOptions& options) const
{
    out << "(when (and";
    for (const auto& condition : m_static_conditions)
    {
        out << " " << *condition;
    }
    for (const auto& condition : m_fluent_conditions)
    {
        out << " " << *condition;
    }
    for (const auto& condition : m_derived_conditions)
    {
        out << " " << *condition;
    }
    out << " ) ";  // end and

    out << *m_effect;

    out << ")";  // end when
}

const Literal<Fluent>& EffectConditionalImpl::get_effect() const { return m_effect; }

/**
 * Type 3 effect
 */

EffectUniversalImpl::EffectUniversalImpl(int identifier,
                                         VariableList quantified_variables,
                                         LiteralList<Static> static_conditions,
                                         LiteralList<Fluent> fluent_conditions,
                                         LiteralList<Derived> derived_conditions,
                                         Literal<Fluent> effect) :
    Base(identifier),
    m_quantified_variables(std::move(quantified_variables)),
    m_static_conditions(std::move(static_conditions)),
    m_fluent_conditions(std::move(fluent_conditions)),
    m_derived_conditions(std::move(derived_conditions)),
    m_effect(std::move(effect))
{
    assert(!m_quantified_variables.empty());
    assert(is_all_unique(m_quantified_variables));
    assert(is_all_unique(m_static_conditions));
    assert(is_all_unique(m_fluent_conditions));
    assert(is_all_unique(m_derived_conditions));
}

bool EffectUniversalImpl::is_structurally_equivalent_to_impl(const EffectUniversalImpl& other) const
{
    if (this != &other)
    {
        return (m_quantified_variables == other.m_quantified_variables)
               && (loki::get_sorted_vector(m_static_conditions) == loki::get_sorted_vector(other.m_static_conditions))
               && (loki::get_sorted_vector(m_fluent_conditions) == loki::get_sorted_vector(other.m_fluent_conditions))
               && (loki::get_sorted_vector(m_derived_conditions) == loki::get_sorted_vector(other.m_derived_conditions)) && m_effect == other.m_effect;
    }
    return true;
}
size_t EffectUniversalImpl::hash_impl() const
{
    return loki::hash_combine(loki::hash_container(m_quantified_variables),
                              loki::hash_container(loki::get_sorted_vector(m_static_conditions)),
                              loki::hash_container(loki::get_sorted_vector(m_fluent_conditions)),
                              loki::hash_container(loki::get_sorted_vector(m_derived_conditions)),
                              m_effect);
}

void EffectUniversalImpl::str_impl(std::ostream& out, const loki::FormattingOptions& options) const
{
    out << "(forall (";
    for (size_t i = 0; i < m_quantified_variables.size(); ++i)
    {
        if (i != 0)
        {
            out << " ";
        }
        out << *m_quantified_variables[i];
    }
    out << ") ";  // end quantifiers

    if (!(m_static_conditions.empty() && m_fluent_conditions.empty()))
    {
        out << "(when (and";
        for (const auto& condition : m_static_conditions)
        {
            out << " " << *condition;
        }
        for (const auto& condition : m_fluent_conditions)
        {
            out << " " << *condition;
        }
        for (const auto& condition : m_derived_conditions)
        {
            out << " " << *condition;
        }
        out << " ) ";  // end and
    }

    out << *m_effect;

    if (!(m_static_conditions.empty() && m_fluent_conditions.empty()))
    {
        out << ")";  // end when
    }

    out << ")";  // end forall
}

const VariableList& EffectUniversalImpl::get_parameters() const { return m_quantified_variables; }

const Literal<Fluent>& EffectUniversalImpl::get_effect() const { return m_effect; }

size_t EffectUniversalImpl::get_arity() const { return m_quantified_variables.size(); }

}
