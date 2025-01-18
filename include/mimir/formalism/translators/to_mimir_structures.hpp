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

#include "mimir/formalism/repositories.hpp"

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

    /// @brief Prepare all elements in a container.
    template<typename Container>
    void prepare_common(const Container& input)
    {
        std::for_each(std::begin(input), std::end(input), [this](auto&& arg) { this->prepare_common(*arg); });
    }
    void prepare_common(const loki::FunctionSkeletonImpl& function_skeleton);
    void prepare_common(const loki::ObjectImpl& object);
    void prepare_common(const loki::ParameterImpl& parameter);
    void prepare_common(const loki::PredicateImpl& predicate);
    void prepare_common(const loki::RequirementsImpl& requirements);
    void prepare_common(const loki::TypeImpl& type);
    void prepare_common(const loki::VariableImpl& variable);

    template<typename Container>
    void prepare_lifted(const Container& input)
    {
        std::for_each(std::begin(input), std::end(input), [this](auto&& arg) { this->prepare_lifted(*arg); });
    }
    void prepare_lifted(const loki::TermImpl& term);
    void prepare_lifted(const loki::AtomImpl& atom);
    void prepare_lifted(const loki::LiteralImpl& literal);
    void prepare_lifted(const loki::FunctionExpressionNumberImpl& function_expression);
    void prepare_lifted(const loki::FunctionExpressionBinaryOperatorImpl& function_expression);
    void prepare_lifted(const loki::FunctionExpressionMultiOperatorImpl& function_expression);
    void prepare_lifted(const loki::FunctionExpressionMinusImpl& function_expression);
    void prepare_lifted(const loki::FunctionExpressionFunctionImpl& function_expression);
    void prepare_lifted(const loki::FunctionExpressionImpl& function_expression);
    void prepare_lifted(const loki::FunctionImpl& function);
    void prepare_lifted(const loki::ConditionImpl& condition);
    void prepare_lifted(const loki::EffectImpl& effect);
    void prepare_lifted(const loki::ActionImpl& action);
    void prepare_lifted(const loki::AxiomImpl& axiom);
    void prepare_lifted(const loki::DomainImpl& domain);

    template<typename Container>
    void prepare_grounded(const Container& input)
    {
        std::for_each(std::begin(input), std::end(input), [this](auto&& arg) { this->prepare_grounded(*arg); });
    }
    void prepare_grounded(const loki::TermImpl& term);
    void prepare_grounded(const loki::AtomImpl& atom);
    void prepare_grounded(const loki::LiteralImpl& literal);
    void prepare_grounded(const loki::FunctionExpressionNumberImpl& function_expression);
    void prepare_grounded(const loki::FunctionExpressionBinaryOperatorImpl& function_expression);
    void prepare_grounded(const loki::FunctionExpressionMultiOperatorImpl& function_expression);
    void prepare_grounded(const loki::FunctionExpressionMinusImpl& function_expression);
    void prepare_grounded(const loki::FunctionExpressionFunctionImpl& function_expression);
    void prepare_grounded(const loki::FunctionExpressionImpl& function_expression);
    void prepare_grounded(const loki::FunctionImpl& function);
    void prepare_grounded(const loki::FunctionValueImpl& function_value);
    void prepare_grounded(const loki::ConditionImpl& condition);
    void prepare_grounded(const loki::OptimizationMetricImpl& metric);
    void prepare_grounded(const loki::ProblemImpl& problem);

    /**
     * Common translations.
     */
    template<typename T>
    auto translate_common(const std::vector<const T*>& input)
    {
        using ReturnType = decltype(this->translate_common(std::declval<T>()));
        auto output = std::vector<ReturnType> {};
        output.reserve(input.size());
        std::transform(std::begin(input), std::end(input), std::back_inserter(output), [this](auto&& arg) { return this->translate_common(*arg); });
        return output;
    }
    StaticOrFluentOrAuxiliaryFunctionSkeleton translate_common(const loki::FunctionSkeletonImpl& function_skeleton);
    Object translate_common(const loki::ObjectImpl& object);
    Variable translate_common(const loki::ParameterImpl& parameter);
    StaticOrFluentOrDerivedPredicate translate_common(const loki::PredicateImpl& predicate);
    Requirements translate_common(const loki::RequirementsImpl& requirements);
    Variable translate_common(const loki::VariableImpl& variable);

    /**
     * Lifted translation.
     */

    /// @brief Translate a container of elements into a container of elements.
    template<typename T>
    auto translate_lifted(const std::vector<const T*>& input)
    {
        using ReturnType = decltype(this->translate_lifted(std::declval<T>()));
        auto output = std::vector<ReturnType> {};
        output.reserve(input.size());
        std::transform(std::begin(input), std::end(input), std::back_inserter(output), [this](auto&& arg) { return this->translate_lifted(*arg); });
        return output;
    }
    Term translate_lifted(const loki::TermImpl& term);
    StaticOrFluentOrDerivedAtom translate_lifted(const loki::AtomImpl& atom);
    StaticOrFluentOrDerivedLiteral translate_lifted(const loki::LiteralImpl& literal);
    FunctionExpression translate_lifted(const loki::FunctionExpressionNumberImpl& function_expression);
    FunctionExpression translate_lifted(const loki::FunctionExpressionBinaryOperatorImpl& function_expression);
    FunctionExpression translate_lifted(const loki::FunctionExpressionMultiOperatorImpl& function_expression);
    FunctionExpression translate_lifted(const loki::FunctionExpressionMinusImpl& function_expression);
    FunctionExpression translate_lifted(const loki::FunctionExpressionFunctionImpl& function_expression);
    FunctionExpression translate_lifted(const loki::FunctionExpressionImpl& function_expression);
    StaticOrFluentOrAuxiliaryFunction translate_lifted(const loki::FunctionImpl& function);
    std::tuple<LiteralList<Static>, LiteralList<Fluent>, LiteralList<Derived>> translate_lifted(const loki::ConditionImpl& condition);
    std::tuple<EffectStrips, EffectConditionalList> translate_lifted(const loki::EffectImpl& effect);
    Action translate_lifted(const loki::ActionImpl& action);
    Axiom translate_lifted(const loki::AxiomImpl& axiom);
    Domain translate_lifted(const loki::DomainImpl& domain);

    /**
     * Grounded translation
     */

    template<typename T>
    auto translate_grounded(const std::vector<const T*>& input)
    {
        using ReturnType = decltype(this->translate_grounded(std::declval<T>()));
        auto output = std::vector<ReturnType> {};
        output.reserve(input.size());
        std::transform(std::begin(input), std::end(input), std::back_inserter(output), [this](auto&& arg) { return this->translate_grounded(*arg); });
        return output;
    }
    Object translate_grounded(const loki::TermImpl& term);
    StaticOrFluentOrDerivedGroundAtom translate_grounded(const loki::AtomImpl& atom);
    StaticOrFluentOrDerivedGroundLiteral translate_grounded(const loki::LiteralImpl& literal);
    GroundFunctionExpression translate_grounded(const loki::FunctionExpressionNumberImpl& function_expression);
    GroundFunctionExpression translate_grounded(const loki::FunctionExpressionBinaryOperatorImpl& function_expression);
    GroundFunctionExpression translate_grounded(const loki::FunctionExpressionMultiOperatorImpl& function_expression);
    GroundFunctionExpression translate_grounded(const loki::FunctionExpressionMinusImpl& function_expression);
    GroundFunctionExpression translate_grounded(const loki::FunctionExpressionFunctionImpl& function_expression);
    GroundFunctionExpression translate_grounded(const loki::FunctionExpressionImpl& function_expression);
    StaticOrFluentOrAuxiliaryGroundFunction translate_grounded(const loki::FunctionImpl& function);
    StaticOrFluentOrAuxiliaryGroundFunctionValue translate_grounded(const loki::FunctionValueImpl& numeric_fluent);
    std::tuple<GroundLiteralList<Static>, GroundLiteralList<Fluent>, GroundLiteralList<Derived>> translate_grounded(const loki::ConditionImpl& condition);
    OptimizationMetric translate_grounded(const loki::OptimizationMetricImpl& optimization_metric);
    Problem translate_grounded(const loki::ProblemImpl& problem);

public:
    explicit ToMimirStructures(PDDLRepositories& pddl_repositories);

    Problem run(const loki::ProblemImpl& problem);
};

}

#endif