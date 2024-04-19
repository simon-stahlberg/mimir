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

#include "mimir/formalism/declarations.hpp"

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
template<typename Derived>
class ITransformer
{
private:
    ITransformer() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    /// @brief Prepare all elements in a container.
    template<typename Container>
    void prepare(const Container& input)
    {
        std::for_each(std::begin(input), std::end(input), [this](auto&& arg) { this->prepare(*arg); });
    }
    void prepare(const RequirementsImpl& requirements) { self().prepare_base(requirements); }
    void prepare(const ObjectImpl& object) { self().prepare_base(object); }
    void prepare(const VariableImpl& variable) { self().prepare_base(variable); }
    void prepare(const TermObjectImpl& term) { self().prepare_base(term); }
    void prepare(const TermVariableImpl& term) { self().prepare_base(term); }
    void prepare(const TermImpl& term) { self().prepare_base(term); }
    void prepare(const ParameterImpl& parameter) { self().prepare_base(parameter); }
    void prepare(const PredicateImpl& predicate) { self().prepare_base(predicate); }
    void prepare(const AtomImpl& atom) { self().prepare_base(atom); }
    void prepare(const GroundAtomImpl& atom) { self().prepare_base(atom); }
    void prepare(const LiteralImpl& literal) { self().prepare_base(literal); }
    void prepare(const NumericFluentImpl& numeric_fluent) { self().prepare_base(numeric_fluent); }
    void prepare(const GroundLiteralImpl& literal) { self().prepare_base(literal); }
    void prepare(const EffectImpl& effect) { self().prepare_base(effect); }
    void prepare(const FunctionExpressionNumberImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const FunctionExpressionBinaryOperatorImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const FunctionExpressionMultiOperatorImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const FunctionExpressionMinusImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const FunctionExpressionFunctionImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const FunctionExpressionImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const FunctionSkeletonImpl& function_skeleton) { self().prepare_base(function_skeleton); }
    void prepare(const FunctionImpl& function) { self().prepare_base(function); }
    void prepare(const ActionImpl& action) { self().prepare_base(action); }
    void prepare(const AxiomImpl& axiom) { self().prepare_base(axiom); }
    void prepare(const DomainImpl& domain) { self().prepare_base(domain); }
    void prepare(const OptimizationMetricImpl& metric) { self().prepare_base(metric); }
    void prepare(const ProblemImpl& problem) { self().prepare_base(problem); }

    /// @brief Transform a container of elements into a container of elements.
    template<typename Container>
    auto transform(const Container& input)
    {
        Container output;
        output.reserve(input.size());
        std::transform(std::begin(input), std::end(input), std::back_inserter(output), [this](auto&& arg) { return this->transform(*arg); });
        return output;
    }
    Requirements transform(const RequirementsImpl& requirements) { return self().transform_base(requirements); }
    Object transform(const ObjectImpl& object) { return self().transform_base(object); }
    Variable transform(const VariableImpl& variable) { return self().transform_base(variable); }
    Term transform(const TermObjectImpl& term) { return self().transform_base(term); }
    Term transform(const TermVariableImpl& term) { return self().transform_base(term); }
    Term transform(const TermImpl& term) { return self().transform_base(term); }
    Parameter transform(const ParameterImpl& parameter) { return self().transform_base(parameter); }
    Predicate transform(const PredicateImpl& predicate) { return self().transform_base(predicate); }
    Atom transform(const AtomImpl& atom) { return self().transform_base(atom); }
    GroundAtom transform(const GroundAtomImpl& atom) { return self().transform_base(atom); }
    Literal transform(const LiteralImpl& literal) { return self().transform_base(literal); }
    GroundLiteral transform(const GroundLiteralImpl& literal) { return self().transform_base(literal); }
    NumericFluent transform(const NumericFluentImpl& numeric_fluent) { return self().transform_base(numeric_fluent); }
    Effect transform(const EffectImpl& effect) { return self().transform_base(effect); }
    FunctionExpression transform(const FunctionExpressionNumberImpl& function_expression) { return self().transform_base(function_expression); }
    FunctionExpression transform(const FunctionExpressionBinaryOperatorImpl& function_expression) { return self().transform_base(function_expression); }
    FunctionExpression transform(const FunctionExpressionMultiOperatorImpl& function_expression) { return self().transform_base(function_expression); }
    FunctionExpression transform(const FunctionExpressionMinusImpl& function_expression) { return self().transform_base(function_expression); }
    FunctionExpression transform(const FunctionExpressionFunctionImpl& function_expression) { return self().transform_base(function_expression); }
    FunctionExpression transform(const FunctionExpressionImpl& function_expression) { return self().transform_base(function_expression); }
    FunctionSkeleton transform(const FunctionSkeletonImpl& function_skeleton) { return self().transform_base(function_skeleton); }
    Function transform(const FunctionImpl& function) { return self().transform_base(function); }
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
