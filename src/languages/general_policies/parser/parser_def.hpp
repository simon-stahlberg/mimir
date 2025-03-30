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

#ifndef SRC_LANGUAGES_GENERAL_POLICIES_PARSER_PARSER_DEF_HPP_
#define SRC_LANGUAGES_GENERAL_POLICIES_PARSER_PARSER_DEF_HPP_

#include "../../description_logics/sentence_parser/parser.hpp"
#include "ast.hpp"
#include "ast_adapted.hpp"
#include "error_handler.hpp"
#include "mimir/languages/general_policies/keywords.hpp"
#include "parser.hpp"

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>

namespace mimir::languages::general_policies::parser
{

namespace x3 = boost::spirit::x3;
namespace ascii = boost::spirit::x3::ascii;

using x3::double_;
using x3::eps;
using x3::int_;
using x3::lexeme;
using x3::lit;
using x3::no_skip;
using x3::omit;
using x3::raw;
using x3::string;

using ascii::alnum;
using ascii::alpha;
using ascii::char_;
using ascii::space;

///////////////////////////////////////////////////////////////////////////
// Rule IDs
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Rules
///////////////////////////////////////////////////////////////////////////

named_concept_type const named_concept = "named_concept";
named_role_type const named_role = "named_role";
named_boolean_type const named_boolean = "named_boolean";
named_numerical_type const named_numerical = "named_numerical";

condition_type const condition = "condition";
positive_boolean_condition_type const positive_boolean_condition = "positive_boolean_condition";
negative_boolean_condition_type const negative_boolean_condition = "negative_boolean_condition";
greater_numerical_condition_type const greater_numerical_condition = "greater_numerical_condition";
equal_numerical_condition_type const equal_numerical_condition = "equal_numerical_condition";

effect_type const effect = "effect_type";
positive_boolean_effect_type const positive_boolean_effect = "positive_boolean_effect";
negative_boolean_effect_type const negative_boolean_effect = "negative_boolean_effect";
unchanged_boolean_effect_type const unchanged_boolean_effect = "unchanged_boolean_effect";
increase_numerical_effect_type const increase_numerical_effect = "increase_numerical_effect";
decrease_numerical_effect_type const decrease_numerical_effect = "decrease_numerical_effect";
unchanged_numerical_effect_type const unchanged_numerical_effect = "unchanged_numerical_effect";

rule_type const rule = "rule";

general_policy_type const general_policy = "general_policy";

///////////////////////////////////////////////////////////////////////////
// Grammar
///////////////////////////////////////////////////////////////////////////

inline auto name_parser() { return lexeme[omit[lit('<')]] > raw[lexeme[alpha >> *(alnum | char_('-') | char_('_'))]] > lexeme[omit[lit('>')]]; }

const auto named_concept_def = name_parser() > lit("::=") > dl::sentence_parser::concept_parser();
const auto named_role_def = name_parser() > lit("::=") > dl::sentence_parser::role_parser();
const auto named_boolean_def = name_parser() > lit("::=") > dl::sentence_parser::boolean_parser();
const auto named_numerical_def = name_parser() > lit("::=") > dl::sentence_parser::numerical_parser();

const auto condition_def = positive_boolean_condition | negative_boolean_condition | greater_numerical_condition | equal_numerical_condition;

const auto positive_boolean_condition_def = lit(std::string("@") + keywords::positive_boolean_condition) > name_parser();
const auto negative_boolean_condition_def = lit(std::string("@") + keywords::negative_boolean_condition) > name_parser();
const auto greater_numerical_condition_def = lit(std::string("@") + keywords::greater_numerical_condition) > name_parser();
const auto equal_numerical_condition_def = lit(std::string("@") + keywords::equal_numerical_condition) > name_parser();

const auto effect_def = positive_boolean_effect | negative_boolean_effect | unchanged_boolean_effect | increase_numerical_effect | decrease_numerical_effect
                        | unchanged_numerical_effect;

const auto positive_boolean_effect_def = lit(std::string("@") + keywords::positive_boolean_effect) > name_parser();
const auto negative_boolean_effect_def = lit(std::string("@") + keywords::negative_boolean_effect) > name_parser();
const auto unchanged_boolean_effect_def = lit(std::string("@") + keywords::unchanged_boolean_effect) > name_parser();
const auto increase_numerical_effect_def = lit(std::string("@") + keywords::increase_numerical_effect) > name_parser();
const auto decrease_numerical_effect_def = lit(std::string("@") + keywords::decrease_numerical_effect) > name_parser();
const auto unchanged_numerical_effect_def = lit(std::string("@") + keywords::unchanged_numerical_effect) > name_parser();

const auto rule_def = lit("{") > -(condition % lit(",")) > lit("}") > "->" > lit("{") > -(effect % lit(",")) > lit("}");

const auto general_policy_def = lit("[boolean_features]")      //
                                > *named_boolean               //
                                > lit("[numerical_features]")  //
                                > *named_numerical             //
                                > lit("[policy_rules]")        //
                                > *rule;

BOOST_SPIRIT_DEFINE(named_concept, named_role, named_boolean, named_numerical)

BOOST_SPIRIT_DEFINE(condition, positive_boolean_condition, negative_boolean_condition, greater_numerical_condition, equal_numerical_condition)

BOOST_SPIRIT_DEFINE(effect,
                    positive_boolean_effect,
                    negative_boolean_effect,
                    unchanged_boolean_effect,
                    increase_numerical_effect,
                    decrease_numerical_effect,
                    unchanged_numerical_effect)

BOOST_SPIRIT_DEFINE(rule)

BOOST_SPIRIT_DEFINE(general_policy)

///////////////////////////////////////////////////////////////////////////
// Annotation and Error handling
///////////////////////////////////////////////////////////////////////////

template<>
struct NamedFeatureClass<dl::ConceptTag> : x3::annotate_on_success
{
};
template<>
struct NamedFeatureClass<dl::RoleTag> : x3::annotate_on_success
{
};
template<>
struct NamedFeatureClass<dl::BooleanTag> : x3::annotate_on_success
{
};
template<>
struct NamedFeatureClass<dl::NumericalTag> : x3::annotate_on_success
{
};

struct ConditionClass : x3::annotate_on_success
{
};
struct PositiveBooleanConditionClass : x3::annotate_on_success
{
};
struct NegativeBooleanConditionClass : x3::annotate_on_success
{
};
struct GreaterNumericalConditionClass : x3::annotate_on_success
{
};
struct EqualNumericalConditionClass : x3::annotate_on_success
{
};

struct EffectClass : x3::annotate_on_success
{
};
struct PositiveBooleanEffectClass : x3::annotate_on_success
{
};
struct NegativeBooleanEffectClass : x3::annotate_on_success
{
};
struct UnchangedBooleanEffectClass : x3::annotate_on_success
{
};
struct IncreaseNumericalEffectClass : x3::annotate_on_success
{
};
struct DecreaseNumericalEffectClass : x3::annotate_on_success
{
};
struct UnchangedNumericalEffectClass : x3::annotate_on_success
{
};

struct RuleClass : x3::annotate_on_success
{
};
struct GeneralPolicyClass : x3::annotate_on_success, error_handler_base
{
};

}

