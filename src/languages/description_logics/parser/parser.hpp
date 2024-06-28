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

#ifndef SRC_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTORS_PARSER_PARSER_HPP_
#define SRC_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTORS_PARSER_PARSER_HPP_

#include <boost/spirit/home/x3.hpp>
#include <mimir/languages/description_logics/parser/ast.hpp>

namespace mimir::dl
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
struct ConceptClass;
struct ConceptPredicateStateClass;
struct ConceptPredicateGoalClass;
struct ConceptAndClass;
struct ConceptNonTerminalClass;
struct ConceptChoiceClass;
struct ConceptDerivationRuleClass;

struct RoleClass;
struct RolePredicateStateClass;
struct RolePredicateGoalClass;
struct RoleAndClass;
struct RoleNonTerminalClass;
struct RoleChoiceClass;
struct RoleDerivationRuleClass;

struct DerivationRuleClass;

typedef x3::rule<ConceptClass, ast::Concept> concept_type;
typedef x3::rule<ConceptPredicateStateClass, ast::ConceptPredicateState> concept_predicate_state_type;
typedef x3::rule<ConceptPredicateGoalClass, ast::ConceptPredicateGoal> concept_predicate_goal_type;
typedef x3::rule<ConceptAndClass, ast::ConceptAnd> concept_and_type;
typedef x3::rule<ConceptNonTerminalClass, ast::ConceptNonTerminal> concept_non_terminal_type;
typedef x3::rule<ConceptChoiceClass, ast::ConceptChoice> concept_choice_type;
typedef x3::rule<ConceptDerivationRuleClass, ast::ConceptDerivationRule> concept_derivation_rule_type;

typedef x3::rule<RoleClass, ast::Role> role_type;
typedef x3::rule<RolePredicateStateClass, ast::RolePredicateState> role_predicate_state_type;
typedef x3::rule<RolePredicateGoalClass, ast::RolePredicateGoal> role_predicate_goal_type;
typedef x3::rule<RoleAndClass, ast::RoleAnd> role_and_type;
typedef x3::rule<RoleNonTerminalClass, ast::RoleNonTerminal> role_non_terminal_type;
typedef x3::rule<RoleChoiceClass, ast::RoleChoice> role_choice_type;
typedef x3::rule<RoleDerivationRuleClass, ast::RoleDerivationRule> role_derivation_rule_type;

typedef x3::rule<DerivationRuleClass, ast::DerivationRule> derivation_rule_type;

BOOST_SPIRIT_DECLARE(concept_type,
                     concept_predicate_state_type,
                     concept_predicate_goal_type,
                     concept_and_type,
                     concept_non_terminal_type,
                     concept_choice_type,
                     concept_derivation_rule_type)

BOOST_SPIRIT_DECLARE(role_type,
                     role_predicate_state_type,
                     role_predicate_goal_type,
                     role_and_type,
                     role_non_terminal_type,
                     role_choice_type,
                     role_derivation_rule_type)

BOOST_SPIRIT_DECLARE(derivation_rule_type)

}

parser::concept_type const& concept_();
parser::concept_predicate_state_type const& concept_predicate_state();
parser::concept_predicate_goal_type const& concept_predicate_goal();
parser::concept_and_type const& concept_and();
parser::concept_non_terminal_type const& concept_non_terminal();
parser::concept_choice_type const& concept_choice();
parser::concept_derivation_rule_type const& concept_derivation_rule();

parser::role_type const& role();
parser::role_predicate_state_type const& role_predicate_state();
parser::role_predicate_goal_type const& role_predicate_goal();
parser::role_and_type const& role_and();
parser::role_non_terminal_type const& role_non_terminal();
parser::role_choice_type const& role_choice();
parser::role_derivation_rule_type const& role_derivation_rule();

parser::derivation_rule_type const& derivation_rule();
}

#endif
