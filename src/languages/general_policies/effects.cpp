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

#include "mimir/languages/general_policies/effects.hpp"

#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/general_policies/named_feature.hpp"

namespace mimir::languages::general_policies
{

PositiveBooleanEffectImpl::PositiveBooleanEffectImpl(NamedFeature<dl::Boolean> feature) : EffectBase(feature) {}

bool PositiveBooleanEffectImpl::evaluate_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    return this->m_feature->get_feature()->evaluate(target_context)->get_data();
}

NegativeBooleanEffectImpl::NegativeBooleanEffectImpl(NamedFeature<dl::Boolean> feature) : EffectBase(feature) {}

bool NegativeBooleanEffectImpl::evaluate_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    return !this->m_feature->get_feature()->evaluate(target_context)->get_data();
}

UnchangedBooleanEffectImpl::UnchangedBooleanEffectImpl(NamedFeature<dl::Boolean> feature) : EffectBase(feature) {}

bool UnchangedBooleanEffectImpl::evaluate_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    return this->m_feature->get_feature()->evaluate(source_context)->get_data() == this->m_feature->get_feature()->evaluate(target_context)->get_data();
}

IncreaseNumericalEffectImpl::IncreaseNumericalEffectImpl(NamedFeature<dl::Numerical> feature) : EffectBase(feature) {}

bool IncreaseNumericalEffectImpl::evaluate_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    return this->m_feature->get_feature()->evaluate(source_context)->get_data() < this->m_feature->get_feature()->evaluate(target_context)->get_data();
}

DecreaseNumericalEffectImpl::DecreaseNumericalEffectImpl(NamedFeature<dl::Numerical> feature) : EffectBase(feature) {}

bool DecreaseNumericalEffectImpl::evaluate_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    return this->m_feature->get_feature()->evaluate(source_context)->get_data() > this->m_feature->get_feature()->evaluate(target_context)->get_data();
}

UnchangedNumericalEffectImpl::UnchangedNumericalEffectImpl(NamedFeature<dl::Numerical> feature) : EffectBase(feature) {}

bool UnchangedNumericalEffectImpl::evaluate_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    return this->m_feature->get_feature()->evaluate(source_context)->get_data() == this->m_feature->get_feature()->evaluate(target_context)->get_data();
}

}
