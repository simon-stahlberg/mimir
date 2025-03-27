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

#ifndef SRC_LANGUAGES_DESCRIPTION_LOGICS_PARSER_AST_ADAPTED_HPP_
#define SRC_LANGUAGES_DESCRIPTION_LOGICS_PARSER_AST_ADAPTED_HPP_

#include "ast.hpp"

#include <boost/fusion/include/adapt_struct.hpp>

// We need to tell fusion about our rexpr and rexpr_key_value
// to make them a first-class fusion citizens

BOOST_FUSION_ADAPT_TPL_STRUCT((D), (mimir::languages::dl::ast::NonTerminal)(D), (std::string, name))
BOOST_FUSION_ADAPT_TPL_STRUCT((D),
                              (mimir::languages::dl::ast::DerivationRule)(D),
                              (mimir::languages::dl::ast::NonTerminal<D>, non_terminal)(std::vector<mimir::languages::dl::ast::ConstructorOrNonTerminal<D>>,
                                                                                        constructor_or_non_terminals))

BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::ConceptBot)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::ConceptTop)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::ConceptAtomicState, predicate_name)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::ConceptAtomicGoal, predicate_name, polarity)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::ConceptIntersection, left_concept_or_non_terminal, right_concept_or_non_terminal)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::ConceptUnion, left_concept_or_non_terminal, right_concept_or_non_terminal)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::ConceptNegation, concept_or_non_terminal)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::ConceptValueRestriction, role_or_non_terminal, concept_or_non_terminal)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::ConceptExistentialQuantification, role_or_non_terminal, concept_or_non_terminal)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::ConceptRoleValueMapContainment, left_role_or_non_terminal, right_role_or_non_terminal)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::ConceptRoleValueMapEquality, left_role_or_non_terminal, right_role_or_non_terminal)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::ConceptNominal, object_name)

BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::RoleUniversal)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::RoleAtomicState, predicate_name)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::RoleAtomicGoal, predicate_name, polarity)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::RoleIntersection, left_role_or_non_terminal, right_role_or_non_terminal)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::RoleUnion, left_role_or_non_terminal, right_role_or_non_terminal)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::RoleComplement, role_or_non_terminal)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::RoleInverse, role_or_non_terminal)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::RoleComposition, left_role_or_non_terminal, right_role_or_non_terminal)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::RoleTransitiveClosure, role_or_non_terminal)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::RoleReflexiveTransitiveClosure, role_or_non_terminal)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::RoleRestriction, role_or_non_terminal, concept_or_non_terminal)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::RoleIdentity, concept_or_non_terminal)

BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::BooleanAtomicState, predicate_name)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::BooleanNonempty, concept_or_role_nonterminal)

BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::NumericalCount, concept_or_role_nonterminal)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::NumericalDistance, left_concept_or_nonterminal, role_or_nonterminal, right_concept_or_nonterminal)

BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::GrammarHead, concept_start, role_start, boolean_start, numerical_start)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::GrammarBody, rules)

BOOST_FUSION_ADAPT_STRUCT(mimir::languages::dl::ast::Grammar, head, body)

#endif
