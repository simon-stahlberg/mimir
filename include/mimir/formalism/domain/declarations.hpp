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

#ifndef MIMIR_FORMALISM_DOMAIN_DECLARATIONS_HPP_
#define MIMIR_FORMALISM_DOMAIN_DECLARATIONS_HPP_

#include "../common/declarations.hpp"

#include <boost/container/small_vector.hpp>

#include <unordered_map>
#include <vector>
#include <variant>


namespace mimir 
{
    class RequirementsImpl;
    using Requirements = const RequirementsImpl*;

    class TypeImpl;
    using Type = const TypeImpl*;
    using TypeList = boost::container::small_vector<Type,1>;  // often single type

    class ObjectImpl;
    using Object = const ObjectImpl*;
    using ObjectList = std::vector<Object>;

    class VariableImpl;
    using Variable = const VariableImpl*;
    using VariableList = std::vector<Variable>;  // not used
    using VariableAssignment = std::unordered_map<Variable, Object>;

    class TermObjectImpl;
    class TermVariableImpl;
    using TermImpl = std::variant<TermObjectImpl, TermVariableImpl>;
    using Term = const TermImpl*;
    using TermList = boost::container::small_vector<Term, 2>;  // often unary and binary predicates

    class AtomImpl;
    using Atom = const AtomImpl*;
    using AtomList = std::vector<Atom>;

    class ParameterImpl;
    using Parameter = const ParameterImpl*;
    using ParameterList = boost::container::small_vector<Parameter, 10>;  // often actions, quantifiers with few parameters
    using ParameterAssignment = std::unordered_map<Parameter, Object>;

    class PredicateImpl;
    using Predicate = const PredicateImpl*;
    using PredicateList = std::vector<Predicate>;

    class LiteralImpl;
    using Literal = const LiteralImpl*;
    using LiteralList = std::vector<Literal>;

    class ConditionLiteralImpl;
    class ConditionAndImpl;
    class ConditionOrImpl;
    class ConditionNotImpl;
    class ConditionImplyImpl;
    class ConditionExistsImpl;
    class ConditionForallImpl;
    using ConditionImpl = std::variant<ConditionLiteralImpl
        , ConditionAndImpl
        , ConditionOrImpl
        , ConditionNotImpl
        , ConditionImplyImpl
        , ConditionExistsImpl
        , ConditionForallImpl>;
    using Condition = const ConditionImpl*;
    using ConditionList = std::vector<Condition>;

    class EffectLiteralImpl;
    class EffectAndImpl;
    class EffectNumericImpl;
    class EffectConditionalForallImpl;
    class EffectConditionalWhenImpl;
    using EffectImpl = std::variant<EffectLiteralImpl
        , EffectAndImpl
        , EffectNumericImpl
        , EffectConditionalForallImpl
        , EffectConditionalWhenImpl>;
    using Effect = const EffectImpl*;
    using EffectList = std::vector<Effect>;

    class FunctionExpressionNumberImpl;
    class FunctionExpressionBinaryOperatorImpl;
    class FunctionExpressionMultiOperatorImpl;
    class FunctionExpressionMinusImpl;
    class FunctionExpressionFunctionImpl;
    using FunctionExpressionImpl = std::variant<FunctionExpressionNumberImpl
        , FunctionExpressionBinaryOperatorImpl
        , FunctionExpressionMultiOperatorImpl
        , FunctionExpressionMinusImpl
        , FunctionExpressionFunctionImpl>;
    using FunctionExpression = const FunctionExpressionImpl*;
    using FunctionExpressionList = std::vector<FunctionExpression>;

    class FunctionSkeletonImpl;
    using FunctionSkeleton = const FunctionSkeletonImpl*;
    using FunctionSkeletonList = std::vector<FunctionSkeleton>;

    class FunctionImpl;
    using Function = const FunctionImpl*;
    using FunctionList = std::vector<Function>;

    class ConstraintImpl;
    using Constraint = const ConstraintImpl*;
    using ConstraintList = std::vector<Constraint>;

    class ActionImpl;
    using Action = const ActionImpl*;
    using ActionList = std::vector<Action>;

    class DomainImpl;
    using Domain = const DomainImpl*;
    using DomainList = std::vector<Domain>;
}

#endif
