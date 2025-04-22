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

#include "mimir/languages/general_policies/repositories.hpp"

#include "parser.hpp"

namespace mimir::languages::general_policies
{

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
NamedFeature<D> Repositories::get_or_create_named_feature(std::string name, dl::Constructor<D> feature)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NamedFeatureImpl<D>> {}).get_or_create(name, feature);
}

template NamedFeature<dl::ConceptTag> Repositories::get_or_create_named_feature(std::string name, dl::Constructor<dl::ConceptTag> feature);
template NamedFeature<dl::RoleTag> Repositories::get_or_create_named_feature(std::string name, dl::Constructor<dl::RoleTag> feature);
template NamedFeature<dl::BooleanTag> Repositories::get_or_create_named_feature(std::string name, dl::Constructor<dl::BooleanTag> feature);
template NamedFeature<dl::NumericalTag> Repositories::get_or_create_named_feature(std::string name, dl::Constructor<dl::NumericalTag> feature);

Condition Repositories::get_or_create_positive_boolean_condition(NamedFeature<dl::BooleanTag> feature)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<PositiveBooleanConditionImpl> {}).get_or_create(feature);
}
Condition Repositories::get_or_create_negative_boolean_condition(NamedFeature<dl::BooleanTag> feature)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NegativeBooleanConditionImpl> {}).get_or_create(feature);
}
Condition Repositories::get_or_create_greater_numerical_condition(NamedFeature<dl::NumericalTag> feature)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GreaterNumericalConditionImpl> {}).get_or_create(feature);
}
Condition Repositories::get_or_create_equal_numerical_condition(NamedFeature<dl::NumericalTag> feature)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<EqualNumericalConditionImpl> {}).get_or_create(feature);
}

Effect Repositories::get_or_create_positive_boolean_effect(NamedFeature<dl::BooleanTag> feature)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<PositiveBooleanEffectImpl> {}).get_or_create(feature);
}
Effect Repositories::get_or_create_negative_boolean_effect(NamedFeature<dl::BooleanTag> feature)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NegativeBooleanEffectImpl> {}).get_or_create(feature);
}
Effect Repositories::get_or_create_unchanged_boolean_effect(NamedFeature<dl::BooleanTag> feature)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<UnchangedBooleanEffectImpl> {}).get_or_create(feature);
}
Effect Repositories::get_or_create_increase_numerical_effect(NamedFeature<dl::NumericalTag> feature)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<IncreaseNumericalEffectImpl> {}).get_or_create(feature);
}
Effect Repositories::get_or_create_decrease_numerical_effect(NamedFeature<dl::NumericalTag> feature)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<DecreaseNumericalEffectImpl> {}).get_or_create(feature);
}
Effect Repositories::get_or_create_unchanged_numerical_effect(NamedFeature<dl::NumericalTag> feature)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<UnchangedNumericalEffectImpl> {}).get_or_create(feature);
}

Rule Repositories::get_or_create_rule(ConditionList conditions, EffectList effects)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RuleImpl> {}).get_or_create(std::move(conditions), std::move(effects));
}

GeneralPolicy Repositories::get_or_create_general_policy(NamedFeatureLists<dl::BooleanTag, dl::NumericalTag> features, RuleList rules)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GeneralPolicyImpl> {}).get_or_create(std::move(features), std::move(rules));
}

GeneralPolicy Repositories::get_or_create_general_policy(const std::string& description, const formalism::DomainImpl& domain, dl::Repositories& dl_repositories)
{
    return parse(description, domain, *this, dl_repositories);
}

}
