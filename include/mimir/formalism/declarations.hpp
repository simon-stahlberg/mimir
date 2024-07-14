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
#include "mimir/formalism/predicate_category.hpp"

#include <loki/loki.hpp>
#include <string>
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

template<PredicateCategory P>
class AtomImpl;
template<PredicateCategory P>
using Atom = const AtomImpl<P>*;
template<PredicateCategory P>
using AtomList = std::vector<Atom<P>>;

class AxiomImpl;
using Axiom = const AxiomImpl*;
using AxiomList = std::vector<Axiom>;
using AxiomSet = std::unordered_set<Axiom>;

class DomainImpl;
using Domain = const DomainImpl*;
using DomainList = std::vector<Domain>;

class EffectSimpleImpl;
using EffectSimple = const EffectSimpleImpl*;
using EffectSimpleList = std::vector<EffectSimple>;

class EffectConditionalImpl;
using EffectConditional = const EffectConditionalImpl*;
using EffectConditionalList = std::vector<EffectConditional>;

class EffectUniversalImpl;
using EffectUniversal = const EffectUniversalImpl*;
using EffectUniversalList = std::vector<EffectUniversal>;

class FunctionExpressionNumberImpl;
using FunctionExpressionNumber = const FunctionExpressionNumberImpl*;
class FunctionExpressionBinaryOperatorImpl;
using FunctionExpressionBinaryOperator = const FunctionExpressionBinaryOperatorImpl*;
class FunctionExpressionMultiOperatorImpl;
using FunctionExpressionMultiOperator = const FunctionExpressionMultiOperatorImpl*;
class FunctionExpressionMinusImpl;
using FunctionExpressionMinus = const FunctionExpressionMinusImpl*;
class FunctionExpressionFunctionImpl;
using FunctionExpressionImpl = std::variant<FunctionExpressionNumberImpl,
                                            FunctionExpressionBinaryOperatorImpl,
                                            FunctionExpressionMultiOperatorImpl,
                                            FunctionExpressionMinusImpl,
                                            FunctionExpressionFunctionImpl>;
using FunctionExpression = const FunctionExpressionImpl*;
using FunctionExpressionList = std::vector<FunctionExpression>;

class FunctionSkeletonImpl;
using FunctionSkeleton = const FunctionSkeletonImpl*;
using FunctionSkeletonList = std::vector<FunctionSkeleton>;

class FunctionImpl;
using Function = const FunctionImpl*;
using FunctionList = std::vector<Function>;

template<PredicateCategory P>
class GroundAtomImpl;
template<PredicateCategory P>
using GroundAtom = const GroundAtomImpl<P>*;
template<PredicateCategory P>
using GroundAtomList = std::vector<GroundAtom<P>>;
template<PredicateCategory P>
using GroundAtomSet = std::unordered_set<GroundAtom<P>>;

class GroundFunctionExpressionNumberImpl;
using GroundFunctionExpressionNumber = const GroundFunctionExpressionNumberImpl*;
class GroundFunctionExpressionBinaryOperatorImpl;
using GroundFunctionExpressionBinaryOperator = const GroundFunctionExpressionBinaryOperatorImpl*;
class GroundFunctionExpressionMultiOperatorImpl;
using GroundFunctionExpressionMultiOperator = const GroundFunctionExpressionMultiOperatorImpl*;
class GroundFunctionExpressionMinusImpl;
using GroundFunctionExpressionMinus = const GroundFunctionExpressionMinusImpl*;
class GroundFunctionExpressionFunctionImpl;
using GroundFunctionExpressionImpl = std::variant<GroundFunctionExpressionNumberImpl,
                                                  GroundFunctionExpressionBinaryOperatorImpl,
                                                  GroundFunctionExpressionMultiOperatorImpl,
                                                  GroundFunctionExpressionMinusImpl,
                                                  GroundFunctionExpressionFunctionImpl>;
using GroundFunctionExpression = const GroundFunctionExpressionImpl*;
using GroundFunctionExpressionList = std::vector<GroundFunctionExpression>;

class GroundFunctionImpl;
using GroundFunction = const GroundFunctionImpl*;
using GroundFunctionList = std::vector<GroundFunction>;

template<PredicateCategory P>
class GroundLiteralImpl;
template<PredicateCategory P>
using GroundLiteral = const GroundLiteralImpl<P>*;
template<PredicateCategory P>
using GroundLiteralList = std::vector<GroundLiteral<P>>;
template<PredicateCategory P>
using GroundLiteralSet = std::unordered_set<GroundLiteral<P>>;

template<PredicateCategory P>
class LiteralImpl;
template<PredicateCategory P>
using Literal = const LiteralImpl<P>*;
template<PredicateCategory P>
using LiteralList = std::vector<Literal<P>>;
template<PredicateCategory P>
using LiteralSet = std::unordered_set<Literal<P>>;

class OptimizationMetricImpl;
using OptimizationMetric = const OptimizationMetricImpl*;

class NumericFluentImpl;
using NumericFluent = const NumericFluentImpl*;
using NumericFluentList = std::vector<NumericFluent>;

class ObjectImpl;
using Object = const ObjectImpl*;
using ObjectList = std::vector<Object>;

class PDDLFactories;

template<PredicateCategory P>
class PredicateImpl;
template<PredicateCategory P>
using Predicate = const PredicateImpl<P>*;
template<PredicateCategory P>
using PredicateList = std::vector<Predicate<P>>;
template<PredicateCategory P>
using PredicateSet = std::unordered_set<Predicate<P>>;
template<typename Key, PredicateCategory P, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
using ToPredicateMap = std::unordered_map<Key, Predicate<P>, Hash, KeyEqual>;

class ProblemImpl;
using Problem = const ProblemImpl*;
using ProblemList = std::vector<Problem>;

class RequirementsImpl;
using Requirements = const RequirementsImpl*;

class TermObjectImpl;
using TermObject = const TermObjectImpl*;
class TermVariableImpl;
using TermVariable = const TermVariableImpl*;
using TermImpl = std::variant<TermObjectImpl, TermVariableImpl>;
using Term = const TermImpl*;
using TermList = std::vector<Term>;

class VariableImpl;
using Variable = const VariableImpl*;
using VariableList = std::vector<Variable>;
using VariableSet = std::unordered_set<Variable>;

}

#endif
