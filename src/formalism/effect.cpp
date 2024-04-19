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

#include "mimir/formalism/effect.hpp"

#include "mimir/formalism/function.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/parameter.hpp"

#include <cassert>
#include <loki/loki.hpp>

namespace mimir
{
EffectImpl::EffectImpl(int identifier, ParameterList quantified_variables, LiteralList conditions, Literal effect) :
    Base(identifier),
    m_quantified_variables(std::move(quantified_variables)),
    m_conditions(std::move(conditions)),
    m_effect(std::move(effect))
{
}

bool EffectImpl::is_structurally_equivalent_to_impl(const EffectImpl& other) const
{
    if (this != &other)
    {
        return (m_quantified_variables == other.m_quantified_variables)
               && (loki::get_sorted_vector(m_conditions) == loki::get_sorted_vector(other.m_conditions)) && m_effect == other.m_effect;
    }
    return true;
}
size_t EffectImpl::hash_impl() const
{
    return loki::hash_combine(loki::hash_container(m_quantified_variables), loki::hash_container(loki::get_sorted_vector(m_conditions)), m_effect);
}
void EffectImpl::str_impl(std::ostream& out, const loki::FormattingOptions& options) const
{
    if (!m_quantified_variables.empty())
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
        out << ") ";
    }

    if (!m_conditions.empty())
    {
        out << "(when (and ";
        for (size_t i = 0; i < m_conditions.size(); ++i)
        {
            if (i != 0)
            {
                out << " ";
            }
            out << *m_conditions[i];
        }
        out << ") ";
    }

    out << *m_effect;

    if (!m_conditions.empty())
    {
        out << ")";
    }

    if (!m_quantified_variables.empty())
    {
        out << ")";
    }
}

const ParameterList& EffectImpl::get_parameters() const { return m_quantified_variables; }

const LiteralList& EffectImpl::get_conditions() const { return m_conditions; }

const Literal& EffectImpl::get_effect() const { return m_effect; }

}
