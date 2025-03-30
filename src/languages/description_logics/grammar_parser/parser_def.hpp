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

#ifndef SRC_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_PARSER_PARSER_DEF_HPP_
#define SRC_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_PARSER_PARSER_DEF_HPP_

#include "../parser/ast.hpp"
#include "../parser/ast_adapted.hpp"
#include "../parser/error_handler.hpp"
#include "mimir/languages/description_logics/constructor_keywords.hpp"
#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/description_logics/declarations.hpp"
#include "parser.hpp"

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>

namespace mimir::languages::dl::grammar_parser
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

concept_or_role_nonterminal_type const concept_or_role_nonterminal = "concept_or_role_nonterminal";

boolean_type const boolean = "boolean";
boolean_atomic_state_type const boolean_atomic_state = "boolean_atomic_state";
boolean_nonempty_type const boolean_nonempty = "boolean_nonempty";
boolean_non_terminal_type const boolean_non_terminal = "boolean_non_terminal";
boolean_choice_type const boolean_choice = "boolean_choice";
boolean_derivation_rule_type const boolean_derivation_rule = "boolean_derivation_rule";

numerical_type const numerical = "numerical";
numerical_count_type const numerical_count = "numerical_count";
numerical_distance_type const numerical_distance = "numerical_distance";
numerical_non_terminal_type const numerical_non_terminal = "numerical_non_terminal";
numerical_choice_type const numerical_choice = "numerical_choice";
numerical_derivation_rule_type const numerical_derivation_rule = "numerical_derivation_rule";

feature_category_derivation_rule_type const feature_category_derivation_rule = "feature_category_derivation_rule";

grammar_head_type const grammar_head = "grammar_head";
grammar_body_type const grammar_body = "grammar_body";

grammar_type const grammar = "grammar";

///////////////////////////////////////////////////////////////////////////
// Grammar
///////////////////////////////////////////////////////////////////////////

inline auto concept_non_terminal_string_parser() { return raw[lexeme["<concept" > *(alnum | char_('-') | char_('_')) > ">"]]; }
inline auto role_non_terminal_string_parser() { return raw[lexeme["<role" > *(alnum | char_('-') | char_('_')) > ">"]]; }
inline auto boolean_non_terminal_string_parser() { return raw[lexeme["<boolean" > *(alnum | char_('-') | char_('_')) > ">"]]; }
inline auto numerical_non_terminal_string_parser() { return raw[lexeme["<numerical" > *(alnum | char_('-') | char_('_')) > ">"]]; }
inline auto predicate_name_string_parser()
{
    return lexeme[omit[lit('"')]] > raw[lexeme[alpha >> *(alnum | char_('-') | char_('_'))]] > lexeme[omit[lit('"')]];
}
inline auto object_name_string_parser() { return lexeme[omit[lit('"')]] > raw[lexeme[alpha >> *(alnum | char_('-') | char_('_'))]] > lexeme[omit[lit('"')]]; }
inline auto bool_string_parser()
{
    return x3::lexeme[(x3::lit("true") >> x3::attr(true)) | (x3::lit("false") >> x3::attr(false)) | (x3::lit("1") >> x3::attr(true))
                      | (x3::lit("0") >> x3::attr(false))];
}

const auto concept__def = concept_bot | concept_top | concept_atomic_state | concept_atomic_goal | concept_intersection | concept_union | concept_negation
                          | concept_value_restriction | concept_existential_quantification | concept_role_value_map_containment
                          | concept_role_value_map_equality | concept_nominal;

