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

#include "mimir/formalism/action.hpp"

#include "formatter.hpp"
#include "mimir/common/collections.hpp"
#include "mimir/common/concepts.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/atom.hpp"
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/variable.hpp"

#include <cassert>

namespace mimir
{
ActionImpl::ActionImpl(Index index,
                       std::string name,
                       size_t original_arity,
                       VariableList parameters,
                       LiteralList<Static> static_conditions,
                       LiteralList<Fluent> fluent_conditions,
                       LiteralList<Derived> derived_conditions,
                       EffectSimpleList simple_effects,
                       EffectComplexList complex_effects,
                       FunctionExpression function_expression) :
    m_index(index),
    m_name(std::move(name)),
    m_original_arity(std::move(original_arity)),
    m_parameters(std::move(parameters)),
    m_static_conditions(std::move(static_conditions)),
    m_fluent_conditions(std::move(fluent_conditions)),
    m_derived_conditions(std::move(derived_conditions)),
    m_simple_effects(std::move(simple_effects)),
    m_complex_effects(std::move(complex_effects)),
    m_function_expression(std::move(function_expression))
{
    assert(m_original_arity <= m_parameters.size());
    assert(is_all_unique(m_parameters));
    assert(is_all_unique(m_static_conditions));
    assert(is_all_unique(m_fluent_conditions));
    assert(is_all_unique(m_derived_conditions));
    assert(is_all_unique(m_simple_effects));
    assert(is_all_unique(m_complex_effects));
    assert(
        std::is_sorted(m_static_conditions.begin(), m_static_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(
        std::is_sorted(m_fluent_conditions.begin(), m_fluent_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(
        std::is_sorted(m_derived_conditions.begin(), m_derived_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(m_simple_effects.begin(), m_simple_effects.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(m_complex_effects.begin(),
                          m_complex_effects.end(),
                          [](const auto& l, const auto& r)
                          {
                              if (l->get_effect()->is_negated() == r->get_effect()->is_negated())
                              {
                                  return l->get_index() < r->get_index();
                              }
                              return l->get_effect()->is_negated() > r->get_effect()->is_negated();
                          }));
}

std::string ActionImpl::str() const
{
    auto out = std::stringstream();
    out << *this;
    return out.str();
}

Index ActionImpl::get_index() const { return m_index; }

const std::string& ActionImpl::get_name() const { return m_name; }

size_t ActionImpl::get_original_arity() const { return m_original_arity; }

const VariableList& ActionImpl::get_parameters() const { return m_parameters; }

template<PredicateTag P>
const LiteralList<P>& ActionImpl::get_conditions() const
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

template const LiteralList<Static>& ActionImpl::get_conditions<Static>() const;
template const LiteralList<Fluent>& ActionImpl::get_conditions<Fluent>() const;
template const LiteralList<Derived>& ActionImpl::get_conditions<Derived>() const;

const EffectSimpleList& ActionImpl::get_simple_effects() const { return m_simple_effects; }

const EffectComplexList& ActionImpl::get_complex_effects() const { return m_complex_effects; }

const FunctionExpression& ActionImpl::get_function_expression() const { return m_function_expression; }

size_t ActionImpl::get_arity() const { return m_parameters.size(); }

std::ostream& operator<<(std::ostream& out, const ActionImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, Action element)
{
    out << *element;
    return out;
}

}
