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

#ifndef MIMIR_FORMALISM_DECLARATIONS_HPP_
#define MIMIR_FORMALISM_DECLARATIONS_HPP_

// Do not include headers with transitive dependencies.
#include "mimir/common/types.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/tags.hpp"

#include <cassert>
#include <cstdint>
#include <loki/loki.hpp>
#include <ostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace mimir::formalism
{
/**
 * Forward declarations
 */

template<typename... Ts>
using FlatBitsets = boost::hana::map<boost::hana::pair<boost::hana::type<Ts>, FlatBitset>...>;
template<typename... Ts>
using FlatIndexLists = boost::hana::map<boost::hana::pair<boost::hana::type<Ts>, FlatIndexList>...>;
template<typename... Ts>
using FlatDoubleLists = boost::hana::map<boost::hana::pair<boost::hana::type<Ts>, FlatDoubleList>...>;

template<IsStaticOrFluentOrDerivedTag P>
class AssignmentSet;
template<IsStaticOrFluentTag F>
class NumericAssignmentSet;

class ActionImpl;
using Action = const ActionImpl*;
using ActionList = std::vector<Action>;

class GroundActionImpl;
using GroundAction = const GroundActionImpl*;
using GroundActionList = std::vector<GroundAction>;
using GroundActionSet = std::unordered_set<GroundAction>;

class GeneralizedProblemImpl;
using GeneralizedProblem = std::shared_ptr<GeneralizedProblemImpl>;

template<IsStaticOrFluentOrDerivedTag P>
class AtomImpl;
template<IsStaticOrFluentOrDerivedTag P>
using Atom = const AtomImpl<P>*;
template<IsStaticOrFluentOrDerivedTag P>
using AtomList = std::vector<Atom<P>>;

class AxiomImpl;
using Axiom = const AxiomImpl*;
using AxiomList = std::vector<Axiom>;
using AxiomSet = std::unordered_set<Axiom>;

class GroundAxiomImpl;
using GroundAxiom = const GroundAxiomImpl*;
using GroundAxiomList = std::vector<GroundAxiom>;
using GroundAxiomSet = std::unordered_set<GroundAxiom>;

class DomainImpl;
using Domain = std::shared_ptr<const DomainImpl>;
using DomainList = std::vector<Domain>;
class DomainBuilder;

template<IsFluentOrAuxiliaryTag F>
class NumericEffectImpl;
template<IsFluentOrAuxiliaryTag F>
using NumericEffect = const NumericEffectImpl<F>*;
template<IsFluentOrAuxiliaryTag F>
using NumericEffectList = std::vector<NumericEffect<F>>;

template<IsFluentOrAuxiliaryTag F>
class GroundNumericEffectImpl;
template<IsFluentOrAuxiliaryTag F>
using GroundNumericEffect = const GroundNumericEffectImpl<F>*;

class ConjunctiveEffectImpl;
using ConjunctiveEffect = const ConjunctiveEffectImpl*;

class GroundConjunctiveEffect;

class ConditionalEffectImpl;
using ConditionalEffect = const ConditionalEffectImpl*;
using ConditionalEffectList = std::vector<ConditionalEffect>;

class GroundConditionalEffect;

class FunctionExpressionNumberImpl;
using FunctionExpressionNumber = const FunctionExpressionNumberImpl*;
class FunctionExpressionBinaryOperatorImpl;
using FunctionExpressionBinaryOperator = const FunctionExpressionBinaryOperatorImpl*;
class FunctionExpressionMultiOperatorImpl;
using FunctionExpressionMultiOperator = const FunctionExpressionMultiOperatorImpl*;
class FunctionExpressionMinusImpl;
using FunctionExpressionMinus = const FunctionExpressionMinusImpl*;
template<IsStaticOrFluentOrAuxiliaryTag F>
class FunctionExpressionFunctionImpl;
template<IsStaticOrFluentOrAuxiliaryTag F>
using FunctionExpressionFunction = const FunctionExpressionFunctionImpl<F>*;
class FunctionExpressionImpl;
using FunctionExpression = const FunctionExpressionImpl*;
using FunctionExpressionList = std::vector<FunctionExpression>;

template<IsStaticOrFluentOrAuxiliaryTag F>
class FunctionSkeletonImpl;
template<IsStaticOrFluentOrAuxiliaryTag F>
using FunctionSkeleton = const FunctionSkeletonImpl<F>*;
template<IsStaticOrFluentOrAuxiliaryTag F>
using FunctionSkeletonList = std::vector<FunctionSkeleton<F>>;
template<IsStaticOrFluentOrAuxiliaryTag... Fs>
using FunctionSkeletonLists = boost::hana::map<boost::hana::pair<boost::hana::type<Fs>, FunctionSkeletonList<Fs>>...>;

template<IsStaticOrFluentOrAuxiliaryTag F>
class FunctionImpl;
template<IsStaticOrFluentOrAuxiliaryTag F>
using Function = const FunctionImpl<F>*;
template<IsStaticOrFluentOrAuxiliaryTag F>
using FunctionList = std::vector<Function<F>>;

template<IsStaticOrFluentOrDerivedTag P>
class GroundAtomImpl;
template<IsStaticOrFluentOrDerivedTag P>
using GroundAtom = const GroundAtomImpl<P>*;
template<IsStaticOrFluentOrDerivedTag P>
using GroundAtomList = std::vector<GroundAtom<P>>;
template<IsStaticOrFluentOrDerivedTag... Ps>
using GroundAtomLists = boost::hana::map<boost::hana::pair<boost::hana::type<Ps>, GroundAtomList<Ps>>...>;
template<IsStaticOrFluentOrDerivedTag P>
using GroundAtomSet = std::unordered_set<GroundAtom<P>>;

class GroundFunctionExpressionNumberImpl;
using GroundFunctionExpressionNumber = const GroundFunctionExpressionNumberImpl*;
class GroundFunctionExpressionBinaryOperatorImpl;
using GroundFunctionExpressionBinaryOperator = const GroundFunctionExpressionBinaryOperatorImpl*;
class GroundFunctionExpressionMultiOperatorImpl;
using GroundFunctionExpressionMultiOperator = const GroundFunctionExpressionMultiOperatorImpl*;
class GroundFunctionExpressionMinusImpl;
using GroundFunctionExpressionMinus = const GroundFunctionExpressionMinusImpl*;
template<IsStaticOrFluentOrAuxiliaryTag F>
class GroundFunctionExpressionFunctionImpl;
template<IsStaticOrFluentOrAuxiliaryTag F>
using GroundFunctionExpressionFunction = const GroundFunctionExpressionFunctionImpl<F>*;
class GroundFunctionExpressionImpl;
using GroundFunctionExpression = const GroundFunctionExpressionImpl*;
using GroundFunctionExpressionList = std::vector<GroundFunctionExpression>;

template<IsStaticOrFluentOrAuxiliaryTag F>
class GroundFunctionImpl;
template<IsStaticOrFluentOrAuxiliaryTag F>
using GroundFunction = const GroundFunctionImpl<F>*;
template<IsStaticOrFluentOrAuxiliaryTag F>
using GroundFunctionList = std::vector<GroundFunction<F>>;
template<IsStaticOrFluentOrAuxiliaryTag F, typename T>
using GroundFunctionMap = std::unordered_map<GroundFunction<F>, T>;

template<IsStaticOrFluentOrDerivedTag P>
class GroundLiteralImpl;
template<IsStaticOrFluentOrDerivedTag P>
using GroundLiteral = const GroundLiteralImpl<P>*;
template<IsStaticOrFluentOrDerivedTag P>
using GroundLiteralList = std::vector<GroundLiteral<P>>;
template<IsStaticOrFluentOrDerivedTag... Ps>
using GroundLiteralLists = boost::hana::map<boost::hana::pair<boost::hana::type<Ps>, GroundLiteralList<Ps>>...>;
template<IsStaticOrFluentOrDerivedTag P>
using GroundLiteralSet = std::unordered_set<GroundLiteral<P>>;

template<IsStaticOrFluentOrDerivedTag P>
class LiteralImpl;
template<IsStaticOrFluentOrDerivedTag P>
using Literal = const LiteralImpl<P>*;
template<IsStaticOrFluentOrDerivedTag P>
using LiteralList = std::vector<Literal<P>>;
template<IsStaticOrFluentOrDerivedTag... Ps>
using LiteralLists = boost::hana::map<boost::hana::pair<boost::hana::type<Ps>, LiteralList<Ps>>...>;
template<IsStaticOrFluentOrDerivedTag P>
using LiteralSet = std::unordered_set<Literal<P>>;

class OptimizationMetricImpl;
using OptimizationMetric = const OptimizationMetricImpl*;

template<IsStaticOrFluentOrAuxiliaryTag F>
class GroundFunctionValueImpl;
template<IsStaticOrFluentOrAuxiliaryTag F>
using GroundFunctionValue = const GroundFunctionValueImpl<F>*;
template<IsStaticOrFluentOrAuxiliaryTag F>
using GroundFunctionValueList = std::vector<GroundFunctionValue<F>>;
template<IsStaticOrFluentTag... Fs>
using GroundFunctionValueLists = boost::hana::map<boost::hana::pair<boost::hana::type<Fs>, GroundFunctionValueList<Fs>>...>;

class NumericConstraintImpl;
using NumericConstraint = const NumericConstraintImpl*;
using NumericConstraintList = std::vector<NumericConstraint>;

class GroundNumericConstraintImpl;
using GroundNumericConstraint = const GroundNumericConstraintImpl*;
using GroundNumericConstraintList = std::vector<GroundNumericConstraint>;

class ObjectImpl;
using Object = const ObjectImpl*;
using ObjectList = std::vector<Object>;
using ObjectSet = std::unordered_set<Object>;
template<typename Key>
using ToObjectMap = std::unordered_map<Key, Object, loki::Hash<Key>, loki::EqualTo<Key>>;
template<typename Value>
using ObjectMap = std::unordered_map<Object, Value>;

class Repositories;

template<IsStaticOrFluentOrDerivedTag P>
class PredicateImpl;
template<IsStaticOrFluentOrDerivedTag P>
using Predicate = const PredicateImpl<P>*;
using PredicateVariant = std::variant<Predicate<StaticTag>, Predicate<FluentTag>, Predicate<DerivedTag>>;
using PredicateVariantList = std::vector<PredicateVariant>;
template<IsStaticOrFluentOrDerivedTag P>
using PredicateList = std::vector<Predicate<P>>;
template<IsStaticOrFluentOrDerivedTag... Ps>
using PredicateLists = boost::hana::map<boost::hana::pair<boost::hana::type<Ps>, PredicateList<Ps>>...>;
template<IsStaticOrFluentOrDerivedTag P>
using PredicateSet = std::unordered_set<Predicate<P>>;
template<typename Key, IsStaticOrFluentOrDerivedTag P>
using ToPredicateMap = std::unordered_map<Key, Predicate<P>, loki::Hash<Key>, loki::EqualTo<Key>>;
template<typename Key, IsStaticOrFluentOrDerivedTag... Ps>
using ToPredicateMaps = boost::hana::map<boost::hana::pair<boost::hana::type<Ps>, ToPredicateMap<Key, Ps>>...>;
template<typename Value, IsStaticOrFluentOrDerivedTag P>
using PredicateMap = std::unordered_map<Predicate<P>, Value>;
template<typename Value, IsStaticOrFluentOrDerivedTag... Ps>
using PredicateMaps = boost::hana::map<boost::hana::pair<boost::hana::type<Ps>, PredicateMap<Value, Ps>>...>;

class ProblemImpl;
using Problem = std::shared_ptr<ProblemImpl>;
using ProblemList = std::vector<Problem>;
class ProblemBuilder;
template<typename Value>
using ProblemMap = std::unordered_map<Problem, Value>;

class RequirementsImpl;
using Requirements = const RequirementsImpl*;

class VariableImpl;
using Variable = const VariableImpl*;
using VariableList = std::vector<Variable>;
using VariableSet = std::unordered_set<Variable>;

class ConjunctiveConditionImpl;
using ConjunctiveCondition = const ConjunctiveConditionImpl*;

class GroundConjunctiveCondition;

class TermImpl;
using Term = const TermImpl*;
using TermList = std::vector<Term>;

static_assert(IsHanaMap<PredicateLists<StaticTag>>);

}

#endif
