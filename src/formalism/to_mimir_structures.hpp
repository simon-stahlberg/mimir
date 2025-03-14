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

#ifndef MIMIR_FORMALISM_TRANSLATORS_TO_MIMIR_STRUCTURES_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_TO_MIMIR_STRUCTURES_HPP_

#include "mimir/formalism/declarations.hpp"

#include <loki/loki.hpp>
#include <tuple>
#include <unordered_map>
#include <variant>

namespace mimir::formalism
{

using StaticOrFluentOrDerivedPredicate = std::variant<Predicate<Static>, Predicate<Fluent>, Predicate<Derived>>;
using StaticOrFluentOrDerivedAtom = std::variant<Atom<Static>, Atom<Fluent>, Atom<Derived>>;
using StaticOrFluentOrDerivedLiteral = std::variant<Literal<Static>, Literal<Fluent>, Literal<Derived>>;

using StaticOrFluentOrDerivedGroundAtom = std::variant<GroundAtom<Static>, GroundAtom<Fluent>, GroundAtom<Derived>>;
using StaticOrFluentOrDerivedGroundLiteral = std::variant<GroundLiteral<Static>, GroundLiteral<Fluent>, GroundLiteral<Derived>>;

using StaticOrFluentOrAuxiliaryFunctionSkeleton = std::variant<FunctionSkeleton<Static>, FunctionSkeleton<Fluent>, FunctionSkeleton<Auxiliary>>;
using StaticOrFluentOrAuxiliaryFunction = std::variant<Function<Static>, Function<Fluent>, Function<Auxiliary>>;

using StaticOrFluentOrAuxiliaryGroundFunction = std::variant<GroundFunction<Static>, GroundFunction<Fluent>, GroundFunction<Auxiliary>>;
using StaticOrFluentOrAuxiliaryGroundFunctionValue = std::variant<GroundFunctionValue<Static>, GroundFunctionValue<Fluent>, GroundFunctionValue<Auxiliary>>;

using StaticOrFluentFunctionExpressionFunction = std::variant<FunctionExpressionFunction<Static>, FunctionExpressionFunction<Fluent>>;

using StaticOrFluentOrAuxiliaryGroundFunctionExpressionFunction =
    std::variant<GroundFunctionExpressionFunction<Static>, GroundFunctionExpressionFunction<Fluent>, GroundFunctionExpressionFunction<Auxiliary>>;

class ToMimirStructures
{
private:
    /* Computed in prepare step */

    // For type analysis of predicates.
    std::unordered_set<std::string> m_fluent_predicates;   ///< Fluent predicates that appear in an effect
    std::unordered_set<std::string> m_derived_predicates;  ///< Derived predicates

    // For type analysis of functions.
    std::unordered_set<std::string> m_fexpr_functions;            ///< Functions that appear in a lifted function expression, i.e., numeric effect or constraint
    std::unordered_set<std::string> m_effect_function_skeletons;  ///< Functions that appear in an effect

    template<std::ranges::forward_range Range>
    void prepare(const Range& range)
    {
        std::for_each(std::begin(range), std::end(range), [&](auto&& arg) { this->prepare(arg); });
    }
    template<typename T>
    void prepare(const std::optional<T>& element)
    {
        if (element.has_value())
        {
            this->prepare(element.value());
        }
    }
    void prepare(loki::FunctionSkeleton function_skeleton);
    void prepare(loki::Object object);
    void prepare(loki::Parameter parameter);
    void prepare(loki::Predicate predicate);
    void prepare(loki::Requirements requirements);
    void prepare(loki::Type type);
    void prepare(loki::Variable variable);
    void prepare(loki::Term term);
    void prepare(loki::Atom atom);
    void prepare(loki::Literal literal);
    void prepare(loki::FunctionExpressionNumber function_expression);
    void prepare(loki::FunctionExpressionBinaryOperator function_expression);
    void prepare(loki::FunctionExpressionMultiOperator function_expression);
    void prepare(loki::FunctionExpressionMinus function_expression);
    void prepare(loki::FunctionExpressionFunction function_expression);
    void prepare(loki::FunctionExpression function_expression);
    void prepare(loki::Function function);
    void prepare(loki::Condition condition);
    void prepare(loki::Effect effect);
    void prepare(loki::Action action);
    void prepare(loki::Axiom axiom);
    void prepare(loki::Domain domain);
    void prepare(loki::FunctionValue function_value);
    void prepare(loki::OptimizationMetric metric);
    void prepare(loki::Problem problem);

    /**
     * Common translations.
     */
    template<typename T>
    auto translate_common(const std::vector<const T*>& input, PDDLRepositories& repositories)
    {
        using ReturnType = decltype(this->translate_common(std::declval<const T*>(), std::declval<PDDLRepositories&>()));
        auto output = std::vector<ReturnType> {};
        output.reserve(input.size());
        std::transform(std::begin(input), std::end(input), std::back_inserter(output), [&](auto&& arg) { return this->translate_common(arg, repositories); });
        return output;
    }
    template<typename T>
    auto translate_common(const std::optional<T>& element, PDDLRepositories& repositories)
    {
        return element.has_value() ? this->translate_common(element.value(), repositories) : std::optional<T> { std::nullopt };
    }
    StaticOrFluentOrAuxiliaryFunctionSkeleton translate_common(loki::FunctionSkeleton function_skeleton, PDDLRepositories& repositories);
    Object translate_common(loki::Object object, PDDLRepositories& repositories);
    Variable translate_common(loki::Parameter parameter, PDDLRepositories& repositories);
    StaticOrFluentOrDerivedPredicate translate_common(loki::Predicate predicate, PDDLRepositories& repositories);
    Requirements translate_common(loki::Requirements requirements, PDDLRepositories& repositories);
    Variable translate_common(loki::Variable variable, PDDLRepositories& repositories);

