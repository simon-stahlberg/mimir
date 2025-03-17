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

class PositiveBooleanConditionImpl : public ConditionBase<PositiveBooleanConditionImpl, dl::Boolean>
{
private:
    explicit PositiveBooleanConditionImpl(NamedFeature<dl::Boolean> feature);

    bool evaluate_impl(dl::EvaluationContext& source_context) const;

    friend class ConditionBase<PositiveBooleanConditionImpl, dl::Boolean>;
};

class NegativeBooleanConditionImpl : public ConditionBase<NegativeBooleanConditionImpl, dl::Boolean>
{
private:
    explicit NegativeBooleanConditionImpl(NamedFeature<dl::Boolean> feature);

    bool evaluate_impl(dl::EvaluationContext& source_context) const;

    friend class ConditionBase<NegativeBooleanConditionImpl, dl::Boolean>;
};

class GreaterNumericalConditionImpl : public ConditionBase<GreaterNumericalConditionImpl, dl::Numerical>
{
private:
    explicit GreaterNumericalConditionImpl(NamedFeature<dl::Numerical> feature);

    bool evaluate_impl(dl::EvaluationContext& source_context) const;

    friend class ConditionBase<GreaterNumericalConditionImpl, dl::Numerical>;
};

class EqualNumericalConditionImpl : public ConditionBase<EqualNumericalConditionImpl, dl::Numerical>
{
private:
    explicit EqualNumericalConditionImpl(NamedFeature<dl::Numerical> feature);

    bool evaluate_impl(dl::EvaluationContext& source_context) const;

    friend class ConditionBase<EqualNumericalConditionImpl, dl::Numerical>;
};
}

#endif