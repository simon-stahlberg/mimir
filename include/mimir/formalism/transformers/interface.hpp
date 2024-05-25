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

#ifndef MIMIR_FORMALISM_TRANSFORMERS_INTERFACE_HPP_
#define MIMIR_FORMALISM_TRANSFORMERS_INTERFACE_HPP_

#include "mimir/formalism/pddl.hpp"

#include <algorithm>
#include <boost/container/small_vector.hpp>
#include <tuple>
#include <variant>
#include <vector>

namespace mimir
{

/**
 * Interface class.
 */
template<typename Derived_>
class ITransformer
{
private:
    ITransformer() = default;
    friend Derived_;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

public:
    template<typename Container>
    void prepare(const Container& input)
    {
        self().prepare_base(input);
    }
    void prepare(const RequirementsImpl& requirements) { self().prepare_base(requirements); }
    void prepare(const ObjectImpl& object) { self().prepare_base(object); }
    void prepare(const VariableImpl& variable) { self().prepare_base(variable); }
    void prepare(const TermObjectImpl& term) { self().prepare_base(term); }
    void prepare(const TermVariableImpl& term) { self().prepare_base(term); }
    void prepare(const TermImpl& term) { self().prepare_base(term); }
    template<PredicateCategory P>
    void prepare(const PredicateImpl<P>& predicate)
    {
        self().prepare_base(predicate);
    }
    template<PredicateCategory P>
    void prepare(const AtomImpl<P>& atom)
    {
        self().prepare_base(atom);
    }
    template<PredicateCategory P>
    void prepare(const GroundAtomImpl<P>& atom)
    {
        self().prepare_base(atom);
    }
    template<PredicateCategory P>
    void prepare(const LiteralImpl<P>& literal)
    {
        self().prepare_base(literal);
    }
    template<PredicateCategory P>
    void prepare(const GroundLiteralImpl<P>& literal)
    {
        self().prepare_base(literal);
    }
    void prepare(const NumericFluentImpl& numeric_fluent) { self().prepare_base(numeric_fluent); }
    void prepare(const EffectSimpleImpl& effect) { self().prepare_base(effect); }
    void prepare(const EffectConditionalImpl& effect) { self().prepare_base(effect); }
    void prepare(const EffectUniversalImpl& effect) { self().prepare_base(effect); }
    void prepare(const FunctionExpressionNumberImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const FunctionExpressionBinaryOperatorImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const FunctionExpressionMultiOperatorImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const FunctionExpressionMinusImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const FunctionExpressionFunctionImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const FunctionExpressionImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const GroundFunctionExpressionNumberImpl& function_expression) { return self().prepare_base(function_expression); }
    void prepare(const GroundFunctionExpressionBinaryOperatorImpl& function_expression) { return self().prepare_base(function_expression); }
    void prepare(const GroundFunctionExpressionMultiOperatorImpl& function_expression) { return self().prepare_base(function_expression); }
    void prepare(const GroundFunctionExpressionMinusImpl& function_expression) { return self().prepare_base(function_expression); }
    void prepare(const GroundFunctionExpressionFunctionImpl& function_expression) { return self().prepare_base(function_expression); }
    void prepare(const GroundFunctionExpressionImpl& function_expression) { return self().prepare_base(function_expression); }
    void prepare(const FunctionSkeletonImpl& function_skeleton) { self().prepare_base(function_skeleton); }
    void prepare(const FunctionImpl& function) { self().prepare_base(function); }
    void prepare(const GroundFunctionImpl& function) { return self().prepare_base(function); }
    void prepare(const ActionImpl& action) { self().prepare_base(action); }
    void prepare(const AxiomImpl& axiom) { self().prepare_base(axiom); }
    void prepare(const DomainImpl& domain) { self().prepare_base(domain); }
    void prepare(const OptimizationMetricImpl& metric) { self().prepare_base(metric); }
    void prepare(const ProblemImpl& problem) { self().prepare_base(problem); }

