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

#ifndef MIMIR_LANGUAGES_GENERAL_POLICIES_EFFECTS_HPP_
#define MIMIR_LANGUAGES_GENERAL_POLICIES_EFFECTS_HPP_

#include "mimir/languages/general_policies/declarations.hpp"
#include "mimir/languages/general_policies/effect_base.hpp"

namespace mimir::languages::general_policies
{
class PositiveBooleanEffectImpl : public EffectBase<PositiveBooleanEffectImpl, dl::BooleanTag>
{
private:
    PositiveBooleanEffectImpl(Index index, NamedFeature<dl::BooleanTag> feature);

    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    bool evaluate_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class EffectBase<PositiveBooleanEffectImpl, dl::BooleanTag>;
};

class NegativeBooleanEffectImpl : public EffectBase<NegativeBooleanEffectImpl, dl::BooleanTag>
{
private:
    NegativeBooleanEffectImpl(Index index, NamedFeature<dl::BooleanTag> feature);

    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    bool evaluate_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class EffectBase<NegativeBooleanEffectImpl, dl::BooleanTag>;
};

class UnchangedBooleanEffectImpl : public EffectBase<UnchangedBooleanEffectImpl, dl::BooleanTag>
{
private:
    UnchangedBooleanEffectImpl(Index index, NamedFeature<dl::BooleanTag> feature);

    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    bool evaluate_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class EffectBase<UnchangedBooleanEffectImpl, dl::BooleanTag>;
};

class IncreaseNumericalEffectImpl : public EffectBase<IncreaseNumericalEffectImpl, dl::NumericalTag>
{
private:
    IncreaseNumericalEffectImpl(Index index, NamedFeature<dl::NumericalTag> feature);

    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    bool evaluate_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class EffectBase<IncreaseNumericalEffectImpl, dl::NumericalTag>;
};

class DecreaseNumericalEffectImpl : public EffectBase<DecreaseNumericalEffectImpl, dl::NumericalTag>
{
private:
    DecreaseNumericalEffectImpl(Index index, NamedFeature<dl::NumericalTag> feature);

    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    bool evaluate_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class EffectBase<DecreaseNumericalEffectImpl, dl::NumericalTag>;
};

class UnchangedNumericalEffectImpl : public EffectBase<UnchangedNumericalEffectImpl, dl::NumericalTag>
{
private:
    UnchangedNumericalEffectImpl(Index index, NamedFeature<dl::NumericalTag> feature);

    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    bool evaluate_impl(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class EffectBase<UnchangedNumericalEffectImpl, dl::NumericalTag>;
};
}

#endif