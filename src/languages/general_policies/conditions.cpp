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

#include "mimir/languages/general_policies/conditions.hpp"

#include "mimir/common/debug.hpp"
#include "mimir/languages/description_logics/constructor_visitor_formatter.hpp"
#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/general_policies/keywords.hpp"
#include "mimir/languages/general_policies/named_feature.hpp"
#include "mimir/languages/general_policies/visitor_interface.hpp"

namespace mimir::languages::general_policies
{
PositiveBooleanConditionImpl::PositiveBooleanConditionImpl(Index index, NamedFeature<dl::BooleanTag> feature) : ConditionBase(index, feature) {}

bool PositiveBooleanConditionImpl::evaluate_impl(dl::EvaluationContext& source_context) const
{
    DEBUG_LOG("[DEBUG] " << keywords::positive_boolean_condition << ": feature=" << this->m_feature->get_feature()
                         << " target_value=" << this->m_feature->get_feature()->evaluate(source_context)->get_data())

    return this->m_feature->get_feature()->evaluate(source_context)->get_data();
}

void PositiveBooleanConditionImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

NegativeBooleanConditionImpl::NegativeBooleanConditionImpl(Index index, NamedFeature<dl::BooleanTag> feature) : ConditionBase(index, feature) {}

bool NegativeBooleanConditionImpl::evaluate_impl(dl::EvaluationContext& source_context) const
{
    DEBUG_LOG("[DEBUG] " << keywords::negative_boolean_condition << ": feature=" << this->m_feature->get_feature()
                         << " target_value=" << this->m_feature->get_feature()->evaluate(source_context)->get_data())

    return !this->m_feature->get_feature()->evaluate(source_context)->get_data();
}

void NegativeBooleanConditionImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

GreaterNumericalConditionImpl::GreaterNumericalConditionImpl(Index index, NamedFeature<dl::NumericalTag> feature) : ConditionBase(index, feature) {}

bool GreaterNumericalConditionImpl::evaluate_impl(dl::EvaluationContext& source_context) const
{
    DEBUG_LOG("[DEBUG] " << keywords::greater_numerical_condition << ": feature=" << this->m_feature->get_feature()
                         << " target_value=" << this->m_feature->get_feature()->evaluate(source_context)->get_data())

    return this->m_feature->get_feature()->evaluate(source_context)->get_data() > 0;
}

void GreaterNumericalConditionImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

EqualNumericalConditionImpl::EqualNumericalConditionImpl(Index index, NamedFeature<dl::NumericalTag> feature) : ConditionBase(index, feature) {}

bool EqualNumericalConditionImpl::evaluate_impl(dl::EvaluationContext& source_context) const
{
    DEBUG_LOG("[DEBUG] " << keywords::equal_numerical_condition << ": feature=" << this->m_feature->get_feature()
                         << " target_value=" << this->m_feature->get_feature()->evaluate(source_context)->get_data())

    return this->m_feature->get_feature()->evaluate(source_context)->get_data() == 0;
}

void EqualNumericalConditionImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

}
