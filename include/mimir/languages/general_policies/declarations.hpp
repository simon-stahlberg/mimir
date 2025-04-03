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

#ifndef MIMIR_LANGUAGES_GENERAL_POLICIES_DECLARATIONS_HPP_
#define MIMIR_LANGUAGES_GENERAL_POLICIES_DECLARATIONS_HPP_

#include "mimir/common/types.hpp"
#include "mimir/languages/description_logics/declarations.hpp"

namespace mimir::languages::general_policies
{
class Repositories;

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
class NamedFeatureImpl;
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
using NamedFeature = const NamedFeatureImpl<D>*;
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
using NamedFeatureList = std::vector<NamedFeature<D>>;
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag... Ds>
using NamedFeatureLists = boost::hana::map<boost::hana::pair<boost::hana::type<Ds>, NamedFeatureList<Ds>>...>;
template<typename Key, dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
using ToNamedFeatureMap = std::unordered_map<Key, NamedFeature<D>, loki::Hash<Key>, loki::EqualTo<Key>>;
template<typename Key, dl::IsConceptOrRoleOrBooleanOrNumericalTag... Ds>
using ToNamedFeatureMaps = boost::hana::map<boost::hana::pair<boost::hana::type<Ds>, ToNamedFeatureMap<Key, Ds>>...>;

using NamedFeatureVariant = std::variant<NamedFeature<dl::BooleanTag>, NamedFeature<dl::NumericalTag>>;
using NamedFeatureVariantList = std::vector<NamedFeatureVariant>;

class ICondition;
using Condition = const ICondition*;
using ConditionList = std::vector<Condition>;
using ConditionSet = std::unordered_set<Condition>;

class PositiveBooleanConditionImpl;
using PositiveBooleanCondition = const PositiveBooleanConditionImpl*;
class NegativeBooleanConditionImpl;
using NegativeBooleanCondition = const NegativeBooleanConditionImpl*;
class GreaterNumericalConditionImpl;
using GreaterNumericalCondition = const GreaterNumericalConditionImpl*;
class EqualNumericalConditionImpl;
using EqualNumericalCondition = const EqualNumericalConditionImpl*;

class IEffect;
using Effect = const IEffect*;
using EffectList = std::vector<Effect>;
using EffectSet = std::unordered_set<Effect>;

class PositiveBooleanEffectImpl;
using PositiveBooleanEffect = const PositiveBooleanEffectImpl*;
class NegativeBooleanEffectImpl;
using NegativeBooleanEffect = const NegativeBooleanEffectImpl*;
class UnchangedBooleanEffectImpl;
using UnchangedBooleanEffect = const UnchangedBooleanEffectImpl*;
class IncreaseNumericalEffectImpl;
using IncreaseNumericalEffect = const IncreaseNumericalEffectImpl*;
class DecreaseNumericalEffectImpl;
using DecreaseNumericalEffect = const DecreaseNumericalEffectImpl*;
class UnchangedNumericalEffectImpl;
using UnchangedNumericalEffect = const UnchangedNumericalEffectImpl*;

class RuleImpl;
using Rule = const RuleImpl*;
using RuleList = std::vector<Rule>;

class GeneralPolicyImpl;
using GeneralPolicy = const GeneralPolicyImpl*;

class IVisitor;

}

#endif