namespace mimir::languages::general_policies
{
parser::named_concept_type const& named_concept() { return parser::named_concept; }
parser::named_role_type const& named_role() { return parser::named_role; }
parser::named_boolean_type const& named_boolean() { return parser::named_boolean; }
parser::named_numerical_type const& named_numerical() { return parser::named_numerical; }

parser::condition_type const& condition() { return parser::condition; }
parser::positive_boolean_condition_type const& positive_boolean_condition() { return parser::positive_boolean_condition; }
parser::negative_boolean_condition_type const& negative_boolean_condition() { return parser::negative_boolean_condition; }
parser::greater_numerical_condition_type const& greater_numerical_condition() { return parser::greater_numerical_condition; }
parser::equal_numerical_condition_type const& equal_numerical_condition() { return parser::equal_numerical_condition; }

parser::effect_type const& effect() { return parser::effect; }
parser::positive_boolean_effect_type const& positive_boolean_effect() { return parser::positive_boolean_effect; }
parser::negative_boolean_effect_type const& negative_boolean_effect() { return parser::negative_boolean_effect; }
parser::unchanged_boolean_effect_type const& unchanged_boolean_effect() { return parser::unchanged_boolean_effect; }
parser::increase_numerical_effect_type const& increase_numerical_effect() { return parser::increase_numerical_effect; }
parser::decrease_numerical_effect_type const& decrease_numerical_effect() { return parser::decrease_numerical_effect; }
parser::unchanged_numerical_effect_type const& unchanged_numerical_effect() { return parser::unchanged_numerical_effect; }

parser::rule_type const& rule() { return parser::rule; }

parser::general_policy_type const& general_policy() { return parser::general_policy; }
}

#endif
