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

#ifndef MIMIR_FORMALISM_TRANSLATORS_INTERFACE_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_INTERFACE_HPP_

#include <algorithm>
#include <boost/container/small_vector.hpp>
#include <loki/loki.hpp>
#include <tuple>
#include <variant>
#include <vector>

namespace mimir
{

/**
 * Interface class.
 */
template<typename Derived>
class ITranslator
{
private:
    ITranslator() = default;
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
    void prepare(const loki::RequirementsImpl& requirements) { self().prepare_base(requirements); }
    void prepare(const loki::TypeImpl& type) { self().prepare_base(type); }
    void prepare(const loki::ObjectImpl& object) { self().prepare_base(object); }
    void prepare(const loki::VariableImpl& variable) { self().prepare_base(variable); }
    void prepare(const loki::TermObjectImpl& term) { self().prepare_base(term); }
    void prepare(const loki::TermVariableImpl& term) { self().prepare_base(term); }
    void prepare(const loki::TermImpl& term) { self().prepare_base(term); }
    void prepare(const loki::ParameterImpl& parameter) { self().prepare_base(parameter); }
    void prepare(const loki::PredicateImpl& predicate) { self().prepare_base(predicate); }
    void prepare(const loki::AtomImpl& atom) { self().prepare_base(atom); }
    void prepare(const loki::LiteralImpl& literal) { self().prepare_base(literal); }
    void prepare(const loki::NumericFluentImpl& numeric_fluent) { self().prepare_base(numeric_fluent); }
    void prepare(const loki::ConditionLiteralImpl& condition) { self().prepare_base(condition); }
    void prepare(const loki::ConditionAndImpl& condition) { self().prepare_base(condition); }
    void prepare(const loki::ConditionOrImpl& condition) { self().prepare_base(condition); }
    void prepare(const loki::ConditionNotImpl& condition) { self().prepare_base(condition); }
    void prepare(const loki::ConditionImplyImpl& condition) { self().prepare_base(condition); }
    void prepare(const loki::ConditionExistsImpl& condition) { self().prepare_base(condition); }
    void prepare(const loki::ConditionForallImpl& condition) { self().prepare_base(condition); }
    void prepare(const loki::ConditionImpl& condition) { self().prepare_base(condition); }
    void prepare(const loki::EffectLiteralImpl& effect) { self().prepare_base(effect); }
    void prepare(const loki::EffectAndImpl& effect) { self().prepare_base(effect); }
    void prepare(const loki::EffectNumericImpl& effect) { self().prepare_base(effect); }
    void prepare(const loki::EffectConditionalForallImpl& effect) { self().prepare_base(effect); }
    void prepare(const loki::EffectConditionalWhenImpl& effect) { self().prepare_base(effect); }
    void prepare(const loki::EffectImpl& effect) { self().prepare_base(effect); }
    void prepare(const loki::FunctionExpressionNumberImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const loki::FunctionExpressionBinaryOperatorImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const loki::FunctionExpressionMultiOperatorImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const loki::FunctionExpressionMinusImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const loki::FunctionExpressionFunctionImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const loki::FunctionExpressionImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const loki::FunctionSkeletonImpl& function_skeleton) { self().prepare_base(function_skeleton); }
    void prepare(const loki::FunctionImpl& function) { self().prepare_base(function); }
    void prepare(const loki::ActionImpl& action) { self().prepare_base(action); }
    void prepare(const loki::AxiomImpl& axiom) { self().prepare_base(axiom); }
    void prepare(const loki::DomainImpl& domain) { self().prepare_base(domain); }
    void prepare(const loki::OptimizationMetricImpl& metric) { self().prepare_base(metric); }
    void prepare(const loki::ProblemImpl& problem) { self().prepare_base(problem); }

