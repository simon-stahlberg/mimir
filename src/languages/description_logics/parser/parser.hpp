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
template<ConstructorTag D>
struct ConstructorClass;

template<ConstructorTag D>
struct NonTerminalClass;

template<ConstructorTag D>
struct ConstructorOrNonTerminalClass;

template<ConstructorTag D>
struct DerivationRuleClass;

struct ConceptOrRoleDerivationRuleClass;

struct ConceptBotClass;
struct ConceptTopClass;
struct ConceptAtomicStateClass;
struct ConceptAtomicGoalClass;
struct ConceptIntersectionClass;
struct ConceptUnionClass;
struct ConceptNegationClass;
struct ConceptValueRestrictionClass;
struct ConceptExistentialQuantificationClass;
struct ConceptRoleValueMapContainmentClass;
struct ConceptRoleValueMapEqualityClass;
struct ConceptNominalClass;

struct RoleUniversalClass;
struct RoleAtomicStateClass;
struct RoleAtomicGoalClass;
struct RoleIntersectionClass;
struct RoleUnionClass;
struct RoleComplementClass;
struct RoleInverseClass;
struct RoleCompositionClass;
struct RoleTransitiveClosureClass;
struct RoleReflexiveTransitiveClosureClass;
struct RoleRestrictionClass;
struct RoleIdentityClass;

typedef x3::rule<ConstructorClass<Concept>, ast::Constructor<Concept>> concept_type;
typedef x3::rule<ConceptBotClass, ast::ConceptBot> concept_bot_type;
typedef x3::rule<ConceptTopClass, ast::ConceptTop> concept_top_type;
typedef x3::rule<ConceptAtomicStateClass, ast::ConceptAtomicState> concept_atomic_state_type;
typedef x3::rule<ConceptAtomicGoalClass, ast::ConceptAtomicGoal> concept_atomic_goal_type;
typedef x3::rule<ConceptIntersectionClass, ast::ConceptIntersection> concept_intersection_type;
typedef x3::rule<ConceptUnionClass, ast::ConceptUnion> concept_union_type;
typedef x3::rule<ConceptNegationClass, ast::ConceptNegation> concept_negation_type;
typedef x3::rule<ConceptValueRestrictionClass, ast::ConceptValueRestriction> concept_value_restriction_type;
typedef x3::rule<ConceptExistentialQuantificationClass, ast::ConceptExistentialQuantification> concept_existential_quantification_type;
typedef x3::rule<ConceptRoleValueMapContainmentClass, ast::ConceptRoleValueMapContainment> concept_role_value_map_containment_type;
typedef x3::rule<ConceptRoleValueMapEqualityClass, ast::ConceptRoleValueMapEquality> concept_role_value_map_equality_type;
typedef x3::rule<ConceptNominalClass, ast::ConceptNominal> concept_nominal_type;
typedef x3::rule<NonTerminalClass<Concept>, ast::NonTerminal<Concept>> concept_non_terminal_type;
typedef x3::rule<ConstructorOrNonTerminalClass<Concept>, ast::ConstructorOrNonTerminal<Concept>> concept_choice_type;
typedef x3::rule<DerivationRuleClass<Concept>, ast::DerivationRule<Concept>> concept_derivation_rule_type;

