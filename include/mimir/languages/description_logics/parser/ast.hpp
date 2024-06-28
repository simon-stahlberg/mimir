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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTORS_PARSER_AST_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTORS_PARSER_AST_HPP_

#include <boost/optional.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <map>
#include <sstream>
#include <vector>

namespace mimir::dl::ast
{
///////////////////////////////////////////////////////////////////////////
//  The AST
///////////////////////////////////////////////////////////////////////////
namespace x3 = boost::spirit::x3;

/// Example BNF grammar DL fragment with restriction on role-value map (equal)
/// <concept_predicate1_state> ::= @concept_predicate_state "predicate1"
/// <concept_predicate1_goal> ::= @concept_predicate_goal "predicate1"
/// <concept_all> ::= "@concept_all" <role> <concept>
/// <concept_equal> ::= "@concept_equal" <concept_predicate1_state> <concept_predicate1_goal>
/// <concept_bot> ::= "@concept_bot"
/// <concept_top> ::= "@concept_top"
/// <concept> ::= <concept_predicate1_state> | <concept_predicate1_goal> | <concept_bot> | <concept_top> | <concept_all> | <concept_equal>
/// <role_predicate2_state> ::= @role_predicate_state "predicate2"
/// <role_predicate2_goal> ::= @role_predicate_goal "predicate2"
/// <role_bot> ::= "@role_bot"
/// <role_top> ::= "@role_top"
/// <role> ::= <role_predicate2_state> | <role_predicate2_goal> | <role_bot> | <role_top>

/**
 * Concepts
 */

struct ConceptNonTerminal;
struct ConceptPredicateState;
struct ConceptPredicateGoal;
struct ConceptAnd;

struct Concept :
    x3::position_tagged,
    x3::variant<x3::forward_ast<ConceptNonTerminal>,  //
                x3::forward_ast<ConceptPredicateState>,
                x3::forward_ast<ConceptPredicateGoal>,
                x3::forward_ast<ConceptAnd>>
{
    using base_type::base_type;
    using base_type::operator=;
};

struct ConceptPredicateState : x3::position_tagged
{
    std::string predicate_name;
};

struct ConceptPredicateGoal : x3::position_tagged
{
    std::string predicate_name;
};

struct ConceptAnd : x3::position_tagged
{
    Concept concept_left;
    Concept concept_right;
};

struct ConceptNonTerminal : x3::position_tagged
{
    std::string name;
};

struct ConceptChoice : x3::position_tagged, x3::variant<ConceptNonTerminal, Concept>
{
    using base_type::base_type;
    using base_type::operator=;
};

struct ConceptDerivationRule : x3::position_tagged
{
    ConceptNonTerminal non_terminal;
    std::vector<ConceptChoice> choices;
};

/**
 * Roles
 */

struct RoleNonTerminal;
struct RolePredicateState;
struct RolePredicateGoal;
struct RoleAnd;

struct Role :
    x3::position_tagged,
    x3::variant<x3::forward_ast<RoleNonTerminal>,  //
                x3::forward_ast<RolePredicateState>,
                x3::forward_ast<RolePredicateGoal>,
                x3::forward_ast<RoleAnd>>
{
    using base_type::base_type;
    using base_type::operator=;
};

struct RolePredicateState : x3::position_tagged
{
    std::string predicate_name;
};

struct RolePredicateGoal : x3::position_tagged
{
    std::string predicate_name;
};

struct RoleAnd : x3::position_tagged
{
    Role role_left;
    Role role_right;
};

struct RoleNonTerminal : x3::position_tagged
{
    std::string name;
};

struct RoleChoice : x3::position_tagged, x3::variant<RoleNonTerminal, Role>
{
    using base_type::base_type;
    using base_type::operator=;
};

struct RoleDerivationRule : x3::position_tagged
{
    RoleNonTerminal non_terminal;
    std::vector<RoleChoice> choices;
};

/**
 * Grammar
 */

struct DerivationRule : x3::position_tagged, x3::variant<ConceptDerivationRule, RoleDerivationRule>
{
    using base_type::base_type;
    using base_type::operator=;
};

struct Grammar : x3::position_tagged
{
    std::vector<DerivationRule> rules;
};

}

#endif