const auto concept_bot_def = lit(std::string("@") + keywords::concept_bot) >> x3::attr(ast::ConceptBot {});
const auto concept_top_def = lit(std::string("@") + keywords::concept_top) >> x3::attr(ast::ConceptTop {});
const auto concept_atomic_state_def = lit(std::string("@") + keywords::concept_atomic_state) > predicate_name_string_parser();
const auto concept_atomic_goal_def = lit(std::string("@") + keywords::concept_atomic_goal) > predicate_name_string_parser() > bool_string_parser();
const auto concept_intersection_def = lit(std::string("@") + keywords::concept_intersection) > concept_choice > concept_choice;
const auto concept_union_def = lit(std::string("@") + keywords::concept_union) > concept_choice > concept_choice;
const auto concept_negation_def = lit(std::string("@") + keywords::concept_negation) > concept_choice;
const auto concept_value_restriction_def = lit(std::string("@") + keywords::concept_value_restriction) > role_choice > concept_choice;
const auto concept_existential_quantification_def = lit(std::string("@") + keywords::concept_existential_quantification) > role_choice > concept_choice;
const auto concept_role_value_map_containment_def = lit(std::string("@") + keywords::concept_role_value_map_containment) > role_choice > role_choice;
const auto concept_role_value_map_equality_def = lit(std::string("@") + keywords::concept_role_value_map_equality) > role_choice > role_choice;
const auto concept_nominal_def = lit(std::string("@") + keywords::concept_nominal) > object_name_string_parser();
const auto concept_non_terminal_def = concept_non_terminal_string_parser();
const auto concept_choice_def = concept_non_terminal | concept_;
const auto concept_derivation_rule_def = concept_non_terminal > "::=" > -(concept_choice % lit("|"));

const auto role_def = role_universal | role_atomic_state | role_atomic_goal | role_intersection | role_union | role_complement | role_inverse | role_composition
                      | role_transitive_closure | role_reflexive_transitive_closure | role_restriction | role_identity;

const auto role_universal_def = lit(std::string("@") + keywords::role_universal) >> x3::attr(ast::RoleUniversal {});
const auto role_atomic_state_def = lit(std::string("@") + keywords::role_atomic_state) > predicate_name_string_parser();
const auto role_atomic_goal_def = lit(std::string("@") + keywords::role_atomic_goal) > predicate_name_string_parser() > bool_string_parser();
const auto role_intersection_def = lit(std::string("@") + keywords::role_intersection) > role_choice > role_choice;
const auto role_union_def = lit(std::string("@") + keywords::role_union) > role_choice > role_choice;
const auto role_complement_def = lit(std::string("@") + keywords::role_complement) > role_choice;
const auto role_inverse_def = lit(std::string("@") + keywords::role_inverse) > role_choice;
const auto role_composition_def = lit(std::string("@") + keywords::role_composition) > role_choice > role_choice;
const auto role_transitive_closure_def = lit(std::string("@") + keywords::role_transitive_closure) > role_choice;
const auto role_reflexive_transitive_closure_def = lit(std::string("@") + keywords::role_reflexive_transitive_closure) > role_choice;
const auto role_restriction_def = lit(std::string("@") + keywords::role_restriction) > role_choice > concept_choice;
const auto role_identity_def = lit(std::string("@") + keywords::role_identity) > concept_choice;
const auto role_non_terminal_def = role_non_terminal_string_parser();
const auto role_choice_def = role_non_terminal | role;
const auto role_derivation_rule_def = role_non_terminal > "::=" > -(role_choice % lit("|"));

const auto concept_or_role_nonterminal_def = concept_choice | role_choice;

const auto boolean_def = boolean_atomic_state | boolean_nonempty;
const auto boolean_atomic_state_def = lit(std::string("@") + keywords::boolean_atomic_state) > predicate_name_string_parser();
const auto boolean_nonempty_def = lit(std::string("@") + keywords::boolean_nonempty) > concept_or_role_nonterminal;
const auto boolean_non_terminal_def = boolean_non_terminal_string_parser();
const auto boolean_choice_def = boolean_non_terminal | boolean;
const auto boolean_derivation_rule_def = boolean_non_terminal > "::=" > -(boolean_choice % lit("|"));