    /// @brief Translate a container of elements into a container of elements.
    template<typename Container>
    auto translate(const Container& input)
    {
        Container output;
        output.reserve(input.size());
        std::transform(std::begin(input), std::end(input), std::back_inserter(output), [this](auto&& arg) { return this->translate(*arg); });
        return output;
    }
    loki::Requirements translate(const loki::RequirementsImpl& requirements) { return self().translate_base(requirements); }
    loki::Type translate(const loki::TypeImpl& type) { return self().translate_base(type); }
    loki::Object translate(const loki::ObjectImpl& object) { return self().translate_base(object); }
    loki::Variable translate(const loki::VariableImpl& variable) { return self().translate_base(variable); }
    loki::Term translate(const loki::TermObjectImpl& term) { return self().translate_base(term); }
    loki::Term translate(const loki::TermVariableImpl& term) { return self().translate_base(term); }
    loki::Term translate(const loki::TermImpl& term) { return self().translate_base(term); }
    loki::Parameter translate(const loki::ParameterImpl& parameter) { return self().translate_base(parameter); }
    loki::Predicate translate(const loki::PredicateImpl& predicate) { return self().translate_base(predicate); }
    loki::Atom translate(const loki::AtomImpl& atom) { return self().translate_base(atom); }
    loki::Literal translate(const loki::LiteralImpl& literal) { return self().translate_base(literal); }
    loki::NumericFluent translate(const loki::NumericFluentImpl& numeric_fluent) { return self().translate_base(numeric_fluent); }
    loki::Condition translate(const loki::ConditionLiteralImpl& condition) { return self().translate_base(condition); }
    loki::Condition translate(const loki::ConditionAndImpl& condition) { return self().translate_base(condition); }
    loki::Condition translate(const loki::ConditionOrImpl& condition) { return self().translate_base(condition); }
    loki::Condition translate(const loki::ConditionNotImpl& condition) { return self().translate_base(condition); }
    loki::Condition translate(const loki::ConditionImplyImpl& condition) { return self().translate_base(condition); }
    loki::Condition translate(const loki::ConditionExistsImpl& condition) { return self().translate_base(condition); }
    loki::Condition translate(const loki::ConditionForallImpl& condition) { return self().translate_base(condition); }
    loki::Condition translate(const loki::ConditionImpl& condition) { return self().translate_base(condition); }
    loki::Effect translate(const loki::EffectLiteralImpl& effect) { return self().translate_base(effect); }
    loki::Effect translate(const loki::EffectAndImpl& effect) { return self().translate_base(effect); }
    loki::Effect translate(const loki::EffectNumericImpl& effect) { return self().translate_base(effect); }
    loki::Effect translate(const loki::EffectConditionalForallImpl& effect) { return self().translate_base(effect); }
    loki::Effect translate(const loki::EffectConditionalWhenImpl& effect) { return self().translate_base(effect); }
    loki::Effect translate(const loki::EffectImpl& effect) { return self().translate_base(effect); }
    loki::FunctionExpression translate(const loki::FunctionExpressionNumberImpl& function_expression) { return self().translate_base(function_expression); }
    loki::FunctionExpression translate(const loki::FunctionExpressionBinaryOperatorImpl& function_expression)
    {
        return self().translate_base(function_expression);
    }
    loki::FunctionExpression translate(const loki::FunctionExpressionMultiOperatorImpl& function_expression)
    {
        return self().translate_base(function_expression);
    }
    loki::FunctionExpression translate(const loki::FunctionExpressionMinusImpl& function_expression) { return self().translate_base(function_expression); }
    loki::FunctionExpression translate(const loki::FunctionExpressionFunctionImpl& function_expression) { return self().translate_base(function_expression); }
    loki::FunctionExpression translate(const loki::FunctionExpressionImpl& function_expression) { return self().translate_base(function_expression); }
    loki::FunctionSkeleton translate(const loki::FunctionSkeletonImpl& function_skeleton) { return self().translate_base(function_skeleton); }
    loki::Function translate(const loki::FunctionImpl& function) { return self().translate_base(function); }
    loki::Action translate(const loki::ActionImpl& action) { return self().translate_base(action); }
    loki::Axiom translate(const loki::AxiomImpl& axiom) { return self().translate_base(axiom); }
    loki::Domain translate(const loki::DomainImpl& domain) { return self().translate_base(domain); }
    loki::OptimizationMetric translate(const loki::OptimizationMetricImpl& metric) { return self().translate_base(metric); }
    loki::Problem translate(const loki::ProblemImpl& problem) { return self().translate_base(problem); }

    /// @brief Collect information and apply problem translation.
    loki::Problem run(const loki::ProblemImpl& problem) { return self().run_base(problem); }
};

}

#endif
