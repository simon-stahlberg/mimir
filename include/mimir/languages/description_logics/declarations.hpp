/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_DECLARATIONS_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_DECLARATIONS_HPP_

#include "mimir/formalism/tags.hpp"
#include "mimir/languages/description_logics/cnf_grammar_constructor_tag.hpp"
#include "mimir/languages/description_logics/constructor_tag.hpp"

#include <boost/hana.hpp>
#include <unordered_set>
#include <variant>
#include <vector>

namespace mimir::dl
{
template<typename T>
class ConstructorRepository;

class EvaluationContext;

/**
 * Common
 */

using ConceptOrRoleToSizeT =
    boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, std::size_t>, boost::hana::pair<boost::hana::type<Role>, std::size_t>>;

/**
 * Denotations
 */

template<ConceptOrRole D>
struct DenotationImpl;
template<ConceptOrRole D>
using Denotation = const DenotationImpl<D>*;

/**
 * Constructors
 */

/* Constructor */
template<ConceptOrRole D>
class ConstructorImpl;
template<ConceptOrRole D>
using Constructor = const ConstructorImpl<D>*;
template<ConceptOrRole D>
using ConstructorList = std::vector<Constructor<D>>;

/* Concrete concepts */
class ConceptTopImpl;
using ConceptTop = const ConceptTopImpl*;
class ConceptBotImpl;
using ConceptBot = const ConceptBotImpl*;
template<StaticOrFluentOrDerived P>
class ConceptAtomicStateImpl;
template<StaticOrFluentOrDerived P>
using ConceptAtomicState = const ConceptAtomicStateImpl<P>*;
template<StaticOrFluentOrDerived P>
class ConceptAtomicGoalImpl;
template<StaticOrFluentOrDerived P>
using ConceptAtomicGoal = const ConceptAtomicGoalImpl<P>*;
class ConceptIntersectionImpl;
using ConceptIntersection = const ConceptIntersectionImpl*;
class ConceptUnionImpl;
using ConceptUnion = const ConceptUnionImpl*;
class ConceptNegationImpl;
using ConceptNegation = const ConceptNegationImpl*;
class ConceptValueRestrictionImpl;
using ConceptValueRestriction = const ConceptValueRestrictionImpl*;
class ConceptExistentialQuantificationImpl;
using ConceptExistentialQuantification = const ConceptExistentialQuantificationImpl*;
class ConceptRoleValueMapContainmentImpl;
using ConceptRoleValueMapContainment = const ConceptRoleValueMapContainmentImpl*;
class ConceptRoleValueMapEqualityImpl;
using ConceptRoleValueMapEquality = const ConceptRoleValueMapEqualityImpl*;
class ConceptNominalImpl;
using ConceptNominal = const ConceptNominalImpl*;

/* Concrete roles */
class RoleUniversalImpl;
using RoleUniversal = const RoleUniversalImpl*;
template<StaticOrFluentOrDerived P>
class RoleAtomicStateImpl;
template<StaticOrFluentOrDerived P>
using RoleAtomicState = const RoleAtomicStateImpl<P>*;
template<StaticOrFluentOrDerived P>
class RoleAtomicGoalImpl;
template<StaticOrFluentOrDerived P>
using RoleAtomicGoal = const RoleAtomicGoalImpl<P>*;
class RoleIntersectionImpl;
using RoleIntersection = const RoleIntersectionImpl*;
class RoleUnionImpl;
using RoleUnion = const RoleUnionImpl*;
class RoleComplementImpl;
using RoleComplement = const RoleComplementImpl*;
class RoleInverseImpl;
using RoleInverse = const RoleInverseImpl*;
class RoleCompositionImpl;
using RoleComposition = const RoleCompositionImpl*;
class RoleTransitiveClosureImpl;
using RoleTransitiveClosure = const RoleTransitiveClosureImpl*;
class RoleReflexiveTransitiveClosureImpl;
using RoleReflexiveTransitiveClosure = const RoleReflexiveTransitiveClosureImpl*;
class RoleRestrictionImpl;
using RoleRestriction = const RoleRestrictionImpl*;
class RoleIdentityImpl;
using RoleIdentity = const RoleIdentityImpl*;

/**
 * Visitors
 */

class Visitor;

namespace grammar
{
/**
 * Constructors
 */

/* Constructor */
template<dl::ConceptOrRole D>
class ConstructorImpl;
template<dl::ConceptOrRole D>
using Constructor = const ConstructorImpl<D>*;

/* NonTerminal */
template<dl::ConceptOrRole D>
class NonTerminalImpl;
template<dl::ConceptOrRole D>
using NonTerminal = const NonTerminalImpl<D>*;

/* ConstructorOrNonTerminal */
template<dl::ConceptOrRole D>
class ConstructorOrNonTerminalImpl;
template<dl::ConceptOrRole D>
using ConstructorOrNonTerminal = const ConstructorOrNonTerminalImpl<D>*;
template<dl::ConceptOrRole D>
using ConstructorOrNonTerminalList = std::vector<ConstructorOrNonTerminal<D>>;

/* DerivationRule */
template<dl::ConceptOrRole D>
class DerivationRuleImpl;
template<dl::ConceptOrRole D>
using DerivationRule = const DerivationRuleImpl<D>*;
template<ConceptOrRole D>
using DerivationRuleList = std::vector<DerivationRule<D>>;
template<ConceptOrRole D>
using DerivationRuleSet = std::unordered_set<DerivationRule<D>>;
using ConceptOrRoleToDerivationRuleList = boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, DerivationRuleList<Concept>>,
                                                           boost::hana::pair<boost::hana::type<Role>, DerivationRuleList<Role>>>;

/* Concrete concepts */
class ConceptTopImpl;
using ConceptTop = const ConceptTopImpl*;
class ConceptBotImpl;
using ConceptBot = const ConceptBotImpl*;
template<StaticOrFluentOrDerived P>
class ConceptAtomicStateImpl;
template<StaticOrFluentOrDerived P>
using ConceptAtomicState = const ConceptAtomicStateImpl<P>*;
template<StaticOrFluentOrDerived P>
class ConceptAtomicGoalImpl;
template<StaticOrFluentOrDerived P>
using ConceptAtomicGoal = const ConceptAtomicGoalImpl<P>*;
class ConceptIntersectionImpl;
using ConceptIntersection = const ConceptIntersectionImpl*;
class ConceptUnionImpl;
using ConceptUnion = const ConceptUnionImpl*;
class ConceptNegationImpl;
using ConceptNegation = const ConceptNegationImpl*;
class ConceptValueRestrictionImpl;
using ConceptValueRestriction = const ConceptValueRestrictionImpl*;
class ConceptExistentialQuantificationImpl;
using ConceptExistentialQuantification = const ConceptExistentialQuantificationImpl*;
class ConceptRoleValueMapContainmentImpl;
using ConceptRoleValueMapContainment = const ConceptRoleValueMapContainmentImpl*;
class ConceptRoleValueMapEqualityImpl;
using ConceptRoleValueMapEquality = const ConceptRoleValueMapEqualityImpl*;
class ConceptNominalImpl;
using ConceptNominal = const ConceptNominalImpl*;

/* Concrete roles */
class RoleUniversalImpl;
using RoleUniversal = const RoleUniversalImpl*;
template<StaticOrFluentOrDerived P>
class RoleAtomicStateImpl;
template<StaticOrFluentOrDerived P>
using RoleAtomicState = const RoleAtomicStateImpl<P>*;
template<StaticOrFluentOrDerived P>
class RoleAtomicGoalImpl;
template<StaticOrFluentOrDerived P>
using RoleAtomicGoal = const RoleAtomicGoalImpl<P>*;
class RoleIntersectionImpl;
using RoleIntersection = const RoleIntersectionImpl*;
class RoleUnionImpl;
using RoleUnion = const RoleUnionImpl*;
class RoleComplementImpl;
using RoleComplement = const RoleComplementImpl*;
class RoleInverseImpl;
using RoleInverse = const RoleInverseImpl*;
class RoleCompositionImpl;
using RoleComposition = const RoleCompositionImpl*;
class RoleTransitiveClosureImpl;
using RoleTransitiveClosure = const RoleTransitiveClosureImpl*;
class RoleReflexiveTransitiveClosureImpl;
using RoleReflexiveTransitiveClosure = const RoleReflexiveTransitiveClosureImpl*;
class RoleRestrictionImpl;
using RoleRestriction = const RoleRestrictionImpl*;
class RoleIdentityImpl;
using RoleIdentity = const RoleIdentityImpl*;

/**
 * Visitors
 */

class Visitor;

/**
 * Grammar
 */

class Grammar;
}

namespace cnf_grammar
{
/**
 * Constructors
 */

/* Constructor */
template<dl::ConceptOrRole D, PrimitiveOrComposite C, Arity B = Undefined, Commutativity A = Undefined>
class ConstructorImpl;
template<dl::ConceptOrRole D, PrimitiveOrComposite C, Arity B = Undefined, Commutativity A = Undefined>
using Constructor = const ConstructorImpl<D>*;

/* NonTerminal */
template<dl::ConceptOrRole D>
class NonTerminalImpl;
template<dl::ConceptOrRole D>
using NonTerminal = const NonTerminalImpl<D>*;

/* DerivationRule */
template<dl::ConceptOrRole D, PrimitiveOrComposite C, Arity B, Commutativity A>
class DerivationRuleImpl;
template<dl::ConceptOrRole D, PrimitiveOrComposite C, Arity B, Commutativity A>
using DerivationRule = const DerivationRuleImpl<D>*;
template<dl::ConceptOrRole D, PrimitiveOrComposite C, Arity B, Commutativity A>
using DerivationRuleList = std::vector<DerivationRule<D>>;
template<dl::ConceptOrRole D, PrimitiveOrComposite C, Arity B, Commutativity A>
using DerivationRuleSet = std::unordered_set<DerivationRule<D>>;

/* Concrete concepts */
class ConceptTopImpl;
using ConceptTop = const ConceptTopImpl*;
class ConceptBotImpl;
using ConceptBot = const ConceptBotImpl*;
template<StaticOrFluentOrDerived P>
class ConceptAtomicStateImpl;
template<StaticOrFluentOrDerived P>
using ConceptAtomicState = const ConceptAtomicStateImpl<P>*;
template<StaticOrFluentOrDerived P>
class ConceptAtomicGoalImpl;
template<StaticOrFluentOrDerived P>
using ConceptAtomicGoal = const ConceptAtomicGoalImpl<P>*;
class ConceptIntersectionImpl;
using ConceptIntersection = const ConceptIntersectionImpl*;
class ConceptUnionImpl;
using ConceptUnion = const ConceptUnionImpl*;
class ConceptNegationImpl;
using ConceptNegation = const ConceptNegationImpl*;
class ConceptValueRestrictionImpl;
using ConceptValueRestriction = const ConceptValueRestrictionImpl*;
class ConceptExistentialQuantificationImpl;
using ConceptExistentialQuantification = const ConceptExistentialQuantificationImpl*;
class ConceptRoleValueMapContainmentImpl;
using ConceptRoleValueMapContainment = const ConceptRoleValueMapContainmentImpl*;
class ConceptRoleValueMapEqualityImpl;
using ConceptRoleValueMapEquality = const ConceptRoleValueMapEqualityImpl*;
class ConceptNominalImpl;
using ConceptNominal = const ConceptNominalImpl*;

/* Concrete roles */
class RoleUniversalImpl;
using RoleUniversal = const RoleUniversalImpl*;
template<StaticOrFluentOrDerived P>
class RoleAtomicStateImpl;
template<StaticOrFluentOrDerived P>
using RoleAtomicState = const RoleAtomicStateImpl<P>*;
template<StaticOrFluentOrDerived P>
class RoleAtomicGoalImpl;
template<StaticOrFluentOrDerived P>
using RoleAtomicGoal = const RoleAtomicGoalImpl<P>*;
class RoleIntersectionImpl;
using RoleIntersection = const RoleIntersectionImpl*;
class RoleUnionImpl;
using RoleUnion = const RoleUnionImpl*;
class RoleComplementImpl;
using RoleComplement = const RoleComplementImpl*;
class RoleInverseImpl;
using RoleInverse = const RoleInverseImpl*;
class RoleCompositionImpl;
using RoleComposition = const RoleCompositionImpl*;
class RoleTransitiveClosureImpl;
using RoleTransitiveClosure = const RoleTransitiveClosureImpl*;
class RoleReflexiveTransitiveClosureImpl;
using RoleReflexiveTransitiveClosure = const RoleReflexiveTransitiveClosureImpl*;
class RoleRestrictionImpl;
using RoleRestriction = const RoleRestrictionImpl*;
class RoleIdentityImpl;
using RoleIdentity = const RoleIdentityImpl*;

/**
 * Visitors
 */

class Visitor;

/**
 * Grammar
 */

class Grammar;
}
}

#endif