    template<typename Container>
    auto transform(const Container& input)
    {
        return self().transform_base(input);
    }
    Requirements transform(const RequirementsImpl& requirements) { return self().transform_base(requirements); }
    Object transform(const ObjectImpl& object) { return self().transform_base(object); }
    Variable transform(const VariableImpl& variable) { return self().transform_base(variable); }
    Term transform(const TermObjectImpl& term) { return self().transform_base(term); }
    Term transform(const TermVariableImpl& term) { return self().transform_base(term); }
    Term transform(const TermImpl& term) { return self().transform_base(term); }
    template<PredicateCategory P>
    Predicate<P> transform(const PredicateImpl<P>& predicate)
    {
        return self().transform_base(predicate);
    }
    template<PredicateCategory P>
    Atom<P> transform(const AtomImpl<P>& atom)
    {
        return self().transform_base(atom);
    }
    template<PredicateCategory P>
    GroundAtom<P> transform(const GroundAtomImpl<P>& atom)
    {
        return self().transform_base(atom);
    }
    template<PredicateCategory P>
    Literal<P> transform(const LiteralImpl<P>& literal)
    {
        return self().transform_base(literal);
    }
    template<PredicateCategory P>
    GroundLiteral<P> transform(const GroundLiteralImpl<P>& literal)
    {
        return self().transform_base(literal);
    }
    NumericFluent transform(const NumericFluentImpl& numeric_fluent) { return self().transform_base(numeric_fluent); }
    EffectSimple transform(const EffectSimpleImpl& effect) { return self().transform_base(effect); }
    EffectConditional transform(const EffectConditionalImpl& effect) { return self().transform_base(effect); }
    EffectUniversal transform(const EffectUniversalImpl& effect) { return self().transform_base(effect); }
    FunctionExpression transform(const FunctionExpressionNumberImpl& function_expression) { return self().transform_base(function_expression); }
    FunctionExpression transform(const FunctionExpressionBinaryOperatorImpl& function_expression) { return self().transform_base(function_expression); }
    FunctionExpression transform(const FunctionExpressionMultiOperatorImpl& function_expression) { return self().transform_base(function_expression); }
    FunctionExpression transform(const FunctionExpressionMinusImpl& function_expression) { return self().transform_base(function_expression); }
    FunctionExpression transform(const FunctionExpressionFunctionImpl& function_expression) { return self().transform_base(function_expression); }
    FunctionExpression transform(const FunctionExpressionImpl& function_expression) { return self().transform_base(function_expression); }
    GroundFunctionExpression transform(const GroundFunctionExpressionNumberImpl& function_expression) { return self().transform_base(function_expression); }
    GroundFunctionExpression transform(const GroundFunctionExpressionBinaryOperatorImpl& function_expression)
    {
        return self().transform_base(function_expression);
    }
    GroundFunctionExpression transform(const GroundFunctionExpressionMultiOperatorImpl& function_expression)
    {
        return self().transform_base(function_expression);
    }
    GroundFunctionExpression transform(const GroundFunctionExpressionMinusImpl& function_expression) { return self().transform_base(function_expression); }
    GroundFunctionExpression transform(const GroundFunctionExpressionFunctionImpl& function_expression) { return self().transform_base(function_expression); }
    GroundFunctionExpression transform(const GroundFunctionExpressionImpl& function_expression) { return self().transform_base(function_expression); }
    FunctionSkeleton transform(const FunctionSkeletonImpl& function_skeleton) { return self().transform_base(function_skeleton); }
    Function transform(const FunctionImpl& function) { return self().transform_base(function); }
    GroundFunction transform(const GroundFunctionImpl& function) { return self().transform_base(function); }
    Action transform(const ActionImpl& action) { return self().transform_base(action); }
    Axiom transform(const AxiomImpl& axiom) { return self().transform_base(axiom); }
    Domain transform(const DomainImpl& domain) { return self().transform_base(domain); }
    OptimizationMetric transform(const OptimizationMetricImpl& metric) { return self().transform_base(metric); }
    Problem transform(const ProblemImpl& problem) { return self().transform_base(problem); }

    /// @brief Collect information and apply problem transformation.
    Problem run(const ProblemImpl& problem) { return self().run_base(problem); }
};
}

#endif
