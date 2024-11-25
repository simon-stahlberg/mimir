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
#include "mimir/languages/description_logics/constructor_keywords.hpp"
#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/description_logics/parser/ast.hpp"
#include "mimir/languages/description_logics/parser/error_handler.hpp"
#include "mimir/languages/description_logics/parser/parser.hpp"
#include "parser.hpp"

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>

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
concept_bot_type const concept_bot = "concept_bot";
concept_top_type const concept_top = "concept_top";
concept_atomic_state_type const concept_atomic_state = "concept_atomic_state";
concept_atomic_goal_type const concept_atomic_goal = "concept_atomic_goal";
concept_intersection_type const concept_intersection = "concept_intersection";
concept_union_type const concept_union = "concept_union";
concept_negation_type const concept_negation = "concept_negation";
concept_value_restriction_type const concept_value_restriction = "concept_value_restriction";
concept_existential_quantification_type const concept_existential_quantification = "concept_existential_quantification";
concept_role_value_map_containment_type const concept_role_value_map_containment = "concept_role_value_map_containment";
concept_role_value_map_equality_type const concept_role_value_map_equality = "concept_role_value_map_equality";
concept_nominal_type const concept_nominal = "concept_nominal";
concept_non_terminal_type const concept_non_terminal = "concept_non_terminal";
concept_choice_type const concept_choice = "concept_choice";
concept_derivation_rule_type const concept_derivation_rule = "concept_derivation_rule";

role_type const role = "role";
role_universal_type const role_universal = "role_universal";
role_atomic_state_type const role_atomic_state = "role_atomic_state";
role_atomic_goal_type const role_atomic_goal = "role_atomic_goal";
role_intersection_type const role_intersection = "role_intersection";
role_union_type const role_union = "role_union";
role_complement_type const role_complement = "role_complement";
role_inverse_type const role_inverse = "role_inverse";
role_composition_type const role_composition = "role_composition";
role_transitive_closure_type const role_transitive_closure = "role_transitive_closure";
role_reflexive_transitive_closure_type const role_reflexive_transitive_closure = "role_reflexive_transitive_closure";
role_restriction_type const role_restriction = "role_restriction";
role_identity_type const role_identity = "role_identity";
role_non_terminal_type const role_non_terminal = "role_non_terminal";
role_choice_type const role_choice = "role_choice";
role_derivation_rule_type const role_derivation_rule = "role_derivation_rule";

concept_or_role_derivation_rule_type const concept_or_role_derivation_rule = "concept_or_role_derivation_rule";
grammar_type const grammar = "grammar";

///////////////////////////////////////////////////////////////////////////
// Grammar
///////////////////////////////////////////////////////////////////////////

inline auto separator_parser() { return (ascii::space | x3::eol | x3::eoi); }
inline auto concept_non_terminal_parser() { return raw[lexeme["<concept" >> *(alnum | char_('-') | char_('_')) > ">"]]; }
inline auto role_non_terminal_parser() { return raw[lexeme["<role" >> *(alnum | char_('-') | char_('_')) >> ">"]]; }
inline auto predicate_name_parser() { return lexeme[omit[lit('"')]] > raw[lexeme[alpha >> *(alnum | char_('-') | char_('_'))]] > lexeme[omit[lit('"')]]; }
inline auto object_name_parser() { return lexeme[omit[lit('"')]] > raw[lexeme[alpha >> *(alnum | char_('-') | char_('_'))]] > lexeme[omit[lit('"')]]; }
inline auto bool_parser()
{
    return x3::lexeme[(x3::lit("true") >> x3::attr(true)) | (x3::lit("false") >> x3::attr(false)) | (x3::lit("1") >> x3::attr(true))
                      | (x3::lit("0") >> x3::attr(false))];
}

const auto concept__def = concept_bot | concept_top | concept_atomic_state | concept_atomic_goal | concept_intersection | concept_union | concept_negation
                          | concept_value_restriction | concept_existential_quantification | concept_role_value_map_containment
                          | concept_role_value_map_equality | concept_nominal;

