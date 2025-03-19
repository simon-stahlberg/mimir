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

#include "mimir/languages/description_logics/constructor_visitor_formatter.hpp"
#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/general_policies/keywords.hpp"
#include "mimir/languages/general_policies/named_feature.hpp"
#include "mimir/languages/general_policies/visitor_interface.hpp"

namespace mimir::languages::general_policies
{

PositiveBooleanEffectImpl::PositiveBooleanEffectImpl(Index index, NamedFeature<dl::Boolean> feature) : EffectBase(index, feature) {}

bool PositiveBooleanEffectImpl::evaluate_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    return this->m_feature->get_feature()->evaluate(target_context)->get_data();
}

bool PositiveBooleanEffectImpl::evaluate_with_debug_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    std::cout << "[DEBUG] " << keywords::positive_boolean_effect << ": " << std::endl;

    const auto eval = this->m_feature->get_feature()->evaluate(target_context)->get_data();

    std::cout << "feature=" << this->m_feature->get_feature() << " target_value=" << eval << std::endl;

    return eval;
}

void PositiveBooleanEffectImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

NegativeBooleanEffectImpl::NegativeBooleanEffectImpl(Index index, NamedFeature<dl::Boolean> feature) : EffectBase(index, feature) {}

bool NegativeBooleanEffectImpl::evaluate_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    return !this->m_feature->get_feature()->evaluate(target_context)->get_data();
}

bool NegativeBooleanEffectImpl::evaluate_with_debug_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    std::cout << "[DEBUG] " << keywords::negative_boolean_effect << ": " << std::endl;

    const auto eval = this->m_feature->get_feature()->evaluate(target_context)->get_data();

    std::cout << "feature=" << this->m_feature->get_feature() << " target_value=" << eval << std::endl;

    return !eval;
}

void NegativeBooleanEffectImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

UnchangedBooleanEffectImpl::UnchangedBooleanEffectImpl(Index index, NamedFeature<dl::Boolean> feature) : EffectBase(index, feature) {}

bool UnchangedBooleanEffectImpl::evaluate_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    return this->m_feature->get_feature()->evaluate(source_context)->get_data() == this->m_feature->get_feature()->evaluate(target_context)->get_data();
}

bool UnchangedBooleanEffectImpl::evaluate_with_debug_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    std::cout << "[DEBUG] " << keywords::unchanged_boolean_effect << ": " << std::endl;

    const auto source_eval = this->m_feature->get_feature()->evaluate(source_context)->get_data();
    const auto target_eval = this->m_feature->get_feature()->evaluate(target_context)->get_data();

    std::cout << "feature=" << this->m_feature->get_feature() << " source_value=" << source_eval << " target_value=" << target_eval << std::endl;

    return source_eval == target_eval;
}

void UnchangedBooleanEffectImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

IncreaseNumericalEffectImpl::IncreaseNumericalEffectImpl(Index index, NamedFeature<dl::Numerical> feature) : EffectBase(index, feature) {}

bool IncreaseNumericalEffectImpl::evaluate_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    return this->m_feature->get_feature()->evaluate(source_context)->get_data() < this->m_feature->get_feature()->evaluate(target_context)->get_data();
}

bool IncreaseNumericalEffectImpl::evaluate_with_debug_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    std::cout << "[DEBUG] " << keywords::increase_numerical_effect << ": " << std::endl;

    const auto source_eval = this->m_feature->get_feature()->evaluate(source_context)->get_data();
    const auto target_eval = this->m_feature->get_feature()->evaluate(target_context)->get_data();

    std::cout << "feature=" << this->m_feature->get_feature() << " source_value=" << source_eval << " target_value=" << target_eval << std::endl;

    return source_eval < target_eval;
}

void IncreaseNumericalEffectImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

DecreaseNumericalEffectImpl::DecreaseNumericalEffectImpl(Index index, NamedFeature<dl::Numerical> feature) : EffectBase(index, feature) {}

bool DecreaseNumericalEffectImpl::evaluate_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    return this->m_feature->get_feature()->evaluate(source_context)->get_data() > this->m_feature->get_feature()->evaluate(target_context)->get_data();
}

bool DecreaseNumericalEffectImpl::evaluate_with_debug_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    std::cout << "[DEBUG] " << keywords::decrease_numerical_effect << ": " << std::endl;

    const auto source_eval = this->m_feature->get_feature()->evaluate(source_context)->get_data();
    const auto target_eval = this->m_feature->get_feature()->evaluate(target_context)->get_data();

    std::cout << "feature=" << this->m_feature->get_feature() << " source_value=" << source_eval << " target_value=" << target_eval << std::endl;

    return source_eval > target_eval;
}

void DecreaseNumericalEffectImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

UnchangedNumericalEffectImpl::UnchangedNumericalEffectImpl(Index index, NamedFeature<dl::Numerical> feature) : EffectBase(index, feature) {}

bool UnchangedNumericalEffectImpl::evaluate_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    return this->m_feature->get_feature()->evaluate(source_context)->get_data() == this->m_feature->get_feature()->evaluate(target_context)->get_data();
}

bool UnchangedNumericalEffectImpl::evaluate_with_debug_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    std::cout << "[DEBUG] " << keywords::unchanged_numerical_effect << ": " << std::endl;

    const auto source_eval = this->m_feature->get_feature()->evaluate(source_context)->get_data();
    const auto target_eval = this->m_feature->get_feature()->evaluate(target_context)->get_data();

    std::cout << "feature=" << this->m_feature->get_feature() << " source_value=" << source_eval << " target_value=" << target_eval << std::endl;

    return source_eval == target_eval;
}

void UnchangedNumericalEffectImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

}
