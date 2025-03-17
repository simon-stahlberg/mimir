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

#ifndef SRC_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTORS_SENTENCE_PARSER_PARSER_HPP_
#define SRC_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTORS_SENTENCE_PARSER_PARSER_HPP_

#include <boost/spirit/home/x3.hpp>
#include <mimir/languages/description_logics/parser/ast.hpp>

namespace mimir::languages::dl
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
namespace sentence_parser
{

template<FeatureCategory D>
struct ConstructorClass;

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

struct BooleanAtomicStateClass;
struct BooleanNonemptyClass;

struct NumericalCountClass;
struct NumericalDistanceClass;

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

typedef x3::rule<ConstructorClass<Boolean>, ast::Constructor<Boolean>> boolean_type;
typedef x3::rule<BooleanAtomicStateClass, ast::BooleanAtomicState> boolean_atomic_state_type;
typedef x3::rule<BooleanNonemptyClass, ast::BooleanNonempty> boolean_nonempty_type;

typedef x3::rule<ConstructorClass<Numerical>, ast::Constructor<Numerical>> numerical_type;
typedef x3::rule<NumericalCountClass, ast::NumericalCount> numerical_count_type;
typedef x3::rule<NumericalDistanceClass, ast::NumericalDistance> numerical_distance_type;

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
                     concept_nominal_type)

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
                     role_identity_type)

BOOST_SPIRIT_DECLARE(boolean_type, boolean_atomic_state_type, boolean_nonempty_type)

BOOST_SPIRIT_DECLARE(numerical_type, numerical_count_type, numerical_distance_type)

}

/**
 * Grammar
 */

sentence_parser::concept_type const& concept_();
sentence_parser::concept_bot_type const& concept_bot();
sentence_parser::concept_top_type const& concept_top();
sentence_parser::concept_atomic_state_type const& concept_atomic_state();
sentence_parser::concept_atomic_goal_type const& concept_atomic_goal();
sentence_parser::concept_intersection_type const& concept_intersection();
sentence_parser::concept_union_type const& concept_union();
sentence_parser::concept_negation_type const& concept_negation();
sentence_parser::concept_value_restriction_type const& concept_value_restriction();
sentence_parser::concept_existential_quantification_type const& concept_existential_quantification();
sentence_parser::concept_role_value_map_containment_type const& concept_role_value_map_containment();
sentence_parser::concept_role_value_map_equality_type const& concept_role_value_map_equality();
sentence_parser::concept_nominal_type const& concept_nominal();

sentence_parser::role_type const& role();
sentence_parser::role_universal_type const& role_universal();
sentence_parser::role_atomic_state_type const& role_atomic_state();
sentence_parser::role_atomic_goal_type const& role_atomic_goal();
sentence_parser::role_intersection_type const& role_intersection();
sentence_parser::role_union_type const& role_union();
sentence_parser::role_complement_type const& role_complement();
sentence_parser::role_inverse_type const& role_inverse();
sentence_parser::role_composition_type const& role_composition();
sentence_parser::role_transitive_closure_type const& role_transitive_closure();
sentence_parser::role_reflexive_transitive_closure_type const& role_reflexive_transitive_closure();
sentence_parser::role_restriction_type const& role_restriction();
sentence_parser::role_identity_type const& role_identity();

sentence_parser::boolean_type const& boolean();
sentence_parser::boolean_atomic_state_type const& boolean_atomic_state();
sentence_parser::boolean_nonempty_type const& boolean_nonempty();

sentence_parser::numerical_type const& numerical();
sentence_parser::numerical_count_type const& numerical_count();
sentence_parser::numerical_distance_type const& numerical_distance();
}

#endif