const auto concept_bot_def = lit(keywords::concept_bot) >> x3::attr(ast::ConceptBot {});
const auto concept_top_def = lit(keywords::concept_top) >> x3::attr(ast::ConceptTop {});
const auto concept_atomic_state_def = lit(keywords::concept_atomic_state) > predicate_name_parser();
const auto concept_atomic_goal_def = lit(keywords::concept_atomic_goal) > predicate_name_parser() > bool_parser();
const auto concept_intersection_def = lit(keywords::concept_intersection) > concept_choice > concept_choice;
const auto concept_union_def = lit(keywords::concept_union) > concept_choice > concept_choice;
const auto concept_negation_def = lit(keywords::concept_negation) > concept_choice;
const auto concept_value_restriction_def = lit(keywords::concept_value_restriction) > role_choice > concept_choice;
const auto concept_existential_quantification_def = lit(keywords::concept_existential_quantification) > role_choice > concept_choice;
const auto concept_role_value_map_containment_def = lit(keywords::concept_role_value_map_containment) > role_choice > role_choice;
const auto concept_role_value_map_equality_def = lit(keywords::concept_role_value_map_equality) > role_choice > role_choice;
const auto concept_nominal_def = lit(keywords::concept_nominal) > object_name_parser();
const auto concept_non_terminal_def = concept_non_terminal_parser();
const auto concept_choice_def = concept_non_terminal | concept_;
const auto concept_derivation_rule_def = concept_non_terminal > "::=" > (concept_choice % lit("|"));

const auto role_def = role_universal | role_atomic_state | role_atomic_goal | role_intersection | role_union | role_complement | role_inverse | role_composition
                      | role_transitive_closure | role_reflexive_transitive_closure | role_restriction | role_identity;

const auto role_universal_def = lit(keywords::role_universal) >> x3::attr(ast::RoleUniversal {});
const auto role_atomic_state_def = lit(keywords::role_atomic_state) > predicate_name_parser();
const auto role_atomic_goal_def = lit(keywords::role_atomic_goal) > predicate_name_parser() > bool_parser();
const auto role_intersection_def = lit(keywords::role_intersection) > role_choice > role_choice;
const auto role_union_def = lit(keywords::role_union) > role_choice > role_choice;
const auto role_complement_def = lit(keywords::role_complement) > role_choice;
const auto role_inverse_def = lit(keywords::role_inverse) > role_choice;
const auto role_composition_def = lit(keywords::role_composition) > role_choice > role_choice;
const auto role_transitive_closure_def = lit(keywords::role_transitive_closure) > role_choice;
const auto role_reflexive_transitive_closure_def = lit(keywords::role_reflexive_transitive_closure) > role_choice;
const auto role_restriction_def = lit(keywords::role_restriction) > role_choice > concept_choice;
const auto role_identity_def = lit(keywords::role_identity) > concept_choice;
const auto role_non_terminal_def = role_non_terminal_parser();
const auto role_choice_def = role_non_terminal | role;
const auto role_derivation_rule_def = role_non_terminal > "::=" > (role_choice % lit("|"));

const auto concept_or_role_derivation_rule_def = (concept_derivation_rule | role_derivation_rule);
const auto grammar_def = *concept_or_role_derivation_rule;

BOOST_SPIRIT_DEFINE(concept_,
                    concept_bot,
                    concept_top,
                    concept_atomic_state,
                    concept_atomic_goal,
                    concept_intersection,
                    concept_union,
                    concept_negation,
                    concept_value_restriction,
                    concept_existential_quantification,
                    concept_role_value_map_containment,
                    concept_role_value_map_equality,
                    concept_nominal,
                    concept_non_terminal,
                    concept_choice,
                    concept_derivation_rule)

BOOST_SPIRIT_DEFINE(role,
                    role_universal,
                    role_atomic_state,
                    role_atomic_goal,
                    role_intersection,
                    role_union,
                    role_complement,
                    role_inverse,
                    role_composition,
                    role_transitive_closure,
                    role_reflexive_transitive_closure,
                    role_restriction,
                    role_identity,
                    role_non_terminal,
                    role_choice,
                    role_derivation_rule)

BOOST_SPIRIT_DEFINE(concept_or_role_derivation_rule, grammar)

///////////////////////////////////////////////////////////////////////////
// Annotation and Error handling
///////////////////////////////////////////////////////////////////////////

template<>
struct ConstructorClass<Concept> : x3::annotate_on_success
{
};
template<>
struct ConstructorClass<Role> : x3::annotate_on_success
{
};

template<>
struct NonTerminalClass<Concept> : x3::annotate_on_success
{
};
template<>
struct NonTerminalClass<Role> : x3::annotate_on_success
{
};
template<>
struct ConstructorOrNonTerminalClass<Concept> : x3::annotate_on_success
{
};
template<>
struct ConstructorOrNonTerminalClass<Role> : x3::annotate_on_success
{
};
template<>
struct DerivationRuleClass<Concept> : x3::annotate_on_success
{
};
template<>
struct DerivationRuleClass<Role> : x3::annotate_on_success
{
};

