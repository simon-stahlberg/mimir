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

#ifndef SRC_LANGUAGES_DESCRIPTION_LOGICS_SENTENCE_PARSER_PARSER_DEF_HPP_
#define SRC_LANGUAGES_DESCRIPTION_LOGICS_SENTENCE_PARSER_PARSER_DEF_HPP_

#include "../parser/ast.hpp"
#include "../parser/ast_adapted.hpp"
#include "../parser/error_handler.hpp"
#include "mimir/languages/description_logics/constructor_keywords.hpp"
#include "mimir/languages/description_logics/constructors.hpp"
#include "parser.hpp"

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>

namespace mimir::languages::dl::sentence_parser
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

concept_root_type const concept_root = "concept_root";
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

role_root_type const role_root = "role_root";
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

concept_or_role_type const concept_or_role = "concept_or_role";

boolean_root_type const boolean_root = "boolean_root";
boolean_type const boolean = "boolean";
boolean_atomic_state_type const boolean_atomic_state = "boolean_atomic_state";
boolean_nonempty_type const boolean_nonempty = "boolean_nonempty";

numerical_root_type const numerical_root = "numerical_root";
numerical_type const numerical = "numerical";
numerical_count_type const numerical_count = "numerical_count";
numerical_distance_type const numerical_distance = "numerical_distance";

///////////////////////////////////////////////////////////////////////////
// Grammar
///////////////////////////////////////////////////////////////////////////

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

const auto concept_root_def = concept_;
const auto concept__def = concept_bot | concept_top | concept_atomic_state | concept_atomic_goal | concept_intersection | concept_union | concept_negation
                          | concept_value_restriction | concept_existential_quantification | concept_role_value_map_containment
                          | concept_role_value_map_equality | concept_nominal;

const auto concept_bot_def = lit(std::string("@") + keywords::concept_bot) >> x3::attr(ast::ConceptBot {});
const auto concept_top_def = lit(std::string("@") + keywords::concept_top) >> x3::attr(ast::ConceptTop {});
const auto concept_atomic_state_def = lit(std::string("@") + keywords::concept_atomic_state) > predicate_name_string_parser();
const auto concept_atomic_goal_def = lit(std::string("@") + keywords::concept_atomic_goal) > predicate_name_string_parser() > bool_string_parser();
const auto concept_intersection_def = lit(std::string("@") + keywords::concept_intersection) > concept_ > concept_;
const auto concept_union_def = lit(std::string("@") + keywords::concept_union) > concept_ > concept_;
const auto concept_negation_def = lit(std::string("@") + keywords::concept_negation) > concept_;
const auto concept_value_restriction_def = lit(std::string("@") + keywords::concept_value_restriction) > role > concept_;
const auto concept_existential_quantification_def = lit(std::string("@") + keywords::concept_existential_quantification) > role > concept_;
const auto concept_role_value_map_containment_def = lit(std::string("@") + keywords::concept_role_value_map_containment) > role > role;
const auto concept_role_value_map_equality_def = lit(std::string("@") + keywords::concept_role_value_map_equality) > role > role;
const auto concept_nominal_def = lit(std::string("@") + keywords::concept_nominal) > object_name_string_parser();

const auto role_root_def = role;
const auto role_def = role_universal | role_atomic_state | role_atomic_goal | role_intersection | role_union | role_complement | role_inverse | role_composition
                      | role_transitive_closure | role_reflexive_transitive_closure | role_restriction | role_identity;

const auto role_universal_def = lit(std::string("@") + keywords::role_universal) >> x3::attr(ast::RoleUniversal {});
const auto role_atomic_state_def = lit(std::string("@") + keywords::role_atomic_state) > predicate_name_string_parser();
const auto role_atomic_goal_def = lit(std::string("@") + keywords::role_atomic_goal) > predicate_name_string_parser() > bool_string_parser();
const auto role_intersection_def = lit(std::string("@") + keywords::role_intersection) > role > role;
const auto role_union_def = lit(std::string("@") + keywords::role_union) > role > role;
const auto role_complement_def = lit(std::string("@") + keywords::role_complement) > role;
const auto role_inverse_def = lit(std::string("@") + keywords::role_inverse) > role;
const auto role_composition_def = lit(std::string("@") + keywords::role_composition) > role > role;
const auto role_transitive_closure_def = lit(std::string("@") + keywords::role_transitive_closure) > role;
const auto role_reflexive_transitive_closure_def = lit(std::string("@") + keywords::role_reflexive_transitive_closure) > role;
const auto role_restriction_def = lit(std::string("@") + keywords::role_restriction) > role > concept_;
const auto role_identity_def = lit(std::string("@") + keywords::role_identity) > concept_;

const auto concept_or_role_def = concept_ | role;

const auto boolean_root_def = boolean;
const auto boolean_def = boolean_atomic_state | boolean_nonempty;
const auto boolean_atomic_state_def = lit(std::string("@") + keywords::boolean_atomic_state) > predicate_name_string_parser();
const auto boolean_nonempty_def = lit(std::string("@") + keywords::boolean_nonempty) > concept_or_role;

const auto numerical_root_def = numerical;
const auto numerical_def = numerical_count | numerical_distance;
const auto numerical_count_def = lit(std::string("@") + keywords::numerical_count) > concept_or_role;
const auto numerical_distance_def = lit(std::string("@") + keywords::numerical_distance) > concept_ > role > concept_;

BOOST_SPIRIT_DEFINE(concept_root,
                    concept_,
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
                    concept_nominal)

BOOST_SPIRIT_DEFINE(role_root,
                    role,
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
                    role_identity)

BOOST_SPIRIT_DEFINE(concept_or_role)

BOOST_SPIRIT_DEFINE(boolean_root, boolean, boolean_atomic_state, boolean_nonempty)

BOOST_SPIRIT_DEFINE(numerical_root, numerical, numerical_count, numerical_distance)

///////////////////////////////////////////////////////////////////////////
// Annotation and Error handling
///////////////////////////////////////////////////////////////////////////

template<>
struct ConstructorRootClass<ConceptTag> : x3::annotate_on_success, error_handler_base
{
};
template<>
struct ConstructorRootClass<RoleTag> : x3::annotate_on_success, error_handler_base
{
};
template<>
struct ConstructorRootClass<BooleanTag> : x3::annotate_on_success, error_handler_base
{
};
template<>
struct ConstructorRootClass<NumericalTag> : x3::annotate_on_success, error_handler_base
{
};

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

struct ConceptOrRoleClass : x3::annotate_on_success
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

}

namespace mimir::languages::dl::sentence_parser
{
concept_root_type const& concept_root_parser() { return concept_root; }
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

role_root_type const& role_root_parser() { return role_root; }
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

concept_or_role_type const& concept_or_role_parser() { return concept_or_role; }

boolean_root_type const& boolean_root_parser() { return boolean_root; }
boolean_type const& boolean_parser() { return boolean; }
boolean_atomic_state_type const& boolean_atomic_state_parser() { return boolean_atomic_state; }
boolean_nonempty_type const& boolean_nonempty_parser() { return boolean_nonempty; }

numerical_root_type const& numerical_root_parser() { return numerical_root; }
numerical_type const& numerical_parser() { return numerical; }
numerical_count_type const& numerical_count_parser() { return numerical_count; }
numerical_distance_type const& numerical_distance_parser() { return numerical_distance; }

}

#endif
