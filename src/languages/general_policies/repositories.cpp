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

namespace mimir::languages::general_policies
{

template<dl::FeatureCategory D>
NamedFeature<D> Repositories::get_or_create_named_feature(std::string name, dl::Constructor<D> feature)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NamedFeatureImpl<D>> {}).get_or_create(name, feature);
}

template NamedFeature<dl::Boolean> Repositories::get_or_create_named_feature(std::string name, dl::Constructor<dl::Boolean> feature);
template NamedFeature<dl::Numerical> Repositories::get_or_create_named_feature(std::string name, dl::Constructor<dl::Numerical> feature);

PositiveBooleanCondition Repositories::get_or_create_positive_boolean_condition(NamedFeature<dl::Boolean> feature)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<PositiveBooleanConditionImpl> {}).get_or_create(feature);
}
NegativeBooleanCondition Repositories::get_or_create_negative_boolean_condition(NamedFeature<dl::Boolean> feature)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NegativeBooleanConditionImpl> {}).get_or_create(feature);
}
GreaterNumericalCondition Repositories::get_or_create_greater_numerical_condition(NamedFeature<dl::Numerical> feature)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GreaterNumericalConditionImpl> {}).get_or_create(feature);
}
EqualNumericalCondition Repositories::get_or_create_equal_numerical_condition(NamedFeature<dl::Numerical> feature)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<EqualNumericalConditionImpl> {}).get_or_create(feature);
}

PositiveBooleanEffect Repositories::get_or_create_positive_boolean_effect(NamedFeature<dl::Boolean> feature)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<PositiveBooleanEffectImpl> {}).get_or_create(feature);
}
NegativeBooleanEffect Repositories::get_or_create_negative_boolean_effect(NamedFeature<dl::Boolean> feature)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NegativeBooleanEffectImpl> {}).get_or_create(feature);
}
UnchangedBooleanEffect Repositories::get_or_create_unchanged_boolean_effect(NamedFeature<dl::Boolean> feature)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<UnchangedBooleanEffectImpl> {}).get_or_create(feature);
}
IncreaseNumericalEffect Repositories::get_or_create_increase_numerical_effect(NamedFeature<dl::Numerical> feature)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<IncreaseNumericalEffectImpl> {}).get_or_create(feature);
}
DecreaseNumericalEffect Repositories::get_or_create_decrease_numerical_effect(NamedFeature<dl::Numerical> feature)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<DecreaseNumericalEffectImpl> {}).get_or_create(feature);
}
UnchangedNumericalEffect Repositories::get_or_create_unchanged_numerical_effect(NamedFeature<dl::Numerical> feature)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<UnchangedNumericalEffectImpl> {}).get_or_create(feature);
}

Rule Repositories::get_or_create_rule(ConditionList conditions, EffectList effects)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RuleImpl> {}).get_or_create(std::move(conditions), std::move(effects));
}

}
