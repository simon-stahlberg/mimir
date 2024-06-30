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

#ifndef SRC_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTORS_PARSER_PARSER_DEF_HPP_
#define SRC_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTORS_PARSER_PARSER_DEF_HPP_

#include "ast_adapted.hpp"
#include "parser.hpp"

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>
#include <mimir/languages/description_logics/parser/ast.hpp>
#include <mimir/languages/description_logics/parser/error_handler.hpp>
#include <mimir/languages/description_logics/parser/parser.hpp>

namespace mimir::dl::parser
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

concept_type const concept_ = "concept";
concept_predicate_state_type const concept_predicate_state = "concept_predicate_state";
concept_predicate_goal_type const concept_predicate_goal = "concept_predicate_goal";
concept_and_type const concept_and = "concept_and";
concept_non_terminal_type const concept_non_terminal = "concept_non_terminal";
concept_choice_type const concept_choice = "concept_choice";
concept_derivation_rule_type const concept_derivation_rule = "concept_derivation_rule";

role_type const role = "role";
role_predicate_state_type const role_predicate_state = "role_predicate_state";
role_predicate_goal_type const role_predicate_goal = "role_predicate_goal";
role_and_type const role_and = "role_and";
role_non_terminal_type const role_non_terminal = "role_non_terminal";
role_choice_type const role_choice = "role_choice";
role_derivation_rule_type const role_derivation_rule = "role_derivation_rule";

derivation_rule_type const derivation_rule = "derivation_rule";
grammar_type const grammar = "grammar";

///////////////////////////////////////////////////////////////////////////
// Grammar
///////////////////////////////////////////////////////////////////////////

inline auto separator_parser() { return (ascii::space | x3::eol | x3::eoi); }
inline auto concept_non_terminal_parser() { return raw[lexeme["<concept" >> *(alnum | char_('-') | char_('_')) > ">"]]; }
inline auto role_non_terminal_parser() { return raw[lexeme["<role" >> *(alnum | char_('-') | char_('_')) >> ">"]]; }
inline auto predicate_name_parser() { return lexeme[omit[lit('"')]] > raw[lexeme[alpha >> *(alnum | char_('-') | char_('_'))]] > lexeme[omit[lit('"')]]; }

const auto concept__def = concept_non_terminal | concept_predicate_state | concept_predicate_goal | concept_and;
const auto concept_predicate_state_def = lit("@concept_predicate_state") > predicate_name_parser();
const auto concept_predicate_goal_def = lit("@concept_predicate_goal") > predicate_name_parser();
const auto concept_and_def = lit("@concept_and") > concept_ > concept_;
const auto concept_non_terminal_def = concept_non_terminal_parser();
const auto concept_choice_def = concept_non_terminal | concept_;
const auto concept_derivation_rule_def = concept_non_terminal > "::=" > (concept_choice % lit("|"));

const auto role_def = role_non_terminal | role_predicate_state | role_predicate_goal | role_and;
const auto role_predicate_state_def = lit("@role_predicate_state") > predicate_name_parser();
const auto role_predicate_goal_def = lit("@role_predicate_goal") > predicate_name_parser();
const auto role_and_def = lit("@role_and") > role > role;
const auto role_non_terminal_def = role_non_terminal_parser();
const auto role_choice_def = role_non_terminal | role;
const auto role_derivation_rule_def = role_non_terminal > "::=" > (role_choice % lit("|"));

const auto derivation_rule_def = (concept_derivation_rule | role_derivation_rule);
const auto grammar_def = *derivation_rule;

BOOST_SPIRIT_DEFINE(concept_, concept_predicate_state, concept_predicate_goal, concept_and, concept_non_terminal, concept_choice, concept_derivation_rule)

BOOST_SPIRIT_DEFINE(role, role_predicate_state, role_predicate_goal, role_and, role_non_terminal, role_choice, role_derivation_rule)

BOOST_SPIRIT_DEFINE(derivation_rule, grammar)

///////////////////////////////////////////////////////////////////////////
// Annotation and Error handling
///////////////////////////////////////////////////////////////////////////

struct ConceptClass : x3::annotate_on_success
{
};
struct ConceptPredicateStateClass : x3::annotate_on_success
{
};
struct ConceptPredicateGoalClass : x3::annotate_on_success
{
};
struct ConceptAndClass : x3::annotate_on_success
{
};
struct ConceptNonTerminalClass : x3::annotate_on_success
{
};
struct ConceptChoiceClass : x3::annotate_on_success
{
};
struct ConceptDerivationRuleClass : x3::annotate_on_success
{
};

struct RoleClass : x3::annotate_on_success
{
};
struct RolePredicateStateClass : x3::annotate_on_success
{
};
struct RolePredicateGoalClass : x3::annotate_on_success
{
};
struct RoleAndClass : x3::annotate_on_success
{
};
struct RoleNonTerminalClass : x3::annotate_on_success
{
};
struct RoleChoiceClass : x3::annotate_on_success
{
};
struct RoleDerivationRuleClass : x3::annotate_on_success
{
};

struct DerivationRuleClass : x3::annotate_on_success
{
};
struct GrammarClass : x3::annotate_on_success, error_handler_base
{
};
}

namespace mimir::dl
{
parser::concept_type const& concept_() { return parser::concept_; }
parser::concept_predicate_state_type const& concept_predicate_state() { return parser::concept_predicate_state; }
parser::concept_predicate_goal_type const& concept_predicate_goal() { return parser::concept_predicate_goal; }
parser::concept_and_type const& concept_and() { return parser::concept_and; }
parser::concept_non_terminal_type const& concept_non_terminal() { return parser::concept_non_terminal; }
parser::concept_choice_type const& concept_choice() { return parser::concept_choice; }
parser::concept_derivation_rule_type const& concept_derivation_rule() { return parser::concept_derivation_rule; }

parser::role_type const& role() { return parser::role; }
parser::role_predicate_state_type const& role_predicate_state() { return parser::role_predicate_state; }
parser::role_predicate_goal_type const& role_predicate_goal() { return parser::role_predicate_goal; }
parser::role_and_type const& role_and() { return parser::role_and; }
parser::role_non_terminal_type const& role_non_terminal() { return parser::role_non_terminal; }
parser::role_choice_type const& role_choice() { return parser::role_choice; }
parser::role_derivation_rule_type const& role_derivation_rule() { return parser::role_derivation_rule; }

parser::derivation_rule_type const& derivation_rule() { return parser::derivation_rule; }
parser::grammar_type const& grammar_parser() { return parser::grammar; }
}

#endif