const auto numerical_def = numerical_count | numerical_distance;
const auto numerical_count_def = lit(std::string("@") + keywords::numerical_count) > concept_or_role_nonterminal;
const auto numerical_distance_def = lit(std::string("@") + keywords::numerical_distance) > concept_choice > role_choice > concept_choice;
const auto numerical_non_terminal_def = numerical_non_terminal_string_parser();
const auto numerical_choice_def = numerical_non_terminal | numerical;
const auto numerical_derivation_rule_def = numerical_non_terminal > "::=" > -(numerical_choice % lit("|"));

const auto feature_category_derivation_rule_def = (concept_derivation_rule | role_derivation_rule | boolean_derivation_rule | numerical_derivation_rule);
const auto grammar_head_def = lit("[start_symbols]")                                 //
                              > -(lit("concept") > lit("=") > concept_non_terminal)  //
                              > -(lit("role") > lit("=") > role_non_terminal)        //
                              > -(lit("boolean") > lit("=") > boolean_non_terminal)  //
                              > -(lit("numerical") > lit("=") > numerical_non_terminal);
const auto grammar_body_def = lit("[grammar_rules]") > *feature_category_derivation_rule;
const auto grammar_def = grammar_head > grammar_body;

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

BOOST_SPIRIT_DEFINE(concept_or_role_nonterminal)

BOOST_SPIRIT_DEFINE(boolean, boolean_atomic_state, boolean_nonempty, boolean_non_terminal, boolean_choice, boolean_derivation_rule)

BOOST_SPIRIT_DEFINE(numerical, numerical_count, numerical_distance, numerical_non_terminal, numerical_choice, numerical_derivation_rule)

BOOST_SPIRIT_DEFINE(feature_category_derivation_rule, grammar_head, grammar_body, grammar)

///////////////////////////////////////////////////////////////////////////
// Annotation and Error handling
///////////////////////////////////////////////////////////////////////////

template<>
struct ConstructorClass<ConceptTag> : x3::annotate_on_success
{
};
template<>
struct ConstructorClass<RoleTag> : x3::annotate_on_success
{
};
template<>
struct ConstructorClass<BooleanTag> : x3::annotate_on_success
{
};
template<>
struct ConstructorClass<NumericalTag> : x3::annotate_on_success
{
};

template<>
struct NonTerminalClass<ConceptTag> : x3::annotate_on_success
{
};
template<>
struct NonTerminalClass<RoleTag> : x3::annotate_on_success
{
};
template<>
struct NonTerminalClass<BooleanTag> : x3::annotate_on_success
{
};
template<>
struct NonTerminalClass<NumericalTag> : x3::annotate_on_success
{
};
template<>
struct ConstructorOrNonTerminalClass<ConceptTag> : x3::annotate_on_success
{
};
template<>
struct ConstructorOrNonTerminalClass<RoleTag> : x3::annotate_on_success
{
};
template<>
struct ConstructorOrNonTerminalClass<BooleanTag> : x3::annotate_on_success
{
};
template<>
struct ConstructorOrNonTerminalClass<NumericalTag> : x3::annotate_on_success
{
};
template<>
struct DerivationRuleClass<ConceptTag> : x3::annotate_on_success
{
};
template<>
struct DerivationRuleClass<RoleTag> : x3::annotate_on_success
{
};
template<>
struct DerivationRuleClass<BooleanTag> : x3::annotate_on_success
{
};
template<>
struct DerivationRuleClass<NumericalTag> : x3::annotate_on_success
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

struct ConceptOrRoleNonterminalClass : x3::annotate_on_success
{
};

struct BooleanAtomicStateClass : x3::annotate_on_success
{
};
struct BooleanNonemptyClass : x3::annotate_on_success
{
};

struct NumericalCountClass : x3::annotate_on_success
{
};
struct NumericalDistanceClass : x3::annotate_on_success
{
};

struct FeatureCategoryDerivationRuleClass : x3::annotate_on_success
{
};
struct GrammarHeadClass : x3::annotate_on_success
{
};
struct GrammarBodyClass : x3::annotate_on_success
{
};
struct GrammarClass : x3::annotate_on_success, error_handler_base
{
};
}

