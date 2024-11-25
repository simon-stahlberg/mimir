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

#include "mimir/languages/description_logics/parser/ast.hpp"

#include <boost/fusion/include/adapt_struct.hpp>

// We need to tell fusion about our rexpr and rexpr_key_value
// to make them a first-class fusion citizens

BOOST_FUSION_ADAPT_TPL_STRUCT((D), (mimir::dl::ast::NonTerminal)(D), (std::string, name))
BOOST_FUSION_ADAPT_TPL_STRUCT((D),
                              (mimir::dl::ast::DerivationRule)(D),
                              (mimir::dl::ast::NonTerminal<D>, non_terminal)(std::vector<mimir::dl::ast::ConstructorOrNonTerminal<D>>,
                                                                             constructor_or_non_terminals))

BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptBot)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptTop)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptAtomicState, predicate_name)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptAtomicGoal, predicate_name, is_negated)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptIntersection, concept_or_non_terminal_left, concept_or_non_terminal_right)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptUnion, concept_or_non_terminal_left, concept_or_non_terminal_right)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptNegation, concept_or_non_terminal)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptValueRestriction, role_or_non_terminal, concept_or_non_terminal)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptExistentialQuantification, role_or_non_terminal, concept_or_non_terminal)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptRoleValueMapContainment, role_or_non_terminal_left, role_or_non_terminal_right)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptRoleValueMapEquality, role_or_non_terminal_left, role_or_non_terminal_right)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::ConceptNominal, object_name)

BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleUniversal)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleAtomicState, predicate_name)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleAtomicGoal, predicate_name, is_negated)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleIntersection, role_or_non_terminal_left, role_or_non_terminal_right)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleUnion, role_or_non_terminal_left, role_or_non_terminal_right)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleComplement, role_or_non_terminal)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleInverse, role_or_non_terminal)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleComposition, role_or_non_terminal_left, role_or_non_terminal_right)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleTransitiveClosure, role_or_non_terminal)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleReflexiveTransitiveClosure, role_or_non_terminal)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleRestriction, role_or_non_terminal, concept_or_non_terminal)
BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::RoleIdentity, concept_or_non_terminal)

BOOST_FUSION_ADAPT_STRUCT(mimir::dl::ast::Grammar, rules)

#endif
