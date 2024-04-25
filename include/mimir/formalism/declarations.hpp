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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_FORMALISM_DECLARATIONS_HPP_
#define MIMIR_FORMALISM_DECLARATIONS_HPP_

#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace mimir
{

/* Forward declarations */
class PDDLFactories;

/* Data types*/
using ElementsPerSegment = size_t;

/* Domain Specific */
class RequirementsImpl;
using Requirements = const RequirementsImpl*;

class ObjectImpl;
using Object = const ObjectImpl*;
using ObjectList = std::vector<Object>;

class VariableImpl;
using Variable = const VariableImpl*;
using VariableList = std::vector<Variable>;
using VariableSet = std::unordered_set<Variable>;
using VariableAssignment = std::unordered_map<Variable, Object>;

class TermObjectImpl;
using TermObject = const TermObjectImpl*;
class TermVariableImpl;
using TermVariable = const TermVariableImpl*;
using TermImpl = std::variant<TermObjectImpl, TermVariableImpl>;
using Term = const TermImpl*;
using TermList = std::vector<Term>;

class ParameterImpl;
using Parameter = const ParameterImpl*;
using ParameterList = std::vector<Parameter>;
using ParameterAssignment = std::unordered_map<Parameter, Object>;

class PredicateImpl;
using Predicate = const PredicateImpl*;
using PredicateList = std::vector<Predicate>;
using PredicateSet = std::unordered_set<Predicate>;

class AtomImpl;
using Atom = const AtomImpl*;
using AtomList = std::vector<Atom>;

class GroundAtomImpl;
using GroundAtom = const GroundAtomImpl*;
using GroundAtomList = std::vector<GroundAtom>;

class LiteralImpl;
using Literal = const LiteralImpl*;
using LiteralList = std::vector<Literal>;

class GroundLiteralImpl;
using GroundLiteral = const GroundLiteralImpl*;
using GroundLiteralList = std::vector<GroundLiteral>;

class NumericFluentImpl;
using NumericFluent = const NumericFluentImpl*;
using NumericFluentList = std::vector<NumericFluent>;

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

class FunctionSkeletonImpl;
using FunctionSkeleton = const FunctionSkeletonImpl*;
using FunctionSkeletonList = std::vector<FunctionSkeleton>;

class FunctionImpl;
using Function = const FunctionImpl*;
using FunctionList = std::vector<Function>;

class GroundFunctionImpl;
using GroundFunction = const GroundFunctionImpl*;
using GroundFunctionList = std::vector<GroundFunction>;

class ActionImpl;
using Action = const ActionImpl*;
using ActionList = std::vector<Action>;

class AxiomImpl;
using Axiom = const AxiomImpl*;
using AxiomList = std::vector<Axiom>;
using AxiomSet = std::unordered_set<Axiom>;

class DomainImpl;
using Domain = const DomainImpl*;
using DomainList = std::vector<Domain>;

class OptimizationMetricImpl;
using OptimizationMetric = const OptimizationMetricImpl*;

class ProblemImpl;
using Problem = const ProblemImpl*;
using ProblemList = std::vector<Problem>;
}

#endif
