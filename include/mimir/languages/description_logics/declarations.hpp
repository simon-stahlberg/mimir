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

#include "mimir/common/types.hpp"
#include "mimir/formalism/tags.hpp"
#include "mimir/languages/description_logics/tags.hpp"

#include <boost/hana.hpp>
#include <loki/loki.hpp>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace mimir::languages::dl
{
class Repositories;

class EvaluationContext;

/**
 * Denotations
 */

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
struct DenotationImpl;
template<IsConceptOrRoleOrBooleanOrNumericalTag D>
using Denotation = const DenotationImpl<D>*;
template<IsConceptOrRoleOrBooleanOrNumericalTag D>
using DenotationList = std::vector<Denotation<D>>;
template<IsConceptOrRoleOrBooleanOrNumericalTag D>
using DenotationListSet = std::unordered_set<DenotationList<D>, loki::Hash<DenotationList<D>>, loki::EqualTo<DenotationList<D>>>;
using DenotationListSets = HanaMappedContainer<DenotationListSet, ConceptTag, RoleTag, BooleanTag, NumericalTag>;

/**
 * Constructors
 */

/* Constructor */
template<IsConceptOrRoleOrBooleanOrNumericalTag D>
class IConstructor;
template<IsConceptOrRoleOrBooleanOrNumericalTag D>
using Constructor = const IConstructor<D>*;
template<IsConceptOrRoleOrBooleanOrNumericalTag D>
using ConstructorList = std::vector<Constructor<D>>;
using ConstructorLists = HanaMappedContainer<ConstructorList, ConceptTag, RoleTag, BooleanTag, NumericalTag>;

/* Concrete concepts */
class ConceptTopImpl;
using ConceptTop = const ConceptTopImpl*;
class ConceptBotImpl;
using ConceptBot = const ConceptBotImpl*;
template<formalism::IsStaticOrFluentOrDerivedTag P>
class ConceptAtomicStateImpl;
template<formalism::IsStaticOrFluentOrDerivedTag P>
using ConceptAtomicState = const ConceptAtomicStateImpl<P>*;
template<formalism::IsStaticOrFluentOrDerivedTag P>
class ConceptAtomicGoalImpl;
template<formalism::IsStaticOrFluentOrDerivedTag P>
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
template<formalism::IsStaticOrFluentOrDerivedTag P>
class RoleAtomicStateImpl;
template<formalism::IsStaticOrFluentOrDerivedTag P>
using RoleAtomicState = const RoleAtomicStateImpl<P>*;
template<formalism::IsStaticOrFluentOrDerivedTag P>
class RoleAtomicGoalImpl;
template<formalism::IsStaticOrFluentOrDerivedTag P>
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

/* Concrete Booleans */
template<formalism::IsStaticOrFluentOrDerivedTag P>
class BooleanAtomicStateImpl;
template<formalism::IsStaticOrFluentOrDerivedTag P>
using BooleanAtomicState = const BooleanAtomicStateImpl<P>*;
template<IsConceptOrRoleTag D>
class BooleanNonemptyImpl;
template<IsConceptOrRoleTag D>
using BooleanNonempty = const BooleanNonemptyImpl<D>*;

/* Concrete Numerical */
template<IsConceptOrRoleTag D>
class NumericalCountImpl;
template<IsConceptOrRoleTag D>
using NumericalCount = const NumericalCountImpl<D>*;
class NumericalDistanceImpl;
using NumericalDistance = const NumericalDistanceImpl*;

/**
 * Visitors
 */

class IVisitor;

class GrammarVisitor;

namespace grammar
{

/* DerivationRule */
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
class DerivationRuleImpl;
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
using DerivationRule = const DerivationRuleImpl<D>*;
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
using DerivationRuleList = std::vector<DerivationRule<D>>;
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag... D>
using DerivationRuleLists = boost::hana::map<boost::hana::pair<boost::hana::type<D>, DerivationRuleList<D>>...>;
template<IsConceptOrRoleOrBooleanOrNumericalTag D>
using DerivationRuleSet = std::unordered_set<DerivationRule<D>>;
using DerivationRuleSets = HanaMappedContainer<DerivationRuleSet, ConceptTag, RoleTag, BooleanTag, NumericalTag>;

/* Constructor */
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
class IConstructor;
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
using Constructor = const IConstructor<D>*;

/* NonTerminal */
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
class NonTerminalImpl;
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
using NonTerminal = const NonTerminalImpl<D>*;
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
using OptionalNonTerminal = std::optional<NonTerminal<D>>;
using OptionalNonTerminals = HanaMappedContainer<OptionalNonTerminal, ConceptTag, RoleTag, BooleanTag, NumericalTag>;
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
using NonTerminalSet = std::unordered_set<NonTerminal<D>>;

template<template<typename> typename Value, dl::IsConceptOrRoleOrBooleanOrNumericalTag... Ds>
using NonTerminalMaps = boost::hana::map<boost::hana::pair<boost::hana::type<Ds>, std::unordered_map<NonTerminal<Ds>, Value<Ds>>>...>;
using NonTerminalToDerivationRuleSets = NonTerminalMaps<DerivationRuleSet, ConceptTag, RoleTag, BooleanTag, NumericalTag>;
template<typename Key, dl::IsConceptOrRoleOrBooleanOrNumericalTag... Ds>
using ToNonTerminalMaps =
    boost::hana::map<boost::hana::pair<boost::hana::type<Ds>, std::unordered_map<Key, NonTerminal<Ds>, loki::Hash<Key>, loki::EqualTo<Key>>>...>;
using StringToNonTerminalMaps = ToNonTerminalMaps<std::string, ConceptTag, RoleTag, BooleanTag, NumericalTag>;

/* ConstructorOrNonTerminal */
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
class ConstructorOrNonTerminalImpl;
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
using ConstructorOrNonTerminal = const ConstructorOrNonTerminalImpl<D>*;
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
using ConstructorOrNonTerminalList = std::vector<ConstructorOrNonTerminal<D>>;

/* Concrete concepts */
class ConceptTopImpl;
using ConceptTop = const ConceptTopImpl*;
class ConceptBotImpl;
using ConceptBot = const ConceptBotImpl*;
template<formalism::IsStaticOrFluentOrDerivedTag P>
class ConceptAtomicStateImpl;
template<formalism::IsStaticOrFluentOrDerivedTag P>
using ConceptAtomicState = const ConceptAtomicStateImpl<P>*;
template<formalism::IsStaticOrFluentOrDerivedTag P>
class ConceptAtomicGoalImpl;
template<formalism::IsStaticOrFluentOrDerivedTag P>
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
template<formalism::IsStaticOrFluentOrDerivedTag P>
class RoleAtomicStateImpl;
template<formalism::IsStaticOrFluentOrDerivedTag P>
using RoleAtomicState = const RoleAtomicStateImpl<P>*;
template<formalism::IsStaticOrFluentOrDerivedTag P>
class RoleAtomicGoalImpl;
template<formalism::IsStaticOrFluentOrDerivedTag P>
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

/* Concrete Booleans */
template<formalism::IsStaticOrFluentOrDerivedTag P>
class BooleanAtomicStateImpl;
template<formalism::IsStaticOrFluentOrDerivedTag P>
using BooleanAtomicState = const BooleanAtomicStateImpl<P>*;
template<IsConceptOrRoleTag D>
class BooleanNonemptyImpl;
template<IsConceptOrRoleTag D>
using BooleanNonempty = const BooleanNonemptyImpl<D>*;

/* Concrete Numerical */
template<IsConceptOrRoleTag D>
class NumericalCountImpl;
template<IsConceptOrRoleTag D>
using NumericalCount = const NumericalCountImpl<D>*;
class NumericalDistanceImpl;
using NumericalDistance = const NumericalDistanceImpl*;

/**
 * Visitors
 */

class IVisitor;

/**
 * Grammar
 */

class StartSymbolsContainer;
class DerivationRulesContainer;
class Grammar;

/**
 * Repositories
 */

class Repositories;
}

namespace cnf_grammar
{
/* DerivationRule */
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
class DerivationRuleImpl;
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
using DerivationRule = const DerivationRuleImpl<D>*;
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
using DerivationRuleList = std::vector<DerivationRule<D>>;
using DerivationRuleLists = HanaMappedContainer<DerivationRuleList, ConceptTag, RoleTag, BooleanTag, NumericalTag>;

/* SubstitutionRule */
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
class SubstitutionRuleImpl;
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
using SubstitutionRule = const SubstitutionRuleImpl<D>*;
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
using SubstitutionRuleList = std::vector<SubstitutionRule<D>>;
using SubstitutionRuleLists = HanaMappedContainer<SubstitutionRuleList, ConceptTag, RoleTag, BooleanTag, NumericalTag>;

/* NonTerminal */
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
class NonTerminalImpl;
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
using NonTerminal = const NonTerminalImpl<D>*;
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
using OptionalNonTerminal = std::optional<NonTerminal<D>>;
using OptionalNonTerminals = HanaMappedContainer<OptionalNonTerminal, ConceptTag, RoleTag, BooleanTag, NumericalTag>;
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
using NonTerminalList = std::vector<NonTerminal<D>>;
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
using NonTerminalSet = std::unordered_set<NonTerminal<D>>;
template<template<typename> typename Value, dl::IsConceptOrRoleOrBooleanOrNumericalTag... Ds>
using NonTerminalMaps = boost::hana::map<boost::hana::pair<boost::hana::type<Ds>, std::unordered_map<NonTerminal<Ds>, Value<Ds>>>...>;
using NonTerminalToNonTerminalMaps = NonTerminalMaps<NonTerminal, ConceptTag, RoleTag, BooleanTag, NumericalTag>;
using NonTerminalToDerivationRuleListMaps = NonTerminalMaps<DerivationRuleList, ConceptTag, RoleTag, BooleanTag, NumericalTag>;
using NonTerminalToSubstitutionRuleListMaps = NonTerminalMaps<SubstitutionRuleList, ConceptTag, RoleTag, BooleanTag, NumericalTag>;

/**
 * Constructors
 */

/* Constructor */
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
class IConstructor;
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
using Constructor = const IConstructor<D>*;
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
using ConstructorList = std::vector<Constructor<D>>;
template<template<typename> typename Value, dl::IsConceptOrRoleOrBooleanOrNumericalTag... D>
using ConstructorMaps = boost::hana::map<boost::hana::pair<boost::hana::type<D>, std::unordered_map<Constructor<D>, Value<D>>>...>;
using ConstructorToNonTerminalListMaps = ConstructorMaps<NonTerminalList, ConceptTag, RoleTag, BooleanTag, NumericalTag>;

// ConstructorMap<NonTerminalList, Concept, Role, Boolean, Numerical>

/* Concrete concepts */
class ConceptTopImpl;
using ConceptTop = const ConceptTopImpl*;
class ConceptBotImpl;
using ConceptBot = const ConceptBotImpl*;
template<formalism::IsStaticOrFluentOrDerivedTag P>
class ConceptAtomicStateImpl;
template<formalism::IsStaticOrFluentOrDerivedTag P>
using ConceptAtomicState = const ConceptAtomicStateImpl<P>*;
template<formalism::IsStaticOrFluentOrDerivedTag P>
class ConceptAtomicGoalImpl;
template<formalism::IsStaticOrFluentOrDerivedTag P>
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
template<formalism::IsStaticOrFluentOrDerivedTag P>
class RoleAtomicStateImpl;
template<formalism::IsStaticOrFluentOrDerivedTag P>
using RoleAtomicState = const RoleAtomicStateImpl<P>*;
template<formalism::IsStaticOrFluentOrDerivedTag P>
class RoleAtomicGoalImpl;
template<formalism::IsStaticOrFluentOrDerivedTag P>
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

/* Concrete Booleans */
template<formalism::IsStaticOrFluentOrDerivedTag P>
class BooleanAtomicStateImpl;
template<formalism::IsStaticOrFluentOrDerivedTag P>
using BooleanAtomicState = const BooleanAtomicStateImpl<P>*;
template<IsConceptOrRoleTag D>
class BooleanNonemptyImpl;
template<IsConceptOrRoleTag D>
using BooleanNonempty = const BooleanNonemptyImpl<D>*;

/* Concrete Numerical */
template<IsConceptOrRoleTag D>
class NumericalCountImpl;
template<IsConceptOrRoleTag D>
using NumericalCount = const NumericalCountImpl<D>*;
class NumericalDistanceImpl;
using NumericalDistance = const NumericalDistanceImpl*;

/**
 * Visitors
 */

class IVisitor;

/**
 * Grammar
 */

class Grammar;

/**
 * Repositories
 */

class Repositories;
}
}

#endif