struct ConceptBotClass : x3::annotate_on_success
{
};
struct ConceptTopClass : x3::annotate_on_success
{
};
struct ConceptAtomicStateClass : x3::annotate_on_success
{
};
struct ConceptAtomicGoalClass : x3::annotate_on_success
{
};
struct ConceptIntersectionClass : x3::annotate_on_success
{
};
struct ConceptUnionClass : x3::annotate_on_success
{
};
struct ConceptNegationClass : x3::annotate_on_success
{
};
struct ConceptValueRestrictionClass : x3::annotate_on_success
{
};
struct ConceptExistentialQuantificationClass : x3::annotate_on_success
{
};
struct ConceptRoleValueMapContainmentClass : x3::annotate_on_success
{
};
struct ConceptRoleValueMapEqualityClass : x3::annotate_on_success
{
};
struct ConceptNominalClass : x3::annotate_on_success
{
};

struct RoleUniversalClass : x3::annotate_on_success
{
};
struct RoleAtomicStateClass : x3::annotate_on_success
{
};
struct RoleAtomicGoalClass : x3::annotate_on_success
{
};
struct RoleIntersectionClass : x3::annotate_on_success
{
};
struct RoleUnionClass : x3::annotate_on_success
{
};
struct RoleComplementClass : x3::annotate_on_success
{
};
struct RoleInverseClass : x3::annotate_on_success
{
};
struct RoleCompositionClass : x3::annotate_on_success
{
};
struct RoleTransitiveClosureClass : x3::annotate_on_success
{
};
struct RoleReflexiveTransitiveClosureClass : x3::annotate_on_success
{
};
struct RoleRestrictionClass : x3::annotate_on_success
{
};
struct RoleIdentityClass : x3::annotate_on_success
{
};

struct ConceptOrRoleDerivationRuleClass : x3::annotate_on_success
{
};
struct GrammarClass : x3::annotate_on_success, error_handler_base
{
};
}

namespace mimir::dl
{
parser::concept_type const& concept_() { return parser::concept_; }
parser::concept_bot_type const& concept_bot() { return parser::concept_bot; }
parser::concept_top_type const& concept_top() { return parser::concept_top; }
parser::concept_atomic_state_type const& concept_atomic_state() { return parser::concept_atomic_state; }
parser::concept_atomic_goal_type const& concept_atomic_goal() { return parser::concept_atomic_goal; }
parser::concept_intersection_type const& concept_intersection() { return parser::concept_intersection; }
parser::concept_union_type const& concept_union() { return parser::concept_union; }
parser::concept_negation_type const& concept_negation() { return parser::concept_negation; }
parser::concept_value_restriction_type const& concept_value_restriction() { return parser::concept_value_restriction; }
parser::concept_existential_quantification_type const& concept_existential_quantification() { return parser::concept_existential_quantification; }
parser::concept_role_value_map_containment_type const& concept_role_value_map_containment() { return parser::concept_role_value_map_containment; }
parser::concept_role_value_map_equality_type const& concept_role_value_map_equality() { return parser::concept_role_value_map_equality; }
parser::concept_nominal_type const& concept_nominal() { return parser::concept_nominal; }
parser::concept_non_terminal_type const& concept_non_terminal() { return parser::concept_non_terminal; }
parser::concept_choice_type const& concept_choice() { return parser::concept_choice; }
parser::concept_derivation_rule_type const& concept_derivation_rule() { return parser::concept_derivation_rule; }

parser::role_type const& role() { return parser::role; }
parser::role_universal_type const& role_universal() { return parser::role_universal; }
parser::role_atomic_state_type const& role_atomic_state() { return parser::role_atomic_state; }
parser::role_atomic_goal_type const& role_atomic_goal() { return parser::role_atomic_goal; }
parser::role_intersection_type const& role_intersection() { return parser::role_intersection; }
parser::role_union_type const& role_union() { return parser::role_union; }
parser::role_complement_type const& role_complement() { return parser::role_complement; }
parser::role_inverse_type const& role_inverse() { return parser::role_inverse; }
parser::role_composition_type const& role_composition() { return parser::role_composition; }
parser::role_transitive_closure_type const& role_transitive_closure() { return parser::role_transitive_closure; }
parser::role_reflexive_transitive_closure_type const& role_reflexive_transitive_closure() { return parser::role_reflexive_transitive_closure; }
parser::role_restriction_type const& role_restriction() { return parser::role_restriction; }
parser::role_identity_type const& role_identity() { return parser::role_identity; }
parser::role_non_terminal_type const& role_non_terminal() { return parser::role_non_terminal; }
parser::role_choice_type const& role_choice() { return parser::role_choice; }
parser::role_derivation_rule_type const& role_derivation_rule() { return parser::role_derivation_rule; }

parser::concept_or_role_derivation_rule_type const& concept_or_role_derivation_rule() { return parser::concept_or_role_derivation_rule; }
parser::grammar_type const& grammar_parser() { return parser::grammar; }
}

#endif
