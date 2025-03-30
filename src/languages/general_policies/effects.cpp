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

#include "mimir/common/debug.hpp"
#include "mimir/languages/description_logics/constructor_visitor_formatter.hpp"
#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/general_policies/keywords.hpp"
#include "mimir/languages/general_policies/named_feature.hpp"
#include "mimir/languages/general_policies/visitor_interface.hpp"

namespace mimir::languages::general_policies
{

PositiveBooleanEffectImpl::PositiveBooleanEffectImpl(Index index, NamedFeature<dl::BooleanTag> feature) : EffectBase(index, feature) {}

bool PositiveBooleanEffectImpl::evaluate_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    DEBUG_LOG("[DEBUG] " << keywords::positive_boolean_effect << ": feature=" << this->m_feature->get_feature()
                         << " target_value=" << this->m_feature->get_feature()->evaluate(target_context)->get_data())

    return this->m_feature->get_feature()->evaluate(target_context)->get_data();
}

void PositiveBooleanEffectImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

NegativeBooleanEffectImpl::NegativeBooleanEffectImpl(Index index, NamedFeature<dl::BooleanTag> feature) : EffectBase(index, feature) {}

bool NegativeBooleanEffectImpl::evaluate_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    DEBUG_LOG("[DEBUG] " << keywords::negative_boolean_effect << ": feature=" << this->m_feature->get_feature()
                         << " target_value=" << this->m_feature->get_feature()->evaluate(target_context)->get_data())

    return !this->m_feature->get_feature()->evaluate(target_context)->get_data();
}

void NegativeBooleanEffectImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

UnchangedBooleanEffectImpl::UnchangedBooleanEffectImpl(Index index, NamedFeature<dl::BooleanTag> feature) : EffectBase(index, feature) {}

bool UnchangedBooleanEffectImpl::evaluate_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    DEBUG_LOG("[DEBUG] " << keywords::unchanged_boolean_effect << ": feature=" << this->m_feature->get_feature()
                         << " source_value=" << this->m_feature->get_feature()->evaluate(source_context)->get_data()
                         << " target_value=" << this->m_feature->get_feature()->evaluate(target_context)->get_data())

    return this->m_feature->get_feature()->evaluate(source_context)->get_data() == this->m_feature->get_feature()->evaluate(target_context)->get_data();
}

void UnchangedBooleanEffectImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

IncreaseNumericalEffectImpl::IncreaseNumericalEffectImpl(Index index, NamedFeature<dl::NumericalTag> feature) : EffectBase(index, feature) {}

bool IncreaseNumericalEffectImpl::evaluate_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    DEBUG_LOG("[DEBUG] " << keywords::evaluate_with_debug_impl << ": feature=" << this->m_feature->get_feature()
                         << " source_value=" << this->m_feature->get_feature()->evaluate(source_context)->get_data()
                         << " target_value=" << this->m_feature->get_feature()->evaluate(target_context)->get_data())

    return this->m_feature->get_feature()->evaluate(source_context)->get_data() < this->m_feature->get_feature()->evaluate(target_context)->get_data();
}

void IncreaseNumericalEffectImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

DecreaseNumericalEffectImpl::DecreaseNumericalEffectImpl(Index index, NamedFeature<dl::NumericalTag> feature) : EffectBase(index, feature) {}

bool DecreaseNumericalEffectImpl::evaluate_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    DEBUG_LOG("[DEBUG] " << keywords::decrease_numerical_effect << ": feature=" << this->m_feature->get_feature()
                         << " source_value=" << this->m_feature->get_feature()->evaluate(source_context)->get_data()
                         << " target_value=" << this->m_feature->get_feature()->evaluate(target_context)->get_data())

    return this->m_feature->get_feature()->evaluate(source_context)->get_data() > this->m_feature->get_feature()->evaluate(target_context)->get_data();
}

void DecreaseNumericalEffectImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

UnchangedNumericalEffectImpl::UnchangedNumericalEffectImpl(Index index, NamedFeature<dl::NumericalTag> feature) : EffectBase(index, feature) {}

bool UnchangedNumericalEffectImpl::evaluate_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    DEBUG_LOG("[DEBUG] " << keywords::unchanged_numerical_effect << ": feature=" << this->m_feature->get_feature()
                         << " source_value=" << this->m_feature->get_feature()->evaluate(source_context)->get_data()
                         << " target_value=" << this->m_feature->get_feature()->evaluate(target_context)->get_data())

    return this->m_feature->get_feature()->evaluate(source_context)->get_data() == this->m_feature->get_feature()->evaluate(target_context)->get_data();
}

void UnchangedNumericalEffectImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

}
