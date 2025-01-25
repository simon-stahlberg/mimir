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

namespace mimir
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
    PDDLRepositories& m_pddl_repositories;

    /* Computed in prepare step */

    std::unordered_set<std::string> m_fluent_predicates;   ///< Fluent predicates that appear in an effect
    std::unordered_set<std::string> m_derived_predicates;  ///< Derived predicates

    // Note1: functions that do not appear in a function expression but in an effect are auxiliary.
    // Note2: functions that do not appear in a function expression and not in an effect are irrelevant.
    // Note3: functions that do not appear in an effect are static.
    std::unordered_set<std::string> m_lifted_fexpr_functions;    ///< Functions that appear in a lifted function expression, i.e., numeric effect or  constraint
    std::unordered_set<std::string> m_grounded_fexpr_functions;  ///< Functions that appear in a grounded function expression, i.e., metric
    std::unordered_set<std::string> m_fluent_functions;          ///< Functions that appear in an effect

    // Whether action costs are enabled
    bool m_action_costs_enabled;

    // Equality predicate that does not occur in predicates section
    std::unordered_map<std::string, Predicate<Derived>> m_derived_predicates_by_name;
    Predicate<Static> m_equal_predicate;
    // We allow total-cost to be used in constraints, i.e., it is allowed to be fluent.
    std::variant<FunctionSkeleton<Fluent>, FunctionSkeleton<Auxiliary>> m_total_cost_function;

    GroundFunctionMap<Static, ContinuousCost> m_static_function_to_value;

    /// @brief Prepare all elements in a container.
    template<std::ranges::forward_range Range>
    void prepare_common(const Range& range)
    {
        std::for_each(std::begin(range), std::end(range), [this](auto&& arg) { this->prepare_common(arg); });
    }
    void prepare_common(loki::FunctionSkeleton function_skeleton);
    void prepare_common(loki::Object object);
    void prepare_common(loki::Parameter parameter);
    void prepare_common(loki::Predicate predicate);
    void prepare_common(loki::Requirements requirements);
    void prepare_common(loki::Type type);
    void prepare_common(loki::Variable variable);

    template<std::ranges::forward_range Range>
    void prepare_lifted(const Range& range)
    {
        std::for_each(std::begin(range), std::end(range), [this](auto&& arg) { this->prepare_lifted(arg); });
    }
    void prepare_lifted(loki::Term term);
    void prepare_lifted(loki::Atom atom);
    void prepare_lifted(loki::Literal literal);
    void prepare_lifted(loki::FunctionExpressionNumber function_expression);
    void prepare_lifted(loki::FunctionExpressionBinaryOperator function_expression);
    void prepare_lifted(loki::FunctionExpressionMultiOperator function_expression);
    void prepare_lifted(loki::FunctionExpressionMinus function_expression);
    void prepare_lifted(loki::FunctionExpressionFunction function_expression);
    void prepare_lifted(loki::FunctionExpression function_expression);
    void prepare_lifted(loki::Function function);
    void prepare_lifted(loki::Condition condition);
    void prepare_lifted(loki::Effect effect);
    void prepare_lifted(loki::Action action);
    void prepare_lifted(loki::Axiom axiom);
    void prepare_lifted(loki::Domain domain);

    template<std::ranges::forward_range Range>
    void prepare_grounded(const Range& range)
    {
        std::for_each(std::begin(range), std::end(range), [this](auto&& arg) { this->prepare_grounded(arg); });
    }
    void prepare_grounded(loki::Term term);
    void prepare_grounded(loki::Atom atom);
    void prepare_grounded(loki::Literal literal);
    void prepare_grounded(loki::FunctionExpressionNumber function_expression);
    void prepare_grounded(loki::FunctionExpressionBinaryOperator function_expression);
    void prepare_grounded(loki::FunctionExpressionMultiOperator function_expression);
    void prepare_grounded(loki::FunctionExpressionMinus function_expression);
    void prepare_grounded(loki::FunctionExpressionFunction function_expression);
    void prepare_grounded(loki::FunctionExpression function_expression);
    void prepare_grounded(loki::Function function);
    void prepare_grounded(loki::FunctionValue function_value);
    void prepare_grounded(loki::Condition condition);
    void prepare_grounded(loki::OptimizationMetric metric);
    void prepare_grounded(loki::Problem problem);

    /**
     * Common translations.
     */
    template<typename T>
    auto translate_common(const std::vector<const T*>& input)
    {
        using ReturnType = decltype(this->translate_common(std::declval<const T*>()));
        auto output = std::vector<ReturnType> {};
        output.reserve(input.size());
        std::transform(std::begin(input), std::end(input), std::back_inserter(output), [this](auto&& arg) { return this->translate_common(arg); });
        return output;
    }
    StaticOrFluentOrAuxiliaryFunctionSkeleton translate_common(loki::FunctionSkeleton function_skeleton);
    Object translate_common(loki::Object object);
    Variable translate_common(loki::Parameter parameter);
    StaticOrFluentOrDerivedPredicate translate_common(const loki::Predicate predicate);
    Requirements translate_common(loki::Requirements requirements);
    Variable translate_common(loki::Variable variable);

    /**
     * Lifted translation.
     */

    /// @brief Translate a container of elements into a container of elements.
    template<typename T>
    auto translate_lifted(const std::vector<const T*>& input)
    {
        using ReturnType = decltype(this->translate_lifted(std::declval<const T*>()));
        auto output = std::vector<ReturnType> {};
        output.reserve(input.size());
        std::transform(std::begin(input), std::end(input), std::back_inserter(output), [this](auto&& arg) { return this->translate_lifted(arg); });
        return output;
    }
    Term translate_lifted(loki::Term term);
    StaticOrFluentOrDerivedAtom translate_lifted(loki::Atom atom);
    StaticOrFluentOrDerivedLiteral translate_lifted(loki::Literal literal);
    FunctionExpression translate_lifted(loki::FunctionExpressionNumber function_expression);
    FunctionExpression translate_lifted(loki::FunctionExpressionBinaryOperator function_expression);
    FunctionExpression translate_lifted(loki::FunctionExpressionMultiOperator function_expression);
    FunctionExpression translate_lifted(loki::FunctionExpressionMinus function_expression);
    FunctionExpression translate_lifted(loki::FunctionExpressionFunction function_expression);
    FunctionExpression translate_lifted(loki::FunctionExpression function_expression);
    StaticOrFluentOrAuxiliaryFunction translate_lifted(loki::Function function);
    NumericConstraint translate_lifted(loki::ConditionNumericConstraint condition);
    ConjunctiveCondition translate_lifted(loki::Condition condition, const VariableList& parameters);
    std::tuple<ConjunctiveEffect, ConditionalEffectList> translate_lifted(loki::Effect effect, const VariableList& parameters);
    Action translate_lifted(loki::Action action);
    Axiom translate_lifted(loki::Axiom axiom);
    Domain translate_lifted(loki::Domain domain);

    /**
     * Grounded translation
     */

    template<typename T>
    auto translate_grounded(const std::vector<const T*>& input)
    {
        using ReturnType = decltype(this->translate_grounded(std::declval<const T*>()));
        auto output = std::vector<ReturnType> {};
        output.reserve(input.size());
        std::transform(std::begin(input), std::end(input), std::back_inserter(output), [this](auto&& arg) { return this->translate_grounded(arg); });
        return output;
    }
    Object translate_grounded(loki::Term term);
    StaticOrFluentOrDerivedGroundAtom translate_grounded(loki::Atom atom);
    StaticOrFluentOrDerivedGroundLiteral translate_grounded(loki::Literal literal);
    GroundFunctionExpression translate_grounded(loki::FunctionExpressionNumber function_expression);
    GroundFunctionExpression translate_grounded(loki::FunctionExpressionBinaryOperator function_expression);
    GroundFunctionExpression translate_grounded(loki::FunctionExpressionMultiOperator function_expression);
    GroundFunctionExpression translate_grounded(loki::FunctionExpressionMinus function_expression);
    GroundFunctionExpression translate_grounded(loki::FunctionExpressionFunction function_expression);
    GroundFunctionExpression translate_grounded(loki::FunctionExpression function_expression);
    StaticOrFluentOrAuxiliaryGroundFunction translate_grounded(loki::Function function);
    StaticOrFluentOrAuxiliaryGroundFunctionValue translate_grounded(loki::FunctionValue numeric_fluent);
    GroundNumericConstraint translate_grounded(loki::ConditionNumericConstraint condition);
    std::tuple<GroundLiteralList<Static>, GroundLiteralList<Fluent>, GroundLiteralList<Derived>, GroundNumericConstraintList>
    translate_grounded(loki::Condition condition);
    OptimizationMetric translate_grounded(loki::OptimizationMetric optimization_metric);
    Problem translate_grounded(loki::Problem problem);

public:
    explicit ToMimirStructures(PDDLRepositories& pddl_repositories);

    Problem run(loki::Problem problem);
};

}

#endif