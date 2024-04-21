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

    /// @brief Translate a container of elements into a container of elements.
    template<typename T>
    auto translate(const std::vector<const T*>& input)
    {
        using ReturnType = decltype(this->translate(std::declval<T>()));
        auto output = std::vector<ReturnType> {};
        output.reserve(input.size());
        std::transform(std::begin(input), std::end(input), std::back_inserter(output), [this](auto&& arg) { return this->translate(*arg); });
        return output;
    }
    template<typename T, size_t N>
    auto translate(const boost::container::small_vector<const T*, N>& input)
    {
        using ReturnType = decltype(this->translate(std::declval<T>()));
        auto output = boost::container::small_vector<ReturnType, N> {};
        output.reserve(input.size());
        std::transform(std::begin(input), std::end(input), std::back_inserter(output), [this](auto&& arg) { return this->translate(*arg); });
        return output;
    }
    Requirements translate(const loki::RequirementsImpl& requirements);
    Variable translate(const loki::VariableImpl& variable);
    Object translate(const loki::ObjectImpl& object);
    Term translate(const loki::TermVariableImpl& term);
    Term translate(const loki::TermObjectImpl& term);
    Term translate(const loki::TermImpl& term);
    Parameter translate(const loki::ParameterImpl& parameter);
    Predicate translate(const loki::PredicateImpl& predicate);
    Atom translate(const loki::AtomImpl& atom);
    GroundAtom translate(const loki::GroundAtomImpl& atom);
    Literal translate(const loki::LiteralImpl& literal);
    GroundLiteral translate(const loki::GroundLiteralImpl& literal);
    NumericFluent translate(const loki::NumericFluentImpl& numeric_fluent);
    FunctionExpression translate(const loki::FunctionExpressionNumberImpl& function_expression);
    FunctionExpression translate(const loki::FunctionExpressionBinaryOperatorImpl& function_expression);
    FunctionExpression translate(const loki::FunctionExpressionMultiOperatorImpl& function_expression);
    FunctionExpression translate(const loki::FunctionExpressionMinusImpl& function_expression);
    FunctionExpression translate(const loki::FunctionExpressionFunctionImpl& function_expression);
    FunctionExpression translate(const loki::FunctionExpressionImpl& function_expression);
    FunctionSkeleton translate(const loki::FunctionSkeletonImpl& function_skeleton);
    Function translate(const loki::FunctionImpl& function);
    std::pair<ParameterList, LiteralList> translate(const loki::ConditionImpl& condition);
    std::pair<EffectList, FunctionExpression> translate(const loki::EffectImpl& effect);
    Action translate(const loki::ActionImpl& action);
    Axiom translate(const loki::AxiomImpl& axiom);
    OptimizationMetric translate(const loki::OptimizationMetricImpl& optimization_metric);
    Domain translate(const loki::DomainImpl& domain);

public:
    explicit ToMimirStructures(PDDLFactories& pddl_factories);

    Problem translate(const loki::ProblemImpl& problem);
};

}

#endif