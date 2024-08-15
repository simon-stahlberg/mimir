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

EffectSimpleImpl::EffectSimpleImpl(size_t index, Literal<Fluent> effect) : m_index(index), m_effect(std::move(effect)) {}

std::string EffectSimpleImpl::str() const
{
    auto out = std::stringstream();
    out << *this;
    return out.str();
}

size_t EffectSimpleImpl::get_index() const { return m_index; }

const Literal<Fluent>& EffectSimpleImpl::get_effect() const { return m_effect; }

/**
 * Type 2 effect
 */

EffectConditionalImpl::EffectConditionalImpl(size_t index,
                                             LiteralList<Static> static_conditions,
                                             LiteralList<Fluent> fluent_conditions,
                                             LiteralList<Derived> derived_conditions,
                                             Literal<Fluent> effect) :
    m_index(index),
    m_static_conditions(std::move(static_conditions)),
    m_fluent_conditions(std::move(fluent_conditions)),
    m_derived_conditions(std::move(derived_conditions)),
    m_effect(std::move(effect))
{
    assert(is_all_unique(m_static_conditions));
    assert(is_all_unique(m_fluent_conditions));
    assert(is_all_unique(m_derived_conditions));
    assert(
        std::is_sorted(m_static_conditions.begin(), m_static_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(
        std::is_sorted(m_fluent_conditions.begin(), m_fluent_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(
        std::is_sorted(m_derived_conditions.begin(), m_derived_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
}

std::string EffectConditionalImpl::str() const
{
    auto out = std::stringstream();
    out << *this;
    return out.str();
}

size_t EffectConditionalImpl::get_index() const { return m_index; }

template<PredicateCategory P>
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
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template const LiteralList<Static>& EffectConditionalImpl::get_conditions<Static>() const;
template const LiteralList<Fluent>& EffectConditionalImpl::get_conditions<Fluent>() const;
template const LiteralList<Derived>& EffectConditionalImpl::get_conditions<Derived>() const;

const Literal<Fluent>& EffectConditionalImpl::get_effect() const { return m_effect; }

/**
 * Type 3 effect
 */

EffectUniversalImpl::EffectUniversalImpl(size_t index,
                                         VariableList quantified_variables,
                                         LiteralList<Static> static_conditions,
                                         LiteralList<Fluent> fluent_conditions,
                                         LiteralList<Derived> derived_conditions,
                                         Literal<Fluent> effect) :
    m_index(index),
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
    assert(
        std::is_sorted(m_static_conditions.begin(), m_static_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(
        std::is_sorted(m_fluent_conditions.begin(), m_fluent_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(
        std::is_sorted(m_derived_conditions.begin(), m_derived_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
}

std::string EffectUniversalImpl::str() const
{
    auto out = std::stringstream();
    out << *this;
    return out.str();
}

size_t EffectUniversalImpl::get_index() const { return m_index; }

const VariableList& EffectUniversalImpl::get_parameters() const { return m_quantified_variables; }

template<PredicateCategory P>
const LiteralList<P>& EffectUniversalImpl::get_conditions() const
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
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template const LiteralList<Static>& EffectUniversalImpl::get_conditions<Static>() const;
template const LiteralList<Fluent>& EffectUniversalImpl::get_conditions<Fluent>() const;
template const LiteralList<Derived>& EffectUniversalImpl::get_conditions<Derived>() const;

const Literal<Fluent>& EffectUniversalImpl::get_effect() const { return m_effect; }

size_t EffectUniversalImpl::get_arity() const { return m_quantified_variables.size(); }

std::ostream& operator<<(std::ostream& out, const EffectSimpleImpl& element)
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

std::ostream& operator<<(std::ostream& out, const EffectUniversalImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

}
