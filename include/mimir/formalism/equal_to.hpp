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

#ifndef MIMIR_FORMALISM_EQUAL_TO_HPP_
#define MIMIR_FORMALISM_EQUAL_TO_HPP_

#include "mimir/formalism/declarations.hpp"

#include <functional>
#include <variant>

namespace mimir
{

/// @brief `UniquePDDLEqualTo` is used to compare newly created PDDL objects for uniqueness.
/// Since the children are unique, it suffices to compare nested pointers.
template<typename T>
struct UniquePDDLEqualTo
{
    bool operator()(const T& l, const T& r) const { return std::equal_to<T>()(l, r); }
};

template<>
struct UniquePDDLEqualTo<Action>
{
    bool operator()(Action l, Action r) const;
};

template<PredicateTag P>
struct UniquePDDLEqualTo<Atom<P>>
{
    bool operator()(Atom<P> l, Atom<P> r) const;
};

template<>
struct UniquePDDLEqualTo<Axiom>
{
    bool operator()(Axiom l, Axiom r) const;
};

template<>
struct UniquePDDLEqualTo<Domain>
{
    bool operator()(Domain l, Domain r) const;
};

template<>
struct UniquePDDLEqualTo<EffectStrips>
{
    bool operator()(EffectStrips l, EffectStrips r) const;
};

template<>
struct UniquePDDLEqualTo<EffectConditional>
{
    bool operator()(EffectConditional l, EffectConditional r) const;
};

template<>
struct UniquePDDLEqualTo<FunctionExpressionNumber>
{
    bool operator()(FunctionExpressionNumber l, FunctionExpressionNumber r) const;
};

template<>
struct UniquePDDLEqualTo<FunctionExpressionBinaryOperator>
{
    bool operator()(FunctionExpressionBinaryOperator l, FunctionExpressionBinaryOperator r) const;
};

template<>
struct UniquePDDLEqualTo<FunctionExpressionMultiOperator>
{
    bool operator()(FunctionExpressionMultiOperator l, FunctionExpressionMultiOperator r) const;
};

template<>
struct UniquePDDLEqualTo<FunctionExpressionMinus>
{
    bool operator()(FunctionExpressionMinus l, FunctionExpressionMinus r) const;
};

template<>
struct UniquePDDLEqualTo<FunctionExpressionFunction>
{
    bool operator()(FunctionExpressionFunction l, FunctionExpressionFunction r) const;
};

template<>
struct UniquePDDLEqualTo<FunctionExpression>
{
    bool operator()(FunctionExpression l, FunctionExpression r) const;
};

template<>
struct UniquePDDLEqualTo<FunctionSkeleton>
{
    bool operator()(FunctionSkeleton l, FunctionSkeleton r) const;
};

template<>
struct UniquePDDLEqualTo<Function>
{
    bool operator()(Function l, Function r) const;
};

template<PredicateTag P>
struct UniquePDDLEqualTo<GroundAtom<P>>
{
    bool operator()(GroundAtom<P> l, GroundAtom<P> r) const;
};

template<>
struct UniquePDDLEqualTo<GroundFunctionExpressionNumber>
{
    bool operator()(GroundFunctionExpressionNumber l, GroundFunctionExpressionNumber r) const;
};

template<>
struct UniquePDDLEqualTo<GroundFunctionExpressionBinaryOperator>
{
    bool operator()(GroundFunctionExpressionBinaryOperator l, GroundFunctionExpressionBinaryOperator r) const;
};

template<>
struct UniquePDDLEqualTo<GroundFunctionExpressionMultiOperator>
{
    bool operator()(GroundFunctionExpressionMultiOperator l, GroundFunctionExpressionMultiOperator r) const;
};

template<>
struct UniquePDDLEqualTo<GroundFunctionExpressionMinus>
{
    bool operator()(GroundFunctionExpressionMinus l, GroundFunctionExpressionMinus r) const;
};

template<>
struct UniquePDDLEqualTo<GroundFunctionExpressionFunction>
{
    bool operator()(GroundFunctionExpressionFunction l, GroundFunctionExpressionFunction r) const;
};

template<>
struct UniquePDDLEqualTo<GroundFunctionExpression>
{
    bool operator()(GroundFunctionExpression l, GroundFunctionExpression r) const;
};

template<>
struct UniquePDDLEqualTo<GroundFunction>
{
    bool operator()(GroundFunction l, GroundFunction r) const;
};

template<PredicateTag P>
struct UniquePDDLEqualTo<GroundLiteral<P>>
{
    bool operator()(GroundLiteral<P> l, GroundLiteral<P> r) const;
};

template<PredicateTag P>
struct UniquePDDLEqualTo<Literal<P>>
{
    bool operator()(Literal<P> l, Literal<P> r) const;
};

template<>
struct UniquePDDLEqualTo<OptimizationMetric>
{
    bool operator()(OptimizationMetric l, OptimizationMetric r) const;
};

template<>
struct UniquePDDLEqualTo<NumericFluent>
{
    bool operator()(NumericFluent l, NumericFluent r) const;
};

template<>
struct UniquePDDLEqualTo<Object>
{
    bool operator()(Object l, Object r) const;
};

template<PredicateTag P>
struct UniquePDDLEqualTo<Predicate<P>>
{
    bool operator()(Predicate<P> l, Predicate<P> r) const;
};

template<>
struct UniquePDDLEqualTo<Problem>
{
    bool operator()(Problem l, Problem r) const;
};

template<>
struct UniquePDDLEqualTo<Requirements>
{
    bool operator()(Requirements l, Requirements r) const;
};

template<>
struct UniquePDDLEqualTo<Term>
{
    bool operator()(Term l, Term r) const;
};

template<>
struct UniquePDDLEqualTo<Variable>
{
    bool operator()(Variable l, Variable r) const;
};

}

#endif
