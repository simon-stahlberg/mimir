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

#ifndef SRC_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTORS_PARSER_AST_ADAPTED_HPP_
#define SRC_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTORS_PARSER_AST_ADAPTED_HPP_

#include <boost/fusion/include/adapt_struct.hpp>
#include <mimir/languages/description_logics/parser/ast.hpp>

// We need to tell fusion about our rexpr and rexpr_key_value
// to make them a first-class fusion citizens

BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptBot)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptTop)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptAtomicState, predicate_name)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptAtomicGoal, predicate_name, is_negated)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptIntersection, concept_left, concept_right)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptUnion, concept_left, concept_right)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptNegation, concept_)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptValueRestriction, role_, concept_)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptExistentialQuantification, role_, concept_)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptRoleValueMapContainment, role_left, role_right)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptRoleValueMapEquality, role_left, role_right)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptNominal, object_name)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptNonTerminal, name)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptDerivationRule, non_terminal, choices)

BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleUniversal)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleAtomicState, predicate_name)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleAtomicGoal, predicate_name, is_negated)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleIntersection, role_left, role_right)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleUnion, role_left, role_right)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleComplement, role_)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleInverse, role_)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleComposition, role_left, role_right)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleTransitiveClosure, role_)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleReflexiveTransitiveClosure, role_)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleRestriction, role_, concept_)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleIdentity, concept_)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleNonTerminal, name)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleDerivationRule, non_terminal, choices)

BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::Grammar, rules)

#endif
