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

#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/general_policies/named_feature.hpp"

namespace mimir::languages::general_policies
{
PositiveBooleanConditionImpl::PositiveBooleanConditionImpl(NamedFeature<dl::Boolean> feature) : ConditionBase(feature) {}

bool PositiveBooleanConditionImpl::evaluate_impl(dl::EvaluationContext& source_context) const
{
    return this->m_feature->get_feature()->evaluate(source_context)->get_data();
}

NegativeBooleanConditionImpl::NegativeBooleanConditionImpl(NamedFeature<dl::Boolean> feature) : ConditionBase(feature) {}

bool NegativeBooleanConditionImpl::evaluate_impl(dl::EvaluationContext& source_context) const
{
    return !this->m_feature->get_feature()->evaluate(source_context)->get_data();
}

GreaterNumericalConditionImpl::GreaterNumericalConditionImpl(NamedFeature<dl::Numerical> feature) : ConditionBase(feature) {}

bool GreaterNumericalConditionImpl::evaluate_impl(dl::EvaluationContext& source_context) const
{
    return this->m_feature->get_feature()->evaluate(source_context)->get_data() > 0;
}

EqualNumericalConditionImpl::EqualNumericalConditionImpl(NamedFeature<dl::Numerical> feature) : ConditionBase(feature) {}

bool EqualNumericalConditionImpl::evaluate_impl(dl::EvaluationContext& source_context) const
{
    return this->m_feature->get_feature()->evaluate(source_context)->get_data() == 0;
}

}
