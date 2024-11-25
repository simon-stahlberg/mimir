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

#include "mimir/formalism/predicate_tag.hpp"
#include "mimir/languages/description_logics/constructor_tag.hpp"

#include <boost/hana.hpp>
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

using ConstructorTagToSizeT =
    boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, std::size_t>, boost::hana::pair<boost::hana::type<Role>, std::size_t>>;

/**
 * Denotations
 */

template<ConstructorTag D>
struct DenotationImpl;
template<ConstructorTag D>
using Denotation = const DenotationImpl<D>*;

/**
 * Constructors
 */

/* Constructor */
template<ConstructorTag D>
class ConstructorImpl;
template<ConstructorTag D>
using Constructor = const ConstructorImpl<D>*;
template<ConstructorTag D>
using ConstructorList = std::vector<Constructor<D>>;

/* Concrete concepts */
class ConceptTopImpl;
using ConceptTop = const ConceptTopImpl*;
class ConceptBotImpl;
using ConceptBot = const ConceptBotImpl*;
template<PredicateTag P>
class ConceptAtomicStateImpl;
template<PredicateTag P>
using ConceptAtomicState = const ConceptAtomicStateImpl<P>*;
template<PredicateTag P>
class ConceptAtomicGoalImpl;
template<PredicateTag P>
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
template<PredicateTag P>
class RoleAtomicStateImpl;
template<PredicateTag P>
using RoleAtomicState = const RoleAtomicStateImpl<P>*;
template<PredicateTag P>
class RoleAtomicGoalImpl;
template<PredicateTag P>
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
template<dl::ConstructorTag D>
class ConstructorImpl;
template<dl::ConstructorTag D>
using Constructor = const ConstructorImpl<D>*;

/* NonTerminal */
template<dl::ConstructorTag D>
class NonTerminalImpl;
template<dl::ConstructorTag D>
using NonTerminal = const NonTerminalImpl<D>*;

/* ConstructorOrNonTerminal */
template<dl::ConstructorTag D>
class ConstructorOrNonTerminalImpl;
template<dl::ConstructorTag D>
using ConstructorOrNonTerminal = const ConstructorOrNonTerminalImpl<D>*;
template<dl::ConstructorTag D>
using ConstructorOrNonTerminalList = std::vector<ConstructorOrNonTerminal<D>>;

/* DerivationRule */
template<dl::ConstructorTag D>
class DerivationRuleImpl;
template<dl::ConstructorTag D>
using DerivationRule = const DerivationRuleImpl<D>*;
template<ConstructorTag D>
using DerivationRuleList = std::vector<DerivationRule<D>>;
using ConstructorTagToDerivationRuleList = boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, DerivationRuleList<Concept>>,
                                                            boost::hana::pair<boost::hana::type<Role>, DerivationRuleList<Role>>>;

/* Concrete concepts */
class ConceptTopImpl;
using ConceptTop = const ConceptTopImpl*;
class ConceptBotImpl;
using ConceptBot = const ConceptBotImpl*;
template<PredicateTag P>
class ConceptAtomicStateImpl;
template<PredicateTag P>
using ConceptAtomicState = const ConceptAtomicStateImpl<P>*;
template<PredicateTag P>
class ConceptAtomicGoalImpl;
template<PredicateTag P>
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
template<PredicateTag P>
class RoleAtomicStateImpl;
template<PredicateTag P>
using RoleAtomicState = const RoleAtomicStateImpl<P>*;
template<PredicateTag P>
class RoleAtomicGoalImpl;
template<PredicateTag P>
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

}
}

#endif