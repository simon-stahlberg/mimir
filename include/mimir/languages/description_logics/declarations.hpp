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
#include "mimir/languages/description_logics/constructor_tag.hpp"

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
 * Common
 */

template<typename T, FeatureCategory... Ds>
using HanaContainer = boost::hana::map<boost::hana::pair<boost::hana::type<Ds>, T>...>;

template<template<typename> typename T, FeatureCategory... Ds>
using HanaMappedContainer = boost::hana::map<boost::hana::pair<boost::hana::type<Ds>, T<Ds>>...>;

/**
 * Denotations
 */

template<FeatureCategory D>
struct DenotationImpl;
template<FeatureCategory D>
using Denotation = const DenotationImpl<D>*;
template<FeatureCategory D>
using DenotationList = std::vector<Denotation<D>>;
template<FeatureCategory D>
using DenotationListSet = std::unordered_set<DenotationList<D>, loki::Hash<DenotationList<D>>, loki::EqualTo<DenotationList<D>>>;
using DenotationListSets = HanaMappedContainer<DenotationListSet, Concept, Role, Boolean, Numerical>;

/**
 * Constructors
 */

/* Constructor */
template<FeatureCategory D>
class IConstructor;
template<FeatureCategory D>
using Constructor = const IConstructor<D>*;
template<FeatureCategory D>
using ConstructorList = std::vector<Constructor<D>>;
using ConstructorLists = dl::HanaMappedContainer<ConstructorList, Concept, Role, Boolean, Numerical>;

/* Concrete concepts */
class ConceptTopImpl;
using ConceptTop = const ConceptTopImpl*;
class ConceptBotImpl;
using ConceptBot = const ConceptBotImpl*;
template<formalism::StaticOrFluentOrDerived P>
class ConceptAtomicStateImpl;
template<formalism::StaticOrFluentOrDerived P>
using ConceptAtomicState = const ConceptAtomicStateImpl<P>*;
template<formalism::StaticOrFluentOrDerived P>
class ConceptAtomicGoalImpl;
template<formalism::StaticOrFluentOrDerived P>
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
template<formalism::StaticOrFluentOrDerived P>
class RoleAtomicStateImpl;
template<formalism::StaticOrFluentOrDerived P>
using RoleAtomicState = const RoleAtomicStateImpl<P>*;
template<formalism::StaticOrFluentOrDerived P>
class RoleAtomicGoalImpl;
template<formalism::StaticOrFluentOrDerived P>
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
template<formalism::StaticOrFluentOrDerived P>
class BooleanAtomicStateImpl;
template<formalism::StaticOrFluentOrDerived P>
using BooleanAtomicState = const BooleanAtomicStateImpl<P>*;
template<DescriptionLogicCategory D>
class BooleanNonemptyImpl;
template<DescriptionLogicCategory D>
using BooleanNonempty = const BooleanNonemptyImpl<D>*;

/* Concrete Numerical */
template<DescriptionLogicCategory D>
class NumericalCountImpl;
template<DescriptionLogicCategory D>
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
template<dl::FeatureCategory D>
class DerivationRuleImpl;
template<dl::FeatureCategory D>
using DerivationRule = const DerivationRuleImpl<D>*;
template<dl::FeatureCategory D>
using DerivationRuleList = std::vector<DerivationRule<D>>;
template<dl::FeatureCategory... D>
using DerivationRuleLists = boost::hana::map<boost::hana::pair<boost::hana::type<D>, DerivationRuleList<D>>...>;
template<FeatureCategory D>
using DerivationRuleSet = std::unordered_set<DerivationRule<D>>;
using DerivationRuleSets = HanaMappedContainer<DerivationRuleSet, Concept, Role, Boolean, Numerical>;

/* Constructor */
template<dl::FeatureCategory D>
class IConstructor;
template<dl::FeatureCategory D>
using Constructor = const IConstructor<D>*;

/* NonTerminal */
template<dl::FeatureCategory D>
class NonTerminalImpl;
template<dl::FeatureCategory D>
using NonTerminal = const NonTerminalImpl<D>*;
template<dl::FeatureCategory D>
using OptionalNonTerminal = std::optional<NonTerminal<D>>;
using OptionalNonTerminals = HanaMappedContainer<OptionalNonTerminal, Concept, Role, Boolean, Numerical>;
template<dl::FeatureCategory D>
using NonTerminalSet = std::unordered_set<NonTerminal<D>>;

template<template<typename> typename Value, dl::FeatureCategory... Ds>
using NonTerminalMap = boost::hana::map<boost::hana::pair<boost::hana::type<Ds>, std::unordered_map<NonTerminal<Ds>, Value<Ds>>>...>;
using NonTerminalToDerivationRuleSets = NonTerminalMap<DerivationRuleSet, Concept, Role, Boolean, Numerical>;

template<typename Key, dl::FeatureCategory... Ds>
using ToNonTerminalMap =
    boost::hana::map<boost::hana::pair<boost::hana::type<Ds>, std::unordered_map<Key, NonTerminal<Ds>, loki::Hash<Key>, loki::EqualTo<Key>>>...>;

/* ConstructorOrNonTerminal */
template<dl::FeatureCategory D>
class ConstructorOrNonTerminalImpl;
template<dl::FeatureCategory D>
using ConstructorOrNonTerminal = const ConstructorOrNonTerminalImpl<D>*;
template<dl::FeatureCategory D>
using ConstructorOrNonTerminalList = std::vector<ConstructorOrNonTerminal<D>>;

