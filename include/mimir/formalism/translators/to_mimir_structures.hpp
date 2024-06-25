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

#include "mimir/formalism/factories.hpp"
#include "mimir/formalism/pddl.hpp"

#include <loki/loki.hpp>
#include <unordered_map>
#include <variant>

namespace mimir
{

using StaticOrFluentOrDerivedPredicate = std::variant<Predicate<Static>, Predicate<Fluent>, Predicate<Derived>>;
using StaticOrFluentOrDerivedAtom = std::variant<Atom<Static>, Atom<Fluent>, Atom<Derived>>;
using StaticOrFluentOrDerivedLiteral = std::variant<Literal<Static>, Literal<Fluent>, Literal<Derived>>;

using StaticOrFluentOrDerivedGroundAtom = std::variant<GroundAtom<Static>, GroundAtom<Fluent>, GroundAtom<Derived>>;
using StaticOrFluentOrDerivedGroundLiteral = std::variant<GroundLiteral<Static>, GroundLiteral<Fluent>, GroundLiteral<Derived>>;

class ToMimirStructures
{
private:
    PDDLFactories& m_pddl_factories;

    /* Computed in prepare step */

    // Predicate with an action effect
    std::unordered_set<std::string> m_fluent_predicates;
    // Derived predicates
    std::unordered_set<std::string> m_derived_predicates;

    // Whether action costs are enabled
    bool m_action_costs_enabled;

    // Equality predicate that does not occur in predicates section
    std::unordered_map<std::string, Predicate<Derived>> m_derived_predicates_by_name;
    Predicate<Static> m_equal_predicate;

    /// @brief Prepare all elements in a container.
    template<typename Container>
    void prepare(const Container& input)
    {
        std::for_each(std::begin(input), std::end(input), [this](auto&& arg) { this->prepare(*arg); });
    }
    void prepare(const loki::RequirementsImpl& requirements);
    void prepare(const loki::TypeImpl& type);
    void prepare(const loki::ObjectImpl& object);
    void prepare(const loki::VariableImpl& variable);
    void prepare(const loki::TermObjectImpl& term);
    void prepare(const loki::TermVariableImpl& term);
    void prepare(const loki::TermImpl& term);
    void prepare(const loki::ParameterImpl& parameter);
    void prepare(const loki::PredicateImpl& predicate);
    void prepare(const loki::AtomImpl& atom);
    void prepare(const loki::LiteralImpl& literal);
    void prepare(const loki::NumericFluentImpl& numeric_fluent);
    void prepare(const loki::ConditionLiteralImpl& condition);
    void prepare(const loki::ConditionAndImpl& condition);
    void prepare(const loki::ConditionOrImpl& condition);
    void prepare(const loki::ConditionNotImpl& condition);
    void prepare(const loki::ConditionImplyImpl& condition);
    void prepare(const loki::ConditionExistsImpl& condition);
    void prepare(const loki::ConditionForallImpl& condition);
    void prepare(const loki::ConditionImpl& condition);
    void prepare(const loki::EffectImpl& effect);
    void prepare(const loki::FunctionExpressionNumberImpl& function_expression);
    void prepare(const loki::FunctionExpressionBinaryOperatorImpl& function_expression);
    void prepare(const loki::FunctionExpressionMultiOperatorImpl& function_expression);
    void prepare(const loki::FunctionExpressionMinusImpl& function_expression);
    void prepare(const loki::FunctionExpressionFunctionImpl& function_expression);
    void prepare(const loki::FunctionExpressionImpl& function_expression);
    void prepare(const loki::FunctionSkeletonImpl& function_skeleton);
    void prepare(const loki::FunctionImpl& function);
    void prepare(const loki::ActionImpl& action);
    void prepare(const loki::AxiomImpl& axiom);
    void prepare(const loki::DomainImpl& domain);
    void prepare(const loki::OptimizationMetricImpl& metric);
    void prepare(const loki::ProblemImpl& problem);

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
    VariableList translate_common(const loki::ParameterList& parameters);
    Requirements translate_common(const loki::RequirementsImpl& requirements);
    Object translate_common(const loki::ObjectImpl& object);
    Variable translate_common(const loki::VariableImpl& variable);
    StaticOrFluentOrDerivedPredicate translate_common(const loki::PredicateImpl& predicate);

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
    Term translate_lifted(const loki::TermVariableImpl& term);
    Term translate_lifted(const loki::TermObjectImpl& term);
    Term translate_lifted(const loki::TermImpl& term);
    StaticOrFluentOrDerivedAtom translate_lifted(const loki::AtomImpl& atom);
    StaticOrFluentOrDerivedLiteral translate_lifted(const loki::LiteralImpl& literal);
    FunctionExpression translate_lifted(const loki::FunctionExpressionNumberImpl& function_expression);
    FunctionExpression translate_lifted(const loki::FunctionExpressionBinaryOperatorImpl& function_expression);
    FunctionExpression translate_lifted(const loki::FunctionExpressionMultiOperatorImpl& function_expression);
    FunctionExpression translate_lifted(const loki::FunctionExpressionMinusImpl& function_expression);
    FunctionExpression translate_lifted(const loki::FunctionExpressionFunctionImpl& function_expression);
    FunctionExpression translate_lifted(const loki::FunctionExpressionImpl& function_expression);
    FunctionSkeleton translate_lifted(const loki::FunctionSkeletonImpl& function_skeleton);
    Function translate_lifted(const loki::FunctionImpl& function);
    std::tuple<LiteralList<Static>, LiteralList<Fluent>, LiteralList<Derived>> translate_lifted(const loki::ConditionImpl& condition);
    std::tuple<EffectSimpleList, EffectConditionalList, EffectUniversalList, FunctionExpression> translate_lifted(const loki::EffectImpl& effect);
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
    NumericFluent translate_grounded(const loki::NumericFluentImpl& numeric_fluent);
    GroundFunctionExpression translate_grounded(const loki::FunctionExpressionNumberImpl& function_expression);
    GroundFunctionExpression translate_grounded(const loki::FunctionExpressionBinaryOperatorImpl& function_expression);
    GroundFunctionExpression translate_grounded(const loki::FunctionExpressionMultiOperatorImpl& function_expression);
    GroundFunctionExpression translate_grounded(const loki::FunctionExpressionMinusImpl& function_expression);
    GroundFunctionExpression translate_grounded(const loki::FunctionExpressionFunctionImpl& function_expression);
    GroundFunctionExpression translate_grounded(const loki::FunctionExpressionImpl& function_expression);
    GroundFunction translate_grounded(const loki::FunctionImpl& function);
    std::tuple<GroundLiteralList<Static>, GroundLiteralList<Fluent>, GroundLiteralList<Derived>> translate_grounded(const loki::ConditionImpl& condition);
    OptimizationMetric translate_grounded(const loki::OptimizationMetricImpl& optimization_metric);
    Problem translate_grounded(const loki::ProblemImpl& problem);

public:
    explicit ToMimirStructures(PDDLFactories& pddl_factories);

    Problem run(const loki::ProblemImpl& problem);
};

}

#endif