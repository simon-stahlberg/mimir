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

#include "mimir/formalism/atom.hpp"
#include "mimir/formalism/effect.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/parameter.hpp"
#include "mimir/formalism/predicate.hpp"

#include <loki/loki.hpp>

namespace mimir
{
ActionImpl::ActionImpl(int identifier,
                       std::string name,
                       ParameterList parameters,
                       LiteralList condition,
                       EffectList effect,
                       FunctionExpression function_expression) :
    Base(identifier),
    m_name(std::move(name)),
    m_parameters(std::move(parameters)),
    m_condition(std::move(condition)),
    m_effect(std::move(effect)),
    m_function_expression(std::move(function_expression))
{
}

bool ActionImpl::is_structurally_equivalent_to_impl(const ActionImpl& other) const
{
    return (m_name == other.m_name) && (loki::get_sorted_vector(m_parameters) == loki::get_sorted_vector(other.m_parameters))
           && (loki::get_sorted_vector(m_condition) == loki::get_sorted_vector(other.m_condition))
           && (loki::get_sorted_vector(m_effect) == loki::get_sorted_vector(other.m_effect)) && (m_function_expression == other.m_function_expression);
}

size_t ActionImpl::hash_impl() const
{
    return loki::hash_combine(m_name,
                              loki::hash_container(m_parameters),
                              loki::hash_container(loki::get_sorted_vector(m_condition)),
                              loki::hash_container(loki::get_sorted_vector(m_effect)),
                              m_function_expression);
}

void ActionImpl::str_impl(std::ostream& out, const loki::FormattingOptions& options) const
{
    auto nested_options = loki::FormattingOptions { options.indent + options.add_indent, options.add_indent };
    out << std::string(options.indent, ' ') << "(:action " << m_name << "\n" << std::string(nested_options.indent, ' ') << ":parameters (";
    for (size_t i = 0; i < m_parameters.size(); ++i)
    {
        if (i != 0)
            out << " ";
        m_parameters[i]->str(out, options);
    }
    out << ")\n";

    out << std::string(nested_options.indent, ' ') << ":conditions ";
    if (m_condition.empty())
    {
        out << "()\n";
    }
    else
    {
        out << "(and ";
        for (size_t i = 0; i < m_condition.size(); ++i)
        {
            if (i != 0)
            {
                out << " ";
            }
            out << *m_condition[i];
        }
        out << ")\n";
    }

    out << std::string(nested_options.indent, ' ') << ":effects ";
    if (m_effect.empty())
    {
        out << "()\n";
    }
    else
    {
        out << "(and ";
        for (size_t i = 0; i < m_effect.size(); ++i)
        {
            if (i != 0)
            {
                out << " ";
            }
            out << *m_effect[i];
        }
        out << ")";
    }

    out << ")\n";
}

const std::string& ActionImpl::get_name() const { return m_name; }

const ParameterList& ActionImpl::get_parameters() const { return m_parameters; }

const LiteralList& ActionImpl::get_conditions() const { return m_condition; }

const EffectList& ActionImpl::get_effects() const { return m_effect; }

const FunctionExpression& ActionImpl::get_function_expression() const { return m_function_expression; }

size_t ActionImpl::get_arity() const { return m_parameters.size(); }

/// @brief Returns true if the predicate is present in the effect, otherwise false.
bool ActionImpl::affects(Predicate predicate) const
{
    for (const auto& simple_effect : m_effect)
    {
        if (simple_effect->get_effect()->get_atom()->get_predicate() == predicate)
        {
            return true;
        }
    }

    return false;
}

/// @brief Returns true if the predicate is present in the effect of any action, otherwise false.
bool any_affects(const ActionList& actions, Predicate predicate)
{
    for (const auto& action : actions)
    {
        if (action->affects(predicate))
        {
            return true;
        }
    }

    return false;
}
}
