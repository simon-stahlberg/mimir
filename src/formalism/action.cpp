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

#include "mimir/common/collections.hpp"
#include "mimir/formalism/atom.hpp"
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/variable.hpp"

#include <cassert>
#include <loki/loki.hpp>

namespace mimir
{
ActionImpl::ActionImpl(int identifier,
                       std::string name,
                       VariableList parameters,
                       LiteralList conditions,
                       LiteralList static_conditions,
                       LiteralList fluent_conditions,
                       EffectSimpleList simple_effects,
                       EffectConditionalList conditional_effects,
                       EffectUniversalList universal_effects,
                       FunctionExpression function_expression) :
    Base(identifier),
    m_name(std::move(name)),
    m_parameters(std::move(parameters)),
    m_conditions(std::move(conditions)),
    m_static_conditions(std::move(static_conditions)),
    m_fluent_conditions(std::move(fluent_conditions)),
    m_simple_effects(std::move(simple_effects)),
    m_conditional_effects(std::move(conditional_effects)),
    m_universal_effects(std::move(universal_effects)),
    m_function_expression(std::move(function_expression))
{
    assert(is_subseteq(m_static_conditions, m_conditions));
    assert(is_subseteq(m_fluent_conditions, m_conditions));
    assert(is_all_unique(m_parameters));
    assert(is_all_unique(m_conditions));
    assert(is_all_unique(m_static_conditions));
    assert(is_all_unique(m_fluent_conditions));
    assert(is_all_unique(m_simple_effects));
    assert(is_all_unique(m_conditional_effects));
    assert(is_all_unique(m_universal_effects));
}

bool ActionImpl::is_structurally_equivalent_to_impl(const ActionImpl& other) const
{
    if (this != &other)
    {
        return (m_name == other.m_name) && (loki::get_sorted_vector(m_parameters) == loki::get_sorted_vector(other.m_parameters))
               && (loki::get_sorted_vector(m_conditions) == loki::get_sorted_vector(other.m_conditions))
               && (loki::get_sorted_vector(m_simple_effects) == loki::get_sorted_vector(other.m_simple_effects))
               && (loki::get_sorted_vector(m_conditional_effects) == loki::get_sorted_vector(other.m_conditional_effects))
               && (loki::get_sorted_vector(m_universal_effects) == loki::get_sorted_vector(other.m_universal_effects))
               && (m_function_expression == other.m_function_expression);
    }
    return true;
}

size_t ActionImpl::hash_impl() const
{
    return loki::hash_combine(m_name,
                              loki::hash_container(m_parameters),
                              loki::hash_container(loki::get_sorted_vector(m_conditions)),
                              loki::hash_container(loki::get_sorted_vector(m_simple_effects)),
                              loki::hash_container(loki::get_sorted_vector(m_conditional_effects)),
                              loki::hash_container(loki::get_sorted_vector(m_universal_effects)),
                              m_function_expression);
}

void ActionImpl::str_impl(std::ostream& out, const loki::FormattingOptions& options) const { return str(out, options, true); }

void ActionImpl::str(std::ostream& out, const loki::FormattingOptions& options, bool action_costs) const
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
    if (m_conditions.empty())
    {
        out << "()\n";
    }
    else
    {
        out << "(and ";
        for (size_t i = 0; i < m_conditions.size(); ++i)
        {
            if (i != 0)
            {
                out << " ";
            }
            out << *m_conditions[i];
        }
        out << ")\n";
    }

    out << std::string(nested_options.indent, ' ') << ":effects ";
    if (m_simple_effects.empty() && m_conditional_effects.empty() && m_universal_effects.empty())
    {
        out << "()\n";
    }
    else
    {
        out << "(and";
        for (const auto& effect : m_simple_effects)
        {
            out << " " << *effect;
        }
        for (const auto& effect : m_conditional_effects)
        {
            out << " " << *effect;
        }
        for (const auto& effect : m_universal_effects)
        {
            out << " " << *effect;
        }
        if (action_costs)
        {
            out << " "
                << "(increase total-cost ";
            std::visit(loki::StringifyVisitor(out, options), *m_function_expression);
            out << ")";
        }
        out << ")";  // end and
    }

    out << ")\n";  // end action
}

const std::string& ActionImpl::get_name() const { return m_name; }

const VariableList& ActionImpl::get_parameters() const { return m_parameters; }

const LiteralList& ActionImpl::get_conditions() const { return m_conditions; }

const LiteralList& ActionImpl::get_static_conditions() const { return m_static_conditions; }

const LiteralList& ActionImpl::get_fluent_conditions() const { return m_fluent_conditions; }

const EffectSimpleList& ActionImpl::get_simple_effects() const { return m_simple_effects; }

const EffectConditionalList& ActionImpl::get_conditional_effects() const { return m_conditional_effects; }

const EffectUniversalList& ActionImpl::get_universal_effects() const { return m_universal_effects; }

const FunctionExpression& ActionImpl::get_function_expression() const { return m_function_expression; }

size_t ActionImpl::get_arity() const { return m_parameters.size(); }

}