namespace mimir::languages::dl::grammar_parser
{
concept_type const& concept_parser() { return concept_; }
concept_bot_type const& concept_bot_parser() { return concept_bot; }
concept_top_type const& concept_top_parser() { return concept_top; }
concept_atomic_state_type const& concept_atomic_state_parser() { return concept_atomic_state; }
concept_atomic_goal_type const& concept_atomic_goal_parser() { return concept_atomic_goal; }
concept_intersection_type const& concept_intersection_parser() { return concept_intersection; }
concept_union_type const& concept_union_parser() { return concept_union; }
concept_negation_type const& concept_negation_parser() { return concept_negation; }
concept_value_restriction_type const& concept_value_restriction_parser() { return concept_value_restriction; }
concept_existential_quantification_type const& concept_existential_quantification_parser() { return concept_existential_quantification; }
concept_role_value_map_containment_type const& concept_role_value_map_containment_parser() { return concept_role_value_map_containment; }
concept_role_value_map_equality_type const& concept_role_value_map_equality_parser() { return concept_role_value_map_equality; }
concept_nominal_type const& concept_nominal_parser() { return concept_nominal; }
concept_non_terminal_type const& concept_non_terminal_parser() { return concept_non_terminal; }
concept_choice_type const& concept_choice_parser() { return concept_choice; }
concept_derivation_rule_type const& concept_derivation_rule_parser() { return concept_derivation_rule; }

role_type const& role_parser() { return role; }
role_universal_type const& role_universal_parser() { return role_universal; }
role_atomic_state_type const& role_atomic_state_parser() { return role_atomic_state; }
role_atomic_goal_type const& role_atomic_goal_parser() { return role_atomic_goal; }
role_intersection_type const& role_intersection_parser() { return role_intersection; }
role_union_type const& role_union_parser() { return role_union; }
role_complement_type const& role_complement_parser() { return role_complement; }
role_inverse_type const& role_inverse_parser() { return role_inverse; }
role_composition_type const& role_composition_parser() { return role_composition; }
role_transitive_closure_type const& role_transitive_closure_parser() { return role_transitive_closure; }
role_reflexive_transitive_closure_type const& role_reflexive_transitive_closure_parser() { return role_reflexive_transitive_closure; }
role_restriction_type const& role_restriction_parser() { return role_restriction; }
role_identity_type const& role_identity_parser() { return role_identity; }
role_non_terminal_type const& role_non_terminal_parser() { return role_non_terminal; }
role_choice_type const& role_choice_parser() { return role_choice; }
role_derivation_rule_type const& role_derivation_rule_parser() { return role_derivation_rule; }

concept_or_role_nonterminal_type const& concept_or_role_nonterminal_parser() { return concept_or_role_nonterminal; }

boolean_type const& boolean_parser() { return boolean; }
boolean_atomic_state_type const& boolean_atomic_state_parser() { return boolean_atomic_state; }
boolean_nonempty_type const& boolean_nonempty_parser() { return boolean_nonempty; }
boolean_non_terminal_type const& boolean_non_terminal_parser() { return boolean_non_terminal; }
boolean_choice_type const& boolean_choice_parser() { return boolean_choice; }
boolean_derivation_rule_type const& boolean_derivation_rule_parser() { return boolean_derivation_rule; }

numerical_type const& numerical_parser() { return numerical; }
numerical_count_type const& numerical_count_parser() { return numerical_count; }
numerical_distance_type const& numerical_distance_parser() { return numerical_distance; }
numerical_non_terminal_type const& numerical_non_terminal_parser() { return numerical_non_terminal; }
numerical_choice_type const& numerical_choice_parser() { return numerical_choice; }
numerical_derivation_rule_type const& numerical_derivation_rule_parser() { return numerical_derivation_rule; }

feature_category_derivation_rule_type const& feature_category_derivation_rule_parser() { return feature_category_derivation_rule; }
grammar_head_type const& grammar_head_parser() { return grammar_head; }
grammar_body_type const& grammar_body_parser() { return grammar_body; }
grammar_type const& grammar_parser_() { return grammar; }
}

#endif
