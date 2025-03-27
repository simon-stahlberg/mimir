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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_PARSER_AST_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_PARSER_AST_HPP_

#include "mimir/languages/description_logics/declarations.hpp"

#include <boost/optional.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <map>
#include <sstream>
#include <vector>

namespace mimir::languages::dl::ast
{
///////////////////////////////////////////////////////////////////////////
//  The AST
///////////////////////////////////////////////////////////////////////////
namespace x3 = boost::spirit::x3;

struct ConceptBot;
struct ConceptTop;
struct ConceptAtomicState;
struct ConceptAtomicGoal;
struct ConceptIntersection;
struct ConceptUnion;
struct ConceptNegation;
struct ConceptValueRestriction;
struct ConceptExistentialQuantification;
struct ConceptRoleValueMapContainment;
struct ConceptRoleValueMapEquality;
struct ConceptNominal;

struct RoleUniversal;
struct RoleAtomicState;
struct RoleAtomicGoal;
struct RoleIntersection;
struct RoleUnion;
struct RoleComplement;
struct RoleInverse;
struct RoleComposition;
struct RoleTransitiveClosure;
struct RoleReflexiveTransitiveClosure;
struct RoleRestriction;
struct RoleIdentity;

struct BooleanAtomicState;
struct BooleanNonempty;

struct NumericalCount;
struct NumericalDistance;

struct FeatureCategoryDerivationRule;

struct GrammarHead;
struct GrammarBody;
struct Grammar;

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
struct Constructor
{
};

template<>
struct Constructor<ConceptTag> :
    x3::position_tagged,
    x3::variant<x3::forward_ast<ConceptBot>,
                x3::forward_ast<ConceptTop>,
                x3::forward_ast<ConceptAtomicState>,
                x3::forward_ast<ConceptAtomicGoal>,
                x3::forward_ast<ConceptIntersection>,
                x3::forward_ast<ConceptUnion>,
                x3::forward_ast<ConceptNegation>,
                x3::forward_ast<ConceptValueRestriction>,
                x3::forward_ast<ConceptExistentialQuantification>,
                x3::forward_ast<ConceptRoleValueMapContainment>,
                x3::forward_ast<ConceptRoleValueMapEquality>,
                x3::forward_ast<ConceptNominal>>
{
    using base_type::base_type;
    using base_type::operator=;
};

template<>
struct Constructor<RoleTag> :
    x3::position_tagged,
    x3::variant<x3::forward_ast<RoleUniversal>,
                x3::forward_ast<RoleAtomicState>,
                x3::forward_ast<RoleAtomicGoal>,
                x3::forward_ast<RoleIntersection>,
                x3::forward_ast<RoleUnion>,
                x3::forward_ast<RoleComplement>,
                x3::forward_ast<RoleInverse>,
                x3::forward_ast<RoleComposition>,
                x3::forward_ast<RoleTransitiveClosure>,
                x3::forward_ast<RoleReflexiveTransitiveClosure>,
                x3::forward_ast<RoleRestriction>,
                x3::forward_ast<RoleIdentity>>
{
    using base_type::base_type;
    using base_type::operator=;
};

template<>
struct Constructor<BooleanTag> : x3::position_tagged, x3::variant<x3::forward_ast<BooleanAtomicState>, x3::forward_ast<BooleanNonempty>>
{
    using base_type::base_type;
    using base_type::operator=;
};

template<>
struct Constructor<NumericalTag> : x3::position_tagged, x3::variant<x3::forward_ast<NumericalCount>, x3::forward_ast<NumericalDistance>>
{
    using base_type::base_type;
    using base_type::operator=;
};

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
struct NonTerminal : x3::position_tagged
{
    std::string name;
};

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
struct ConstructorOrNonTerminal : x3::position_tagged, x3::variant<NonTerminal<D>, Constructor<D>>
{
    using typename x3::variant<NonTerminal<D>, Constructor<D>>::base_type;
    using base_type::base_type;
    using base_type::operator=;
};

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
struct DerivationRule : x3::position_tagged
{
    NonTerminal<D> non_terminal;
    std::vector<ConstructorOrNonTerminal<D>> constructor_or_non_terminals;
};

/**
 * Concepts
 */

struct ConceptBot : x3::position_tagged
{
};

struct ConceptTop : x3::position_tagged
{
};

struct ConceptAtomicState : x3::position_tagged
{
    std::string predicate_name;
};

struct ConceptAtomicGoal : x3::position_tagged
{
    std::string predicate_name;
    bool polarity;
};

struct ConceptIntersection : x3::position_tagged
{
    ConstructorOrNonTerminal<ConceptTag> left_concept_or_non_terminal;
    ConstructorOrNonTerminal<ConceptTag> right_concept_or_non_terminal;
};

struct ConceptUnion : x3::position_tagged
{
    ConstructorOrNonTerminal<ConceptTag> left_concept_or_non_terminal;
    ConstructorOrNonTerminal<ConceptTag> right_concept_or_non_terminal;
};

struct ConceptNegation : x3::position_tagged
{
    ConstructorOrNonTerminal<ConceptTag> concept_or_non_terminal;
};

struct ConceptValueRestriction : x3::position_tagged
{
    ConstructorOrNonTerminal<RoleTag> role_or_non_terminal;
    ConstructorOrNonTerminal<ConceptTag> concept_or_non_terminal;
};

struct ConceptExistentialQuantification : x3::position_tagged
{
    ConstructorOrNonTerminal<RoleTag> role_or_non_terminal;
    ConstructorOrNonTerminal<ConceptTag> concept_or_non_terminal;
};

struct ConceptRoleValueMapContainment : x3::position_tagged
{
    ConstructorOrNonTerminal<RoleTag> left_role_or_non_terminal;
    ConstructorOrNonTerminal<RoleTag> right_role_or_non_terminal;
};

struct ConceptRoleValueMapEquality : x3::position_tagged
{
    ConstructorOrNonTerminal<RoleTag> left_role_or_non_terminal;
    ConstructorOrNonTerminal<RoleTag> right_role_or_non_terminal;
};

struct ConceptNominal : x3::position_tagged
{
    std::string object_name;
};

/**
 * Roles
 */

struct RoleUniversal : x3::position_tagged
{
};

struct RoleAtomicState : x3::position_tagged
{
    std::string predicate_name;
};

struct RoleAtomicGoal : x3::position_tagged
{
    std::string predicate_name;
    bool polarity;
};

struct RoleIntersection : x3::position_tagged
{
    ConstructorOrNonTerminal<RoleTag> left_role_or_non_terminal;
    ConstructorOrNonTerminal<RoleTag> right_role_or_non_terminal;
};

struct RoleUnion : x3::position_tagged
{
    ConstructorOrNonTerminal<RoleTag> left_role_or_non_terminal;
    ConstructorOrNonTerminal<RoleTag> right_role_or_non_terminal;
};

struct RoleComplement : x3::position_tagged
{
    ConstructorOrNonTerminal<RoleTag> role_or_non_terminal;
};

struct RoleInverse : x3::position_tagged
{
    ConstructorOrNonTerminal<RoleTag> role_or_non_terminal;
};

struct RoleComposition : x3::position_tagged
{
    ConstructorOrNonTerminal<RoleTag> left_role_or_non_terminal;
    ConstructorOrNonTerminal<RoleTag> right_role_or_non_terminal;
};

struct RoleTransitiveClosure : x3::position_tagged
{
    ConstructorOrNonTerminal<RoleTag> role_or_non_terminal;
};

struct RoleReflexiveTransitiveClosure : x3::position_tagged
{
    ConstructorOrNonTerminal<RoleTag> role_or_non_terminal;
};

struct RoleRestriction : x3::position_tagged
{
    ConstructorOrNonTerminal<RoleTag> role_or_non_terminal;
    ConstructorOrNonTerminal<ConceptTag> concept_or_non_terminal;
};

struct RoleIdentity : x3::position_tagged
{
    ConstructorOrNonTerminal<ConceptTag> concept_or_non_terminal;
};

/**
 * ConceptOrRole
 */

struct ConceptOrRoleNonterminal : x3::variant<ConstructorOrNonTerminal<ConceptTag>, ConstructorOrNonTerminal<RoleTag>>
{
    using base_type::base_type;
    using base_type::operator=;
};

/**
 * Booleans
 */

struct BooleanAtomicState : x3::position_tagged
{
    std::string predicate_name;
};
struct BooleanNonempty : x3::position_tagged
{
    ConceptOrRoleNonterminal concept_or_role_nonterminal;
};

/**
 * Numericals
 */

struct NumericalCount : x3::position_tagged
{
    ConceptOrRoleNonterminal concept_or_role_nonterminal;
};
struct NumericalDistance : x3::position_tagged
{
    ConstructorOrNonTerminal<ConceptTag> left_concept_or_nonterminal;
    ConstructorOrNonTerminal<RoleTag> role_or_nonterminal;
    ConstructorOrNonTerminal<ConceptTag> right_concept_or_nonterminal;
};

/**
 * Grammar
 */

struct FeatureCategoryDerivationRule :
    x3::position_tagged,
    x3::variant<DerivationRule<ConceptTag>, DerivationRule<RoleTag>, DerivationRule<BooleanTag>, DerivationRule<NumericalTag>>
{
    using base_type::base_type;
    using base_type::operator=;
};

struct GrammarHead : x3::position_tagged
{
    boost::optional<NonTerminal<ConceptTag>> concept_start;
    boost::optional<NonTerminal<RoleTag>> role_start;
    boost::optional<NonTerminal<BooleanTag>> boolean_start;
    boost::optional<NonTerminal<NumericalTag>> numerical_start;
};

struct GrammarBody : x3::position_tagged
{
    std::vector<FeatureCategoryDerivationRule> rules;
};

struct Grammar : x3::position_tagged
{
    GrammarHead head;
    GrammarBody body;
};

}

#endif