/* Concrete concepts */
class ConceptTopImpl;
using ConceptTop = const ConceptTopImpl*;
class ConceptBotImpl;
using ConceptBot = const ConceptBotImpl*;
template<formalism::StaticOrFluentOrDerived P>
class ConceptAtomicStateImpl;
template<formalism::StaticOrFluentOrDerived P>
using ConceptAtomicState = const ConceptAtomicStateImpl<P>*;
template<formalism::StaticOrFluentOrDerived P>
class ConceptAtomicGoalImpl;
template<formalism::StaticOrFluentOrDerived P>
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
template<formalism::StaticOrFluentOrDerived P>
class RoleAtomicStateImpl;
template<formalism::StaticOrFluentOrDerived P>
using RoleAtomicState = const RoleAtomicStateImpl<P>*;
template<formalism::StaticOrFluentOrDerived P>
class RoleAtomicGoalImpl;
template<formalism::StaticOrFluentOrDerived P>
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
template<formalism::StaticOrFluentOrDerived P>
class BooleanAtomicStateImpl;
template<formalism::StaticOrFluentOrDerived P>
using BooleanAtomicState = const BooleanAtomicStateImpl<P>*;
template<DescriptionLogicCategory D>
class BooleanNonemptyImpl;
template<DescriptionLogicCategory D>
using BooleanNonempty = const BooleanNonemptyImpl<D>*;

/* Concrete Numerical */
template<DescriptionLogicCategory D>
class NumericalCountImpl;
template<DescriptionLogicCategory D>
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
template<dl::FeatureCategory D>
class DerivationRuleImpl;
template<dl::FeatureCategory D>
using DerivationRule = const DerivationRuleImpl<D>*;
template<dl::FeatureCategory D>
using DerivationRuleList = std::vector<DerivationRule<D>>;
using DerivationRuleLists = HanaMappedContainer<DerivationRuleList, Concept, Role, Boolean, Numerical>;

/* SubstitutionRule */
template<dl::FeatureCategory D>
class SubstitutionRuleImpl;
template<dl::FeatureCategory D>
using SubstitutionRule = const SubstitutionRuleImpl<D>*;
template<dl::FeatureCategory D>
using SubstitutionRuleList = std::vector<SubstitutionRule<D>>;
using SubstitutionRuleLists = HanaMappedContainer<SubstitutionRuleList, Concept, Role, Boolean, Numerical>;

/* NonTerminal */
template<dl::FeatureCategory D>
class NonTerminalImpl;
template<dl::FeatureCategory D>
using NonTerminal = const NonTerminalImpl<D>*;
template<dl::FeatureCategory D>
using OptionalNonTerminal = std::optional<NonTerminal<D>>;
using OptionalNonTerminals = HanaMappedContainer<OptionalNonTerminal, Concept, Role, Boolean, Numerical>;

template<template<typename> typename Value, dl::FeatureCategory... Ds>
using NonTerminalMap = boost::hana::map<boost::hana::pair<boost::hana::type<Ds>, std::unordered_map<NonTerminal<Ds>, Value<Ds>>>...>;
using NonTerminalToDerivationRuleLists = NonTerminalMap<DerivationRuleList, Concept, Role, Boolean, Numerical>;
using NonTerminalToSubstitutionRuleLists = NonTerminalMap<SubstitutionRuleList, Concept, Role, Boolean, Numerical>;

template<dl::FeatureCategory D>
using NonTerminalList = std::vector<NonTerminal<D>>;
template<dl::FeatureCategory D>
using NonTerminalSet = std::unordered_set<NonTerminal<D>>;

/**
 * Constructors
 */

/* Constructor */
template<dl::FeatureCategory D>
class IConstructor;
template<dl::FeatureCategory D>
using Constructor = const IConstructor<D>*;
template<dl::FeatureCategory D>
using ConstructorList = std::vector<Constructor<D>>;
template<template<typename> typename Value, dl::FeatureCategory... D>
using ConstructorMap = boost::hana::map<boost::hana::pair<boost::hana::type<D>, std::unordered_map<Constructor<D>, Value<D>>>...>;
template<dl::FeatureCategory D>
using ConstructorSet = std::unordered_set<Constructor<D>>;

/* Concrete concepts */
class ConceptTopImpl;
using ConceptTop = const ConceptTopImpl*;
class ConceptBotImpl;
using ConceptBot = const ConceptBotImpl*;
template<formalism::StaticOrFluentOrDerived P>
class ConceptAtomicStateImpl;
template<formalism::StaticOrFluentOrDerived P>
using ConceptAtomicState = const ConceptAtomicStateImpl<P>*;
template<formalism::StaticOrFluentOrDerived P>
class ConceptAtomicGoalImpl;
template<formalism::StaticOrFluentOrDerived P>
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
template<formalism::StaticOrFluentOrDerived P>
class RoleAtomicStateImpl;
template<formalism::StaticOrFluentOrDerived P>
using RoleAtomicState = const RoleAtomicStateImpl<P>*;
template<formalism::StaticOrFluentOrDerived P>
class RoleAtomicGoalImpl;
template<formalism::StaticOrFluentOrDerived P>
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
template<formalism::StaticOrFluentOrDerived P>
class BooleanAtomicStateImpl;
template<formalism::StaticOrFluentOrDerived P>
using BooleanAtomicState = const BooleanAtomicStateImpl<P>*;
template<DescriptionLogicCategory D>
class BooleanNonemptyImpl;
template<DescriptionLogicCategory D>
using BooleanNonempty = const BooleanNonemptyImpl<D>*;

/* Concrete Numerical */
template<DescriptionLogicCategory D>
class NumericalCountImpl;
template<DescriptionLogicCategory D>
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