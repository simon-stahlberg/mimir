/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg and Simon Stahlberg
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

#include "mimir/formalism/factories.hpp"

#include <boost/container/small_vector.hpp>
#include <loki/pddl/declarations.hpp>
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

    struct PrepareVisitor
    {
        ITranslator& m_translator;

        explicit PrepareVisitor(ITranslator& translator) : m_translator(translator) {}

        template<typename T>
        void operator()(const T& element) const
        {
            m_translator.prepare(element);
        }
    };

    struct TranslateVisitor
    {
        ITranslator& m_translator;

        explicit TranslateVisitor(ITranslator& translator) : m_translator(translator) {}

        template<typename T>
        auto operator()(const T& element) const
        {
            return m_translator.translate(element);
        }
    };

public:
    /// @brief Collect information.
    template<typename T>
    void prepare(const std::vector<const T*>& vec)
    {
        for (const auto ptr : vec)
        {
            self().prepare(*ptr);
        }
    }
    template<typename T, size_t N>
    void prepare(const boost::container::small_vector<const T*, N>& vec)
    {
        for (const auto ptr : vec)
        {
            self().prepare(*ptr);
        }
    }
    void prepare(const RequirementsImpl& requirements) { self().prepare_base(requirements); }
    void prepare(const TypeImpl& type) { self().prepare_base(type); }
    void prepare(const ObjectImpl& object) { self().prepare_base(object); }
    void prepare(const VariableImpl& variable) { self().prepare_base(variable); }
    void prepare(const TermObjectImpl& term) { self().prepare_base(term); }
    void prepare(const TermVariableImpl& term) { self().prepare_base(term); }
    void prepare(const TermImpl& term) { std::visit(PrepareVisitor(*this), term); }
    void prepare(const ParameterImpl& parameter) { self().prepare_base(parameter); }
    void prepare(const PredicateImpl& predicate) { self().prepare_base(predicate); }
    void prepare(const AtomImpl& atom) { self().prepare_base(atom); }
    void prepare(const GroundAtomImpl& atom) { self().prepare_base(atom); }
    void prepare(const LiteralImpl& literal) { self().prepare_base(literal); }
    void prepare(const NumericFluentImpl& numeric_fluent) { self().prepare_base(numeric_fluent); }
    void prepare(const GroundLiteralImpl& literal) { self().prepare_base(literal); }
    void prepare(const ConditionLiteralImpl& condition) { self().prepare_base(condition); }
    void prepare(const ConditionAndImpl& condition) { self().prepare_base(condition); }
    void prepare(const ConditionOrImpl& condition) { self().prepare_base(condition); }
    void prepare(const ConditionNotImpl& condition) { self().prepare_base(condition); }
    void prepare(const ConditionImplyImpl& condition) { self().prepare_base(condition); }
    void prepare(const ConditionExistsImpl& condition) { self().prepare_base(condition); }
    void prepare(const ConditionForallImpl& condition) { self().prepare_base(condition); }
    void prepare(const ConditionImpl& condition) { std::visit(PrepareVisitor(*this), condition); }
    void prepare(const EffectLiteralImpl& effect) { self().prepare_base(effect); }
    void prepare(const EffectAndImpl& effect) { self().prepare_base(effect); }
    void prepare(const EffectNumericImpl& effect) { self().prepare_base(effect); }
    void prepare(const EffectConditionalForallImpl& effect) { self().prepare_base(effect); }
    void prepare(const EffectConditionalWhenImpl& effect) { self().prepare_base(effect); }
    void prepare(const EffectImpl& effect) { std::visit(PrepareVisitor(*this), effect); }
    void prepare(const FunctionExpressionNumberImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const FunctionExpressionBinaryOperatorImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const FunctionExpressionMultiOperatorImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const FunctionExpressionMinusImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const FunctionExpressionFunctionImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const FunctionExpressionImpl& function_expression) { std::visit(PrepareVisitor(*this), function_expression); }
    void prepare(const FunctionSkeletonImpl& function_skeleton) { self().prepare_base(function_skeleton); }
    void prepare(const FunctionImpl& function) { self().prepare_base(function); }
    void prepare(const ActionImpl& action) { self().prepare_base(action); }
    void prepare(const DerivedPredicateImpl& derived_predicate) { self().prepare_base(derived_predicate); }
    void prepare(const DomainImpl& domain) { self().prepare_base(domain); }
    void prepare(const OptimizationMetricImpl& metric) { self().prepare_base(metric); }
    void prepare(const ProblemImpl& problem) { self().prepare_base(problem); }

    /// @brief Apply problem translation.
    template<typename T>
    auto translate(const std::vector<const T*>& vec)
    {
        std::vector<const T*> result_vec;
        result_vec.reserve(vec.size());
        for (const auto ptr : vec)
        {
            result_vec.push_back(self().translate(*ptr));
        }
        return result_vec;
    }
    template<typename T, size_t N>
    auto translate(const boost::container::small_vector<const T*, N>& vec)
    {
        boost::container::small_vector<const T*, N> result_vec;
        result_vec.reserve(vec.size());
        for (const auto ptr : vec)
        {
            result_vec.push_back(self().translate(*ptr));
        }
        return result_vec;
    }
    Requirements translate(const RequirementsImpl& requirements) { return self().translate_base(requirements); }
    Type translate(const TypeImpl& type) { return self().translate_base(type); }
    Object translate(const ObjectImpl& object) { return self().translate_base(object); }
    Variable translate(const VariableImpl& variable) { return self().translate_base(variable); }
    Term translate(const TermObjectImpl& term) { return self().translate_base(term); }
    Term translate(const TermVariableImpl& term) { return self().translate_base(term); }
    Term translate(const TermImpl& term) { return std::visit(TranslateVisitor(*this), term); }
    Parameter translate(const ParameterImpl& parameter) { return self().translate_base(parameter); }
    Predicate translate(const PredicateImpl& predicate) { return self().translate_base(predicate); }
    Atom translate(const AtomImpl& atom) { return self().translate_base(atom); }
    GroundAtom translate(const GroundAtomImpl& atom) { return self().translate_base(atom); }
    Literal translate(const LiteralImpl& literal) { return self().translate_base(literal); }
    GroundLiteral translate(const GroundLiteralImpl& literal) { return self().translate_base(literal); }
    NumericFluent translate(const NumericFluentImpl& numeric_fluent) { return self().translate_base(numeric_fluent); }
    Condition translate(const ConditionLiteralImpl& condition) { return self().translate_base(condition); }
    Condition translate(const ConditionAndImpl& condition) { return self().translate_base(condition); }
    Condition translate(const ConditionOrImpl& condition) { return self().translate_base(condition); }
    Condition translate(const ConditionNotImpl& condition) { return self().translate_base(condition); }
    Condition translate(const ConditionImplyImpl& condition) { return self().translate_base(condition); }
    Condition translate(const ConditionExistsImpl& condition) { return self().translate_base(condition); }
    Condition translate(const ConditionForallImpl& condition) { return self().translate_base(condition); }
    Condition translate(const ConditionImpl& condition) { return std::visit(TranslateVisitor(*this), condition); }
    Effect translate(const EffectLiteralImpl& effect) { return self().translate_base(effect); }
    Effect translate(const EffectAndImpl& effect) { return self().translate_base(effect); }
    Effect translate(const EffectNumericImpl& effect) { return self().translate_base(effect); }
    Effect translate(const EffectConditionalForallImpl& effect) { return self().translate_base(effect); }
    Effect translate(const EffectConditionalWhenImpl& effect) { return self().translate_base(effect); }
    Effect translate(const EffectImpl& effect) { return std::visit(TranslateVisitor(*this), effect); }
    FunctionExpression translate(const FunctionExpressionNumberImpl& function_expression) { return self().translate_base(function_expression); }
    FunctionExpression translate(const FunctionExpressionBinaryOperatorImpl& function_expression) { return self().translate_base(function_expression); }
    FunctionExpression translate(const FunctionExpressionMultiOperatorImpl& function_expression) { return self().translate_base(function_expression); }
    FunctionExpression translate(const FunctionExpressionMinusImpl& function_expression) { return self().translate_base(function_expression); }
    FunctionExpression translate(const FunctionExpressionFunctionImpl& function_expression) { return self().translate_base(function_expression); }
    FunctionExpression translate(const FunctionExpressionImpl& function_expression) { return std::visit(TranslateVisitor(*this), function_expression); }
    FunctionSkeleton translate(const FunctionSkeletonImpl& function_skeleton) { return self().translate_base(function_skeleton); }
    Function translate(const FunctionImpl& function) { return self().translate_base(function); }
    Action translate(const ActionImpl& action) { return self().translate_base(action); }
    DerivedPredicate translate(const DerivedPredicateImpl& derived_predicate) { return self().translate_base(derived_predicate); }
    Domain translate(const DomainImpl& domain) { return self().translate_base(domain); }
    OptimizationMetric translate(const OptimizationMetricImpl& metric) { return self().translate_base(metric); }
    Problem translate(const ProblemImpl& problem) { return self().translate_base(problem); }

    /// @brief Collect information and apply problem translation.
    Problem run(const ProblemImpl& problem) { return self().run_base(problem); }
};

}

#endif
