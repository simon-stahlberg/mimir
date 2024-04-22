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
#include "mimir/formalism/factories.hpp"

#include <loki/loki.hpp>
#include <unordered_map>

namespace mimir
{

class ToMimirStructures
{
private:
    PDDLFactories& m_pddl_factories;

    // Predicate with an action effect
    std::unordered_set<loki::Predicate> m_fluent_predicates;

    // Encode parameter index into variables for grounding
    std::unordered_map<loki::Variable, size_t> m_variable_to_parameter_index;

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
     * Domain specific translation.
     */

    /// @brief Translate a container of elements into a container of elements.
    template<typename T>
    auto translate_domain_specific(const std::vector<const T*>& input)
    {
        using ReturnType = decltype(this->translate_domain_specific(std::declval<T>()));
        auto output = std::vector<ReturnType> {};
        output.reserve(input.size());
        std::transform(std::begin(input), std::end(input), std::back_inserter(output), [this](auto&& arg) { return this->translate_domain_specific(*arg); });
        return output;
    }
    template<typename T, size_t N>
    auto translate_domain_specific(const boost::container::small_vector<const T*, N>& input)
    {
        using ReturnType = decltype(this->translate_domain_specific(std::declval<T>()));
        auto output = boost::container::small_vector<ReturnType, N> {};
        output.reserve(input.size());
        std::transform(std::begin(input), std::end(input), std::back_inserter(output), [this](auto&& arg) { return this->translate_domain_specific(*arg); });
        return output;
    }
    auto translate_domain_specific(const loki::ParameterList& parameters)
    {
        // Map variables to parameter index
        for (size_t i = 0; i < parameters.size(); ++i)
        {
            m_variable_to_parameter_index.emplace(parameters[i]->get_variable(), i);
        }
        auto output = ParameterList {};
        output.reserve(parameters.size());
        std::transform(std::begin(parameters),
                       std::end(parameters),
                       std::back_inserter(output),
                       [this](auto&& arg) { return this->translate_domain_specific(*arg); });
        return output;
    }
    Requirements translate_domain_specific(const loki::RequirementsImpl& requirements);
    Variable translate_domain_specific(const loki::VariableImpl& variable);
    Object translate_domain_specific(const loki::ObjectImpl& object);
    Term translate_domain_specific(const loki::TermVariableImpl& term);
    Term translate_domain_specific(const loki::TermObjectImpl& term);
    Term translate_domain_specific(const loki::TermImpl& term);
    Parameter translate_domain_specific(const loki::ParameterImpl& parameter);
    Predicate translate_domain_specific(const loki::PredicateImpl& predicate);
    Atom translate_domain_specific(const loki::AtomImpl& atom);
    Literal translate_domain_specific(const loki::LiteralImpl& literal);
    FunctionExpression translate_domain_specific(const loki::FunctionExpressionNumberImpl& function_expression);
    FunctionExpression translate_domain_specific(const loki::FunctionExpressionBinaryOperatorImpl& function_expression);
    FunctionExpression translate_domain_specific(const loki::FunctionExpressionMultiOperatorImpl& function_expression);
    FunctionExpression translate_domain_specific(const loki::FunctionExpressionMinusImpl& function_expression);
    FunctionExpression translate_domain_specific(const loki::FunctionExpressionFunctionImpl& function_expression);
    FunctionExpression translate_domain_specific(const loki::FunctionExpressionImpl& function_expression);
    FunctionSkeleton translate_domain_specific(const loki::FunctionSkeletonImpl& function_skeleton);
    Function translate_domain_specific(const loki::FunctionImpl& function);
    std::tuple<ParameterList, LiteralList, LiteralList, LiteralList> translate_domain_specific(const loki::ConditionImpl& condition);
    std::pair<EffectList, FunctionExpression> translate_domain_specific(const loki::EffectImpl& effect);
    Action translate_domain_specific(const loki::ActionImpl& action);
    Axiom translate_domain_specific(const loki::AxiomImpl& axiom);
    Domain translate_domain_specific(const loki::DomainImpl& domain);

    /**
     * Problem specific translation
     *
     * Uses grounded structures.
     */

    template<typename T>
    auto translate_problem_specific(const std::vector<const T*>& input)
    {
        using ReturnType = decltype(this->translate_problem_specific(std::declval<T>()));
        auto output = std::vector<ReturnType> {};
        output.reserve(input.size());
        std::transform(std::begin(input), std::end(input), std::back_inserter(output), [this](auto&& arg) { return this->translate_problem_specific(*arg); });
        return output;
    }
    template<typename T, size_t N>
    auto translate_problem_specific(const boost::container::small_vector<const T*, N>& input)
    {
        using ReturnType = decltype(this->translate_problem_specific(std::declval<T>()));
        auto output = boost::container::small_vector<ReturnType, N> {};
        output.reserve(input.size());
        std::transform(std::begin(input), std::end(input), std::back_inserter(output), [this](auto&& arg) { return this->translate_problem_specific(*arg); });
        return output;
    }
    Requirements translate_problem_specific(const loki::RequirementsImpl& requirements);
    Variable translate_problem_specific(const loki::VariableImpl& variable);
    Object translate_problem_specific(const loki::ObjectImpl& object);
    Parameter translate_problem_specific(const loki::ParameterImpl& parameter);
    Predicate translate_problem_specific(const loki::PredicateImpl& predicate);
    GroundAtom translate_problem_specific(const loki::AtomImpl& atom);
    GroundLiteral translate_problem_specific(const loki::LiteralImpl& literal);
    NumericFluent translate_problem_specific(const loki::NumericFluentImpl& numeric_fluent);
    GroundFunctionExpression translate_problem_specific(const loki::FunctionExpressionNumberImpl& function_expression);
    GroundFunctionExpression translate_problem_specific(const loki::FunctionExpressionBinaryOperatorImpl& function_expression);
    GroundFunctionExpression translate_problem_specific(const loki::FunctionExpressionMultiOperatorImpl& function_expression);
    GroundFunctionExpression translate_problem_specific(const loki::FunctionExpressionMinusImpl& function_expression);
    GroundFunctionExpression translate_problem_specific(const loki::FunctionExpressionFunctionImpl& function_expression);
    GroundFunctionExpression translate_problem_specific(const loki::FunctionExpressionImpl& function_expression);
    GroundFunction translate_problem_specific(const loki::FunctionImpl& function);
    GroundLiteralList translate_problem_specific(const loki::ConditionImpl& condition);
    OptimizationMetric translate_problem_specific(const loki::OptimizationMetricImpl& optimization_metric);
    Problem translate_problem_specific(const loki::ProblemImpl& problem);

public:
    explicit ToMimirStructures(PDDLFactories& pddl_factories);

    Problem run(const loki::ProblemImpl& problem);
};

}

#endif