typedef x3::rule<ConstructorClass<Role>, ast::Constructor<Role>> role_type;
typedef x3::rule<RoleUniversalClass, ast::RoleUniversal> role_universal_type;
typedef x3::rule<RoleAtomicStateClass, ast::RoleAtomicState> role_atomic_state_type;
typedef x3::rule<RoleAtomicGoalClass, ast::RoleAtomicGoal> role_atomic_goal_type;
typedef x3::rule<RoleIntersectionClass, ast::RoleIntersection> role_intersection_type;
typedef x3::rule<RoleUnionClass, ast::RoleUnion> role_union_type;
typedef x3::rule<RoleComplementClass, ast::RoleComplement> role_complement_type;
typedef x3::rule<RoleInverseClass, ast::RoleInverse> role_inverse_type;
typedef x3::rule<RoleCompositionClass, ast::RoleComposition> role_composition_type;
typedef x3::rule<RoleTransitiveClosureClass, ast::RoleTransitiveClosure> role_transitive_closure_type;
typedef x3::rule<RoleReflexiveTransitiveClosureClass, ast::RoleReflexiveTransitiveClosure> role_reflexive_transitive_closure_type;
typedef x3::rule<RoleRestrictionClass, ast::RoleRestriction> role_restriction_type;
typedef x3::rule<RoleIdentityClass, ast::RoleIdentity> role_identity_type;
typedef x3::rule<NonTerminalClass<Role>, ast::NonTerminal<Role>> role_non_terminal_type;
typedef x3::rule<ConstructorOrNonTerminalClass<Role>, ast::ConstructorOrNonTerminal<Role>> role_choice_type;
typedef x3::rule<DerivationRuleClass<Role>, ast::DerivationRule<Role>> role_derivation_rule_type;

typedef x3::rule<ConceptOrRoleDerivationRuleClass, ast::ConceptOrRoleDerivationRule> concept_or_role_derivation_rule_type;

BOOST_SPIRIT_DECLARE(concept_type,
                     concept_bot_type,
                     concept_top_type,
                     concept_atomic_state_type,
                     concept_atomic_goal_type,
                     concept_intersection_type,
                     concept_union_type,
                     concept_negation_type,
                     concept_value_restriction_type,
                     concept_existential_quantification_type,
                     concept_role_value_map_containment_type,
                     concept_role_value_map_equality_type,
                     concept_nominal_type,
                     concept_non_terminal_type,
                     concept_choice_type,
                     concept_derivation_rule_type)

BOOST_SPIRIT_DECLARE(role_type,
                     role_universal_type,
                     role_atomic_state_type,
                     role_atomic_goal_type,
                     role_intersection_type,
                     role_union_type,
                     role_complement_type,
                     role_inverse_type,
                     role_composition_type,
                     role_transitive_closure_type,
                     role_reflexive_transitive_closure_type,
                     role_restriction_type,
                     role_identity_type,
                     role_non_terminal_type,
                     role_choice_type,
                     role_derivation_rule_type)

BOOST_SPIRIT_DECLARE(concept_or_role_derivation_rule_type)

}

parser::concept_type const& concept_();
parser::concept_bot_type const& concept_bot();
parser::concept_top_type const& concept_top();
parser::concept_atomic_state_type const& concept_atomic_state();
parser::concept_atomic_goal_type const& concept_atomic_goal();
parser::concept_intersection_type const& concept_intersection();
parser::concept_union_type const& concept_union();
parser::concept_negation_type const& concept_negation();
parser::concept_value_restriction_type const& concept_value_restriction();
parser::concept_existential_quantification_type const& concept_existential_quantification();
parser::concept_role_value_map_containment_type const& concept_role_value_map_containment();
parser::concept_role_value_map_equality_type const& concept_role_value_map_equality();
parser::concept_nominal_type const& concept_nominal();
parser::concept_non_terminal_type const& concept_non_terminal();
parser::concept_choice_type const& concept_choice();
parser::concept_derivation_rule_type const& concept_derivation_rule();

parser::role_type const& role();
parser::role_universal_type const& role_universal();
parser::role_atomic_state_type const& role_atomic_state();
parser::role_atomic_goal_type const& role_atomic_goal();
parser::role_intersection_type const& role_intersection();
parser::role_union_type const& role_union();
parser::role_complement_type const& role_complement();
parser::role_inverse_type const& role_inverse();
parser::role_composition_type const& role_composition();
parser::role_transitive_closure_type const& role_transitive_closure();
parser::role_reflexive_transitive_closure_type const& role_reflexive_transitive_closure();
parser::role_restriction_type const& role_restriction();
parser::role_identity_type const& role_identity();
parser::role_non_terminal_type const& role_non_terminal();
parser::role_choice_type const& role_choice();
parser::role_derivation_rule_type const& role_derivation_rule();

parser::concept_or_role_derivation_rule_type const& concept_or_role_derivation_rule();
}

#endif
