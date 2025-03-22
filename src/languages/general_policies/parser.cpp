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

#include "parser.hpp"

#include "../../common/x3/parser_wrapper.hpp"
#include "../description_logics/sentence_parser.hpp"
#include "mimir/languages/description_logics/declarations.hpp"
#include "mimir/languages/general_policies/declarations.hpp"
#include "mimir/languages/general_policies/repositories.hpp"
#include "parser/ast.hpp"
#include "parser/parser.hpp"

namespace mimir::languages::general_policies
{

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
static NamedFeature<D>
parse(const ast::NamedFeature<D>& node, const formalism::DomainImpl& domain, Repositories& repositories, dl::Repositories& dl_repositories)
{
    return repositories.get_or_create_named_feature(node.name, dl::parse(node.feature, domain, dl_repositories));
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
static void verify_feature_exists(const std::string& feature_name, const ToNamedFeatureMaps<std::string, dl::BooleanTag, dl::NumericalTag>& feature_map)
{
    if (!boost::hana::at_key(feature_map, boost::hana::type<D> {}).contains(feature_name))
    {
        throw std::runtime_error("The feature with name " + feature_name + " was never defined in the header of the general policy!");
    }
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
static NamedFeature<D> get(const std::string& feature_name, const ToNamedFeatureMaps<std::string, dl::BooleanTag, dl::NumericalTag>& feature_map)
{
    return boost::hana::at_key(feature_map, boost::hana::type<D> {}).at(feature_name);
}

static Condition
parse(const ast::PositiveBooleanCondition& node, Repositories& repositories, ToNamedFeatureMaps<std::string, dl::BooleanTag, dl::NumericalTag>& feature_map)
{
    verify_feature_exists<dl::BooleanTag>(node.feature_name, feature_map);
    return repositories.get_or_create_positive_boolean_condition(get<dl::BooleanTag>(node.feature_name, feature_map));
}

static Condition
parse(const ast::NegativeBooleanCondition& node, Repositories& repositories, ToNamedFeatureMaps<std::string, dl::BooleanTag, dl::NumericalTag>& feature_map)
{
    verify_feature_exists<dl::BooleanTag>(node.feature_name, feature_map);
    return repositories.get_or_create_negative_boolean_condition(get<dl::BooleanTag>(node.feature_name, feature_map));
}

static Condition
parse(const ast::GreaterNumericalCondition& node, Repositories& repositories, ToNamedFeatureMaps<std::string, dl::BooleanTag, dl::NumericalTag>& feature_map)
{
    verify_feature_exists<dl::NumericalTag>(node.feature_name, feature_map);
    return repositories.get_or_create_greater_numerical_condition(get<dl::NumericalTag>(node.feature_name, feature_map));
}

static Condition
parse(const ast::EqualNumericalCondition& node, Repositories& repositories, ToNamedFeatureMaps<std::string, dl::BooleanTag, dl::NumericalTag>& feature_map)
{
    verify_feature_exists<dl::NumericalTag>(node.feature_name, feature_map);
    return repositories.get_or_create_equal_numerical_condition(get<dl::NumericalTag>(node.feature_name, feature_map));
}

static Condition parse(const ast::Condition& node, Repositories& repositories, ToNamedFeatureMaps<std::string, dl::BooleanTag, dl::NumericalTag>& feature_map)
{
    return boost::apply_visitor([&](auto&& arg) { return parse(arg, repositories, feature_map); }, node);
}

static Effect
parse(const ast::PositiveBooleanEffect& node, Repositories& repositories, ToNamedFeatureMaps<std::string, dl::BooleanTag, dl::NumericalTag>& feature_map)
{
    verify_feature_exists<dl::BooleanTag>(node.feature_name, feature_map);
    return repositories.get_or_create_positive_boolean_effect(get<dl::BooleanTag>(node.feature_name, feature_map));
}

static Effect
parse(const ast::NegativeBooleanEffect& node, Repositories& repositories, ToNamedFeatureMaps<std::string, dl::BooleanTag, dl::NumericalTag>& feature_map)
{
    verify_feature_exists<dl::BooleanTag>(node.feature_name, feature_map);
    return repositories.get_or_create_negative_boolean_effect(get<dl::BooleanTag>(node.feature_name, feature_map));
}

static Effect
parse(const ast::UnchangedBooleanEffect& node, Repositories& repositories, ToNamedFeatureMaps<std::string, dl::BooleanTag, dl::NumericalTag>& feature_map)
{
    verify_feature_exists<dl::BooleanTag>(node.feature_name, feature_map);
    return repositories.get_or_create_unchanged_boolean_effect(get<dl::BooleanTag>(node.feature_name, feature_map));
}

static Effect
parse(const ast::IncreaseNumericalEffect& node, Repositories& repositories, ToNamedFeatureMaps<std::string, dl::BooleanTag, dl::NumericalTag>& feature_map)
{
    verify_feature_exists<dl::NumericalTag>(node.feature_name, feature_map);
    return repositories.get_or_create_increase_numerical_effect(get<dl::NumericalTag>(node.feature_name, feature_map));
}

static Effect
parse(const ast::DecreaseNumericalEffect& node, Repositories& repositories, ToNamedFeatureMaps<std::string, dl::BooleanTag, dl::NumericalTag>& feature_map)
{
    verify_feature_exists<dl::NumericalTag>(node.feature_name, feature_map);
    return repositories.get_or_create_decrease_numerical_effect(get<dl::NumericalTag>(node.feature_name, feature_map));
}

static Effect
parse(const ast::UnchangedNumericalEffect& node, Repositories& repositories, ToNamedFeatureMaps<std::string, dl::BooleanTag, dl::NumericalTag>& feature_map)
{
    verify_feature_exists<dl::NumericalTag>(node.feature_name, feature_map);
    return repositories.get_or_create_unchanged_numerical_effect(get<dl::NumericalTag>(node.feature_name, feature_map));
}

static Effect parse(const ast::Effect& node, Repositories& repositories, ToNamedFeatureMaps<std::string, dl::BooleanTag, dl::NumericalTag>& feature_map)
{
    return boost::apply_visitor([&](auto&& arg) { return parse(arg, repositories, feature_map); }, node);
}

static Rule parse(const ast::Rule& node, Repositories& repositories, ToNamedFeatureMaps<std::string, dl::BooleanTag, dl::NumericalTag>& feature_map)
{
    auto conditions = ConditionList {};
    for (const auto& condition_node : node.conditions)
    {
        conditions.push_back(parse(condition_node, repositories, feature_map));
    }

    auto effects = EffectList {};
    for (const auto& effect_node : node.effects)
    {
        effects.push_back(parse(effect_node, repositories, feature_map));
    }

    return repositories.get_or_create_rule(std::move(conditions), std::move(effects));
}

GeneralPolicy parse(const ast::GeneralPolicy& node, const formalism::DomainImpl& domain, Repositories& repositories, dl::Repositories& dl_repositories)
{
    auto features = NamedFeatureLists<dl::BooleanTag, dl::NumericalTag> {};
    auto feature_map = ToNamedFeatureMaps<std::string, dl::BooleanTag, dl::NumericalTag> {};

    auto& named_booleans_list = boost::hana::at_key(features, boost::hana::type<dl::BooleanTag> {});
    auto& named_booleans_map = boost::hana::at_key(feature_map, boost::hana::type<dl::BooleanTag> {});
    for (const auto& boolean_node : node.boolean_features)
    {
        const auto named_boolean = parse(boolean_node, domain, repositories, dl_repositories);
        named_booleans_list.push_back(named_boolean);
        named_booleans_map.emplace(named_boolean->get_name(), named_boolean);
    }

    auto& named_numericals_list = boost::hana::at_key(features, boost::hana::type<dl::NumericalTag> {});
    auto& named_numericals_map = boost::hana::at_key(feature_map, boost::hana::type<dl::NumericalTag> {});
    for (const auto& numerical_node : node.numerical_features)
    {
        const auto named_numerical = parse(numerical_node, domain, repositories, dl_repositories);
        named_numericals_list.push_back(named_numerical);
        named_numericals_map.emplace(named_numerical->get_name(), named_numerical);
    }

    auto rules = RuleList {};
    for (const auto& rule_node : node.rules)
    {
        rules.push_back(parse(rule_node, repositories, feature_map));
    }

    return repositories.get_or_create_general_policy(std::move(features), std::move(rules));
}

GeneralPolicy parse(const std::string& description, const formalism::DomainImpl& domain, Repositories& repositories, dl::Repositories& dl_repositories)
{
    auto ast = ast::GeneralPolicy();
    mimir::x3::parse_ast(description, general_policy(), ast);

    return parse(ast, domain, repositories, dl_repositories);
}

}
