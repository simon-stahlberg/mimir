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

#ifndef MIMIR_LANGUAGES_GENERAL_POLICIES_CONDITIONS_HPP_
#define MIMIR_LANGUAGES_GENERAL_POLICIES_CONDITIONS_HPP_

#include "mimir/languages/general_policies/condition_base.hpp"
#include "mimir/languages/general_policies/declarations.hpp"

namespace mimir::languages::general_policies
{

class PositiveBooleanConditionImpl : public ConditionBase<PositiveBooleanConditionImpl, dl::BooleanTag>
{
private:
    PositiveBooleanConditionImpl(Index index, NamedFeature<dl::BooleanTag> feature);

    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    bool evaluate_impl(dl::EvaluationContext& source_context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConditionBase<PositiveBooleanConditionImpl, dl::BooleanTag>;
};

class NegativeBooleanConditionImpl : public ConditionBase<NegativeBooleanConditionImpl, dl::BooleanTag>
{
private:
    NegativeBooleanConditionImpl(Index index, NamedFeature<dl::BooleanTag> feature);

    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    bool evaluate_impl(dl::EvaluationContext& source_context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConditionBase<NegativeBooleanConditionImpl, dl::BooleanTag>;
};

class GreaterNumericalConditionImpl : public ConditionBase<GreaterNumericalConditionImpl, dl::NumericalTag>
{
private:
    GreaterNumericalConditionImpl(Index index, NamedFeature<dl::NumericalTag> feature);

    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    bool evaluate_impl(dl::EvaluationContext& source_context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConditionBase<GreaterNumericalConditionImpl, dl::NumericalTag>;
};

class EqualNumericalConditionImpl : public ConditionBase<EqualNumericalConditionImpl, dl::NumericalTag>
{
private:
    EqualNumericalConditionImpl(Index index, NamedFeature<dl::NumericalTag> feature);

    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    bool evaluate_impl(dl::EvaluationContext& source_context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConditionBase<EqualNumericalConditionImpl, dl::NumericalTag>;
};
}

#endif