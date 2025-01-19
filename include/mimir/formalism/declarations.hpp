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

namespace mimir
{
/**
 * Forward declarations
 */

class ActionImpl;
using Action = const ActionImpl*;
using ActionList = std::vector<Action>;

template<PredicateTag P>
class AtomImpl;
template<PredicateTag P>
using Atom = const AtomImpl<P>*;
template<PredicateTag P>
using AtomList = std::vector<Atom<P>>;

class AxiomImpl;
using Axiom = const AxiomImpl*;
using AxiomList = std::vector<Axiom>;
using AxiomSet = std::unordered_set<Axiom>;

class DomainImpl;
using Domain = const DomainImpl*;
using DomainList = std::vector<Domain>;

template<DynamicFunctionTag F>
class EffectNumericImpl;
template<DynamicFunctionTag F>
using EffectNumeric = const EffectNumericImpl<F>*;
template<DynamicFunctionTag F>
using EffectNumericList = std::vector<EffectNumeric<F>>;

class EffectStripsImpl;
using EffectStrips = const EffectStripsImpl*;

class EffectConditionalImpl;
using EffectConditional = const EffectConditionalImpl*;
using EffectConditionalList = std::vector<EffectConditional>;

class FunctionExpressionNumberImpl;
using FunctionExpressionNumber = const FunctionExpressionNumberImpl*;
class FunctionExpressionBinaryOperatorImpl;
using FunctionExpressionBinaryOperator = const FunctionExpressionBinaryOperatorImpl*;
class FunctionExpressionMultiOperatorImpl;
using FunctionExpressionMultiOperator = const FunctionExpressionMultiOperatorImpl*;
class FunctionExpressionMinusImpl;
using FunctionExpressionMinus = const FunctionExpressionMinusImpl*;
template<FunctionTag F>
class FunctionExpressionFunctionImpl;
template<FunctionTag F>
using FunctionExpressionFunction = const FunctionExpressionFunctionImpl<F>*;
class FunctionExpressionImpl;
using FunctionExpression = const FunctionExpressionImpl*;
using FunctionExpressionList = std::vector<FunctionExpression>;

template<FunctionTag F>
class FunctionSkeletonImpl;
template<FunctionTag F>
using FunctionSkeleton = const FunctionSkeletonImpl<F>*;
template<FunctionTag F>
using FunctionSkeletonList = std::vector<FunctionSkeleton<F>>;

template<FunctionTag F>
class FunctionImpl;
template<FunctionTag F>
using Function = const FunctionImpl<F>*;
template<FunctionTag F>
using FunctionList = std::vector<Function<F>>;

template<PredicateTag P>
class GroundAtomImpl;
template<PredicateTag P>
using GroundAtom = const GroundAtomImpl<P>*;
template<PredicateTag P>
using GroundAtomList = std::vector<GroundAtom<P>>;
template<PredicateTag P>
using GroundAtomSet = std::unordered_set<GroundAtom<P>>;

class GroundFunctionExpressionNumberImpl;
using GroundFunctionExpressionNumber = const GroundFunctionExpressionNumberImpl*;
class GroundFunctionExpressionBinaryOperatorImpl;
using GroundFunctionExpressionBinaryOperator = const GroundFunctionExpressionBinaryOperatorImpl*;
class GroundFunctionExpressionMultiOperatorImpl;
using GroundFunctionExpressionMultiOperator = const GroundFunctionExpressionMultiOperatorImpl*;
class GroundFunctionExpressionMinusImpl;
using GroundFunctionExpressionMinus = const GroundFunctionExpressionMinusImpl*;
template<FunctionTag F>
class GroundFunctionExpressionFunctionImpl;
template<FunctionTag F>
using GroundFunctionExpressionFunction = const GroundFunctionExpressionFunctionImpl<F>*;
class GroundFunctionExpressionImpl;
using GroundFunctionExpression = const GroundFunctionExpressionImpl*;
using GroundFunctionExpressionList = std::vector<GroundFunctionExpression>;

template<FunctionTag F>
class GroundFunctionImpl;
template<FunctionTag F>
using GroundFunction = const GroundFunctionImpl<F>*;
template<FunctionTag F>
using GroundFunctionList = std::vector<GroundFunction<F>>;
template<FunctionTag F, typename T>
using GroundFunctionMap = std::unordered_map<GroundFunction<F>, T>;

template<PredicateTag P>
class GroundLiteralImpl;
template<PredicateTag P>
using GroundLiteral = const GroundLiteralImpl<P>*;
template<PredicateTag P>
using GroundLiteralList = std::vector<GroundLiteral<P>>;
template<PredicateTag P>
using GroundLiteralSet = std::unordered_set<GroundLiteral<P>>;

template<PredicateTag P>
class LiteralImpl;
template<PredicateTag P>
using Literal = const LiteralImpl<P>*;
template<PredicateTag P>
using LiteralList = std::vector<Literal<P>>;
template<PredicateTag P>
using LiteralSet = std::unordered_set<Literal<P>>;

class OptimizationMetricImpl;
using OptimizationMetric = const OptimizationMetricImpl*;

template<FunctionTag F>
class GroundFunctionValueImpl;
template<FunctionTag F>
using GroundFunctionValue = const GroundFunctionValueImpl<F>*;
template<FunctionTag F>
using GroundFunctionValueList = std::vector<GroundFunctionValue<F>>;

class NumericConstraintImpl;
using NumericConstraint = const NumericConstraintImpl*;
using NumericConstraintList = std::vector<NumericConstraint>;

class GroundNumericConstraintImpl;
using GroundNumericConstraint = const GroundNumericConstraintImpl*;
using GroundNumericConstraintList = std::vector<GroundNumericConstraint>;

class ObjectImpl;
using Object = const ObjectImpl*;
using ObjectList = std::vector<Object>;
template<typename Key, typename Hash = loki::Hash<Key>, typename KeyEqual = loki::EqualTo<Key>>
using ToObjectMap = std::unordered_map<Key, Object, Hash, KeyEqual>;

class PDDLRepositories;

template<PredicateTag P>
class PredicateImpl;
template<PredicateTag P>
using Predicate = const PredicateImpl<P>*;
template<PredicateTag P>
using PredicateList = std::vector<Predicate<P>>;
template<PredicateTag P>
using PredicateSet = std::unordered_set<Predicate<P>>;
template<typename Key, PredicateTag P, typename Hash = loki::Hash<Key>, typename KeyEqual = loki::EqualTo<Key>>
using ToPredicateMap = std::unordered_map<Key, Predicate<P>, Hash, KeyEqual>;

class ProblemImpl;
using Problem = const ProblemImpl*;
using ProblemList = std::vector<Problem>;

class RequirementsImpl;
using Requirements = const RequirementsImpl*;

class VariableImpl;
using Variable = const VariableImpl*;
using VariableList = std::vector<Variable>;
using VariableSet = std::unordered_set<Variable>;

class ExistentiallyQuantifiedConjunctiveConditionImpl;
using ExistentiallyQuantifiedConjunctiveCondition = const ExistentiallyQuantifiedConjunctiveConditionImpl*;

class TermImpl;
using Term = const TermImpl*;
using TermList = std::vector<Term>;

}

#endif
