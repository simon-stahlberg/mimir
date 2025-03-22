/*
 * Copyright (C) 2023 Dominik Drexler
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

#ifndef SRC_LANGUAGES_GENERAL_POLICIES_PARSER_PARSER_HPP_
#define SRC_LANGUAGES_GENERAL_POLICIES_PARSER_PARSER_HPP_

#include "../parser/ast.hpp"

#include <boost/spirit/home/x3.hpp>

namespace mimir::languages::general_policies
{
namespace x3 = boost::spirit::x3;
namespace ascii = boost::spirit::x3::ascii;

using x3::eoi;
using x3::eol;
using x3::lit;
using x3::no_skip;
using x3::string;

using ascii::char_;
using ascii::space;

///////////////////////////////////////////////////////////////////////////
// parser public interface for testing
///////////////////////////////////////////////////////////////////////////
namespace parser
{

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
struct NamedFeatureClass;

struct ConditionClass;
struct PositiveBooleanConditionClass;
struct NegativeBooleanConditionClass;
struct GreaterNumericalConditionClass;
struct EqualNumericalConditionClass;

struct EffectClass;
struct PositiveBooleanEffectClass;
struct NegativeBooleanEffectClass;
struct UnchangedBooleanEffectClass;
struct IncreaseNumericalEffectClass;
struct DecreaseNumericalEffectClass;
struct UnchangedNumericalEffectClass;

struct RuleClass;
struct GeneralPolicyClass;

typedef x3::rule<NamedFeatureClass<dl::ConceptTag>, ast::NamedFeature<dl::ConceptTag>> named_concept_type;
typedef x3::rule<NamedFeatureClass<dl::RoleTag>, ast::NamedFeature<dl::RoleTag>> named_role_type;
typedef x3::rule<NamedFeatureClass<dl::BooleanTag>, ast::NamedFeature<dl::BooleanTag>> named_boolean_type;
typedef x3::rule<NamedFeatureClass<dl::NumericalTag>, ast::NamedFeature<dl::NumericalTag>> named_numerical_type;

typedef x3::rule<ConditionClass, ast::Condition> condition_type;
typedef x3::rule<PositiveBooleanConditionClass, ast::PositiveBooleanCondition> positive_boolean_condition_type;
typedef x3::rule<NegativeBooleanConditionClass, ast::NegativeBooleanCondition> negative_boolean_condition_type;
typedef x3::rule<GreaterNumericalConditionClass, ast::GreaterNumericalCondition> greater_numerical_condition_type;
typedef x3::rule<EqualNumericalConditionClass, ast::EqualNumericalCondition> equal_numerical_condition_type;

typedef x3::rule<EffectClass, ast::Effect> effect_type;
typedef x3::rule<PositiveBooleanEffectClass, ast::PositiveBooleanEffect> positive_boolean_effect_type;
typedef x3::rule<NegativeBooleanEffectClass, ast::NegativeBooleanEffect> negative_boolean_effect_type;
typedef x3::rule<UnchangedBooleanEffectClass, ast::UnchangedBooleanEffect> unchanged_boolean_effect_type;
typedef x3::rule<IncreaseNumericalEffectClass, ast::IncreaseNumericalEffect> increase_numerical_effect_type;
typedef x3::rule<DecreaseNumericalEffectClass, ast::DecreaseNumericalEffect> decrease_numerical_effect_type;
typedef x3::rule<UnchangedNumericalEffectClass, ast::UnchangedNumericalEffect> unchanged_numerical_effect_type;

typedef x3::rule<RuleClass, ast::Rule> rule_type;

typedef x3::rule<GeneralPolicyClass, ast::GeneralPolicy> general_policy_type;

BOOST_SPIRIT_DECLARE(named_concept_type, named_role_type, named_boolean_type, named_numerical_type)

BOOST_SPIRIT_DECLARE(condition_type,
                     positive_boolean_condition_type,
                     negative_boolean_condition_type,
                     greater_numerical_condition_type,
                     equal_numerical_condition_type)

BOOST_SPIRIT_DECLARE(effect_type,
                     positive_boolean_effect_type,
                     negative_boolean_effect_type,
                     unchanged_boolean_effect_type,
                     increase_numerical_effect_type,
                     decrease_numerical_effect_type,
                     unchanged_numerical_effect_type)

BOOST_SPIRIT_DECLARE(rule_type)

BOOST_SPIRIT_DECLARE(general_policy_type)

}

/**
 * Grammar
 */

parser::named_concept_type const& named_concept();
parser::named_role_type const& named_role();
parser::named_boolean_type const& named_boolean();
parser::named_numerical_type const& named_numerical();

parser::condition_type const& condition();
parser::positive_boolean_condition_type const& positive_boolean_condition();
parser::negative_boolean_condition_type const& negative_boolean_condition();
parser::greater_numerical_condition_type const& greater_numerical_condition();
parser::equal_numerical_condition_type const& equal_numerical_condition();

parser::effect_type const& effect();
parser::positive_boolean_effect_type const& positive_boolean_effect();
parser::negative_boolean_effect_type const& negative_boolean_effect();
parser::unchanged_boolean_effect_type const& unchanged_boolean_effect();
parser::increase_numerical_effect_type const& increase_numerical_effect();
parser::decrease_numerical_effect_type const& decrease_numerical_effect();
parser::unchanged_numerical_effect_type const& unchanged_numerical_effect();

parser::rule_type const& rule();

parser::general_policy_type const& general_policy();
}

#endif
