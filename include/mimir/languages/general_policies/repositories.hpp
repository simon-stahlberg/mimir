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

#ifndef MIMIR_LANGUAGES_GENERAL_POLICIES_REPOSITORIES_HPP_
#define MIMIR_LANGUAGES_GENERAL_POLICIES_REPOSITORIES_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/languages/general_policies/conditions.hpp"
#include "mimir/languages/general_policies/effects.hpp"
#include "mimir/languages/general_policies/general_policy.hpp"
#include "mimir/languages/general_policies/named_feature.hpp"
#include "mimir/languages/general_policies/rule.hpp"

#include <boost/hana.hpp>
#include <loki/loki.hpp>
#include <ranges>

namespace mimir::languages::general_policies
{

template<typename T>
using SegmentedPDDLRepository = loki::SegmentedRepository<T>;

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
using NamedFeatureRepository = SegmentedPDDLRepository<NamedFeatureImpl<D>>;

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
using NamedFeatureRepository = SegmentedPDDLRepository<NamedFeatureImpl<D>>;

using PositiveBooleanConditionRepository = SegmentedPDDLRepository<PositiveBooleanConditionImpl>;
using NegativeBooleanConditionRepository = SegmentedPDDLRepository<NegativeBooleanConditionImpl>;
using GreaterNumericalConditionRepository = SegmentedPDDLRepository<GreaterNumericalConditionImpl>;
using EqualNumericalConditionRepository = SegmentedPDDLRepository<EqualNumericalConditionImpl>;

using PositiveBooleanEffectRepository = SegmentedPDDLRepository<PositiveBooleanEffectImpl>;
using NegativeBooleanEffectRepository = SegmentedPDDLRepository<NegativeBooleanEffectImpl>;
using UnchangedBooleanEffectRepository = SegmentedPDDLRepository<UnchangedBooleanEffectImpl>;
using IncreaseNumericalEffectRepository = SegmentedPDDLRepository<IncreaseNumericalEffectImpl>;
using DecreaseNumericalEffectRepository = SegmentedPDDLRepository<DecreaseNumericalEffectImpl>;
using UnchangedNumericalEffectRepository = SegmentedPDDLRepository<UnchangedNumericalEffectImpl>;

using RuleRepository = SegmentedPDDLRepository<RuleImpl>;

using GeneralPolicyRepository = SegmentedPDDLRepository<GeneralPolicyImpl>;

using HanaRepositories = boost::hana::map<boost::hana::pair<boost::hana::type<NamedFeatureImpl<dl::ConceptTag>>, NamedFeatureRepository<dl::ConceptTag>>,
                                          boost::hana::pair<boost::hana::type<NamedFeatureImpl<dl::RoleTag>>, NamedFeatureRepository<dl::RoleTag>>,
                                          boost::hana::pair<boost::hana::type<NamedFeatureImpl<dl::BooleanTag>>, NamedFeatureRepository<dl::BooleanTag>>,
                                          boost::hana::pair<boost::hana::type<NamedFeatureImpl<dl::NumericalTag>>, NamedFeatureRepository<dl::NumericalTag>>,
                                          boost::hana::pair<boost::hana::type<PositiveBooleanConditionImpl>, PositiveBooleanConditionRepository>,
                                          boost::hana::pair<boost::hana::type<NegativeBooleanConditionImpl>, NegativeBooleanConditionRepository>,
                                          boost::hana::pair<boost::hana::type<GreaterNumericalConditionImpl>, GreaterNumericalConditionRepository>,
                                          boost::hana::pair<boost::hana::type<EqualNumericalConditionImpl>, EqualNumericalConditionRepository>,
                                          boost::hana::pair<boost::hana::type<PositiveBooleanEffectImpl>, PositiveBooleanEffectRepository>,
                                          boost::hana::pair<boost::hana::type<NegativeBooleanEffectImpl>, NegativeBooleanEffectRepository>,
                                          boost::hana::pair<boost::hana::type<UnchangedBooleanEffectImpl>, UnchangedBooleanEffectRepository>,
                                          boost::hana::pair<boost::hana::type<IncreaseNumericalEffectImpl>, IncreaseNumericalEffectRepository>,
                                          boost::hana::pair<boost::hana::type<DecreaseNumericalEffectImpl>, DecreaseNumericalEffectRepository>,
                                          boost::hana::pair<boost::hana::type<UnchangedNumericalEffectImpl>, UnchangedNumericalEffectRepository>,
                                          boost::hana::pair<boost::hana::type<RuleImpl>, RuleRepository>,
                                          boost::hana::pair<boost::hana::type<GeneralPolicyImpl>, GeneralPolicyRepository>>;

class Repositories
{
private:
    HanaRepositories m_repositories;

public:
    Repositories() = default;

    // delete copy and allow move
    Repositories(const Repositories& other) = delete;
    Repositories& operator=(const Repositories& other) = delete;
    Repositories(Repositories&& other) = default;
    Repositories& operator=(Repositories&& other) = default;

    template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
    NamedFeature<D> get_or_create_named_feature(std::string name, dl::Constructor<D> feature);

    Condition get_or_create_positive_boolean_condition(NamedFeature<dl::BooleanTag> feature);
    Condition get_or_create_negative_boolean_condition(NamedFeature<dl::BooleanTag> feature);
    Condition get_or_create_greater_numerical_condition(NamedFeature<dl::NumericalTag> feature);
    Condition get_or_create_equal_numerical_condition(NamedFeature<dl::NumericalTag> feature);

    Effect get_or_create_positive_boolean_effect(NamedFeature<dl::BooleanTag> feature);
    Effect get_or_create_negative_boolean_effect(NamedFeature<dl::BooleanTag> feature);
    Effect get_or_create_unchanged_boolean_effect(NamedFeature<dl::BooleanTag> feature);
    Effect get_or_create_increase_numerical_effect(NamedFeature<dl::NumericalTag> feature);
    Effect get_or_create_decrease_numerical_effect(NamedFeature<dl::NumericalTag> feature);
    Effect get_or_create_unchanged_numerical_effect(NamedFeature<dl::NumericalTag> feature);

    Rule get_or_create_rule(ConditionList conditions, EffectList effects);

    GeneralPolicy get_or_create_general_policy(NamedFeatureLists<dl::BooleanTag, dl::NumericalTag> features, RuleList rules);

    GeneralPolicy get_or_create_general_policy(const std::string& description, const formalism::DomainImpl& domain, dl::Repositories& dl_repositories);
};

}

#endif