    /**
     * Lifted translation.
     */

    /// @brief Translate a container of elements into a container of elements.
    template<typename T>
    auto translate_lifted(const std::vector<const T*>& input, PDDLRepositories& repositories)
    {
        using ReturnType = decltype(this->translate_lifted(std::declval<const T*>(), std::declval<PDDLRepositories&>()));
        auto output = std::vector<ReturnType> {};
        output.reserve(input.size());
        std::transform(std::begin(input), std::end(input), std::back_inserter(output), [&](auto&& arg) { return this->translate_lifted(arg, repositories); });
        return output;
    }
    Term translate_lifted(loki::Term term, PDDLRepositories& repositories);
    StaticOrFluentOrDerivedAtom translate_lifted(loki::Atom atom, PDDLRepositories& repositories);
    StaticOrFluentOrDerivedLiteral translate_lifted(loki::Literal literal, PDDLRepositories& repositories);
    FunctionExpression translate_lifted(loki::FunctionExpressionNumber function_expression, PDDLRepositories& repositories);
    FunctionExpression translate_lifted(loki::FunctionExpressionBinaryOperator function_expression, PDDLRepositories& repositories);
    FunctionExpression translate_lifted(loki::FunctionExpressionMultiOperator function_expression, PDDLRepositories& repositories);
    FunctionExpression translate_lifted(loki::FunctionExpressionMinus function_expression, PDDLRepositories& repositories);
    FunctionExpression translate_lifted(loki::FunctionExpressionFunction function_expression, PDDLRepositories& repositories);
    FunctionExpression translate_lifted(loki::FunctionExpression function_expression, PDDLRepositories& repositories);
    StaticOrFluentOrAuxiliaryFunction translate_lifted(loki::Function function, PDDLRepositories& repositories);
    NumericConstraint translate_lifted(loki::ConditionNumericConstraint condition, PDDLRepositories& repositories);
    ConjunctiveCondition translate_lifted(loki::Condition condition, const VariableList& parameters, PDDLRepositories& repositories);
    std::tuple<ConjunctiveEffect, ConditionalEffectList> translate_lifted(loki::Effect effect, const VariableList& parameters, PDDLRepositories& repositories);
    Action translate_lifted(loki::Action action, PDDLRepositories& repositories);
    Axiom translate_lifted(loki::Axiom axiom, PDDLRepositories& repositories);

    /**
     * Grounded translation
     */

    template<typename T>
    auto translate_grounded(const std::vector<const T*>& input, PDDLRepositories& repositories)
    {
        using ReturnType = decltype(this->translate_grounded(std::declval<const T*>(), std::declval<PDDLRepositories&>()));
        auto output = std::vector<ReturnType> {};
        output.reserve(input.size());
        std::transform(std::begin(input), std::end(input), std::back_inserter(output), [&](auto&& arg) { return this->translate_grounded(arg, repositories); });
        return output;
    }
    Object translate_grounded(loki::Term term, PDDLRepositories& repositories);
    StaticOrFluentOrDerivedGroundAtom translate_grounded(loki::Atom atom, PDDLRepositories& repositories);
    StaticOrFluentOrDerivedGroundLiteral translate_grounded(loki::Literal literal, PDDLRepositories& repositories);
    GroundFunctionExpression translate_grounded(loki::FunctionExpressionNumber function_expression, PDDLRepositories& repositories);
    GroundFunctionExpression translate_grounded(loki::FunctionExpressionBinaryOperator function_expression, PDDLRepositories& repositories);
    GroundFunctionExpression translate_grounded(loki::FunctionExpressionMultiOperator function_expression, PDDLRepositories& repositories);
    GroundFunctionExpression translate_grounded(loki::FunctionExpressionMinus function_expression, PDDLRepositories& repositories);
    GroundFunctionExpression translate_grounded(loki::FunctionExpressionFunction function_expression, PDDLRepositories& repositories);
    GroundFunctionExpression translate_grounded(loki::FunctionExpression function_expression, PDDLRepositories& repositories);
    StaticOrFluentOrAuxiliaryGroundFunction translate_grounded(loki::Function function, PDDLRepositories& repositories);
    StaticOrFluentOrAuxiliaryGroundFunctionValue translate_grounded(loki::FunctionValue numeric_fluent, PDDLRepositories& repositories);
    GroundNumericConstraint translate_grounded(loki::ConditionNumericConstraint condition, PDDLRepositories& repositories);
    std::tuple<GroundLiteralList<Static>, GroundLiteralList<Fluent>, GroundLiteralList<Derived>, GroundNumericConstraintList>
    translate_grounded(loki::Condition condition, PDDLRepositories& repositories);
    OptimizationMetric translate_grounded(loki::OptimizationMetric optimization_metric, PDDLRepositories& repositories);

public:
    Domain translate(const loki::Domain& domain, DomainBuilder& builder);

    Problem translate(const loki::Problem& problem, ProblemBuilder& builder);
};

}

#endif