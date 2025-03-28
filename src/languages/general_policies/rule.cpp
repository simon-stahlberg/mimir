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

#include "mimir/languages/general_policies/rule.hpp"

#include "mimir/common/debug.hpp"
#include "mimir/languages/general_policies/conditions.hpp"
#include "mimir/languages/general_policies/effects.hpp"
#include "mimir/languages/general_policies/visitor_interface.hpp"

namespace mimir::languages::general_policies
{

RuleImpl::RuleImpl(Index index, ConditionList conditions, EffectList effects) :
    m_index(index),
    m_conditions(std::move(conditions)),
    m_effects(std::move(effects))
{
}

bool RuleImpl::evaluate(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    DEBUG_LOG("[DEBUG] evaluate rule index=" << m_index);

    return std::all_of(get_conditions().begin(), get_conditions().end(), [&](auto&& arg) { return arg->evaluate(source_context); })
           && std::all_of(get_effects().begin(), get_effects().end(), [&](auto&& arg) { return arg->evaluate(source_context, target_context); });
}

void RuleImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index RuleImpl::get_index() const { return m_index; }

const ConditionList& RuleImpl::get_conditions() const { return m_conditions; }

const EffectList& RuleImpl::get_effects() const { return m_effects; }

}
