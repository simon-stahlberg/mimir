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

class ProblemContext;

template<StaticOrFluentOrDerived P>
class AssignmentSet;
template<StaticOrFluentTag F>
class NumericAssignmentSet;

class ActionImpl;
using Action = const ActionImpl*;
using ActionList = std::vector<Action>;

class GroundActionImpl;
using GroundAction = const GroundActionImpl*;
using GroundActionList = std::vector<GroundAction>;
using GroundActionSet = std::unordered_set<GroundAction>;

template<StaticOrFluentOrDerived P>
class AtomImpl;
template<StaticOrFluentOrDerived P>
using Atom = const AtomImpl<P>*;
template<StaticOrFluentOrDerived P>
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
using Domain = const DomainImpl*;
using DomainList = std::vector<Domain>;

template<FluentOrAuxiliary F>
class NumericEffectImpl;
template<FluentOrAuxiliary F>
using NumericEffect = const NumericEffectImpl<F>*;
template<FluentOrAuxiliary F>
using NumericEffectList = std::vector<NumericEffect<F>>;

template<FluentOrAuxiliary F>
class GroundNumericEffectImpl;
template<FluentOrAuxiliary F>
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
template<StaticOrFluentOrAuxiliary F>
class FunctionExpressionFunctionImpl;
template<StaticOrFluentOrAuxiliary F>
using FunctionExpressionFunction = const FunctionExpressionFunctionImpl<F>*;
class FunctionExpressionImpl;
using FunctionExpression = const FunctionExpressionImpl*;
using FunctionExpressionList = std::vector<FunctionExpression>;

template<StaticOrFluentOrAuxiliary F>
class FunctionSkeletonImpl;
template<StaticOrFluentOrAuxiliary F>
using FunctionSkeleton = const FunctionSkeletonImpl<F>*;
template<StaticOrFluentOrAuxiliary F>
using FunctionSkeletonList = std::vector<FunctionSkeleton<F>>;

template<StaticOrFluentOrAuxiliary F>
class FunctionImpl;
template<StaticOrFluentOrAuxiliary F>
using Function = const FunctionImpl<F>*;
template<StaticOrFluentOrAuxiliary F>
using FunctionList = std::vector<Function<F>>;

template<StaticOrFluentOrDerived P>
class GroundAtomImpl;
template<StaticOrFluentOrDerived P>
using GroundAtom = const GroundAtomImpl<P>*;
template<StaticOrFluentOrDerived P>
using GroundAtomList = std::vector<GroundAtom<P>>;
template<StaticOrFluentOrDerived P>
using GroundAtomSet = std::unordered_set<GroundAtom<P>>;

class GroundFunctionExpressionNumberImpl;
using GroundFunctionExpressionNumber = const GroundFunctionExpressionNumberImpl*;
class GroundFunctionExpressionBinaryOperatorImpl;
using GroundFunctionExpressionBinaryOperator = const GroundFunctionExpressionBinaryOperatorImpl*;
class GroundFunctionExpressionMultiOperatorImpl;
using GroundFunctionExpressionMultiOperator = const GroundFunctionExpressionMultiOperatorImpl*;
class GroundFunctionExpressionMinusImpl;
using GroundFunctionExpressionMinus = const GroundFunctionExpressionMinusImpl*;
template<StaticOrFluentOrAuxiliary F>
class GroundFunctionExpressionFunctionImpl;
template<StaticOrFluentOrAuxiliary F>
using GroundFunctionExpressionFunction = const GroundFunctionExpressionFunctionImpl<F>*;
class GroundFunctionExpressionImpl;
using GroundFunctionExpression = const GroundFunctionExpressionImpl*;
using GroundFunctionExpressionList = std::vector<GroundFunctionExpression>;

template<StaticOrFluentOrAuxiliary F>
class GroundFunctionImpl;
template<StaticOrFluentOrAuxiliary F>
using GroundFunction = const GroundFunctionImpl<F>*;
template<StaticOrFluentOrAuxiliary F>
using GroundFunctionList = std::vector<GroundFunction<F>>;
template<StaticOrFluentOrAuxiliary F, typename T>
using GroundFunctionMap = std::unordered_map<GroundFunction<F>, T>;

template<StaticOrFluentOrDerived P>
class GroundLiteralImpl;
template<StaticOrFluentOrDerived P>
using GroundLiteral = const GroundLiteralImpl<P>*;
template<StaticOrFluentOrDerived P>
using GroundLiteralList = std::vector<GroundLiteral<P>>;
template<StaticOrFluentOrDerived P>
using GroundLiteralSet = std::unordered_set<GroundLiteral<P>>;

template<StaticOrFluentOrDerived P>
class LiteralImpl;
template<StaticOrFluentOrDerived P>
using Literal = const LiteralImpl<P>*;
template<StaticOrFluentOrDerived P>
using LiteralList = std::vector<Literal<P>>;
template<StaticOrFluentOrDerived P>
using LiteralSet = std::unordered_set<Literal<P>>;

class OptimizationMetricImpl;
using OptimizationMetric = const OptimizationMetricImpl*;

template<StaticOrFluentOrAuxiliary F>
class GroundFunctionValueImpl;
template<StaticOrFluentOrAuxiliary F>
using GroundFunctionValue = const GroundFunctionValueImpl<F>*;
template<StaticOrFluentOrAuxiliary F>
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

template<StaticOrFluentOrDerived P>
class PredicateImpl;
template<StaticOrFluentOrDerived P>
using Predicate = const PredicateImpl<P>*;
template<StaticOrFluentOrDerived P>
using PredicateList = std::vector<Predicate<P>>;
template<StaticOrFluentOrDerived P>
using PredicateSet = std::unordered_set<Predicate<P>>;
template<typename Key, StaticOrFluentOrDerived P, typename Hash = loki::Hash<Key>, typename KeyEqual = loki::EqualTo<Key>>
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

class ConjunctiveConditionImpl;
using ConjunctiveCondition = const ConjunctiveConditionImpl*;

class GroundConjunctiveCondition;

class TermImpl;
using Term = const TermImpl*;
using TermList = std::vector<Term>;

}

#endif
