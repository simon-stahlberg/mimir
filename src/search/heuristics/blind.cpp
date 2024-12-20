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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "mimir/search/heuristics/blind.hpp"

#include "mimir/formalism/action.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/function.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/function_skeleton.hpp"
#include "mimir/formalism/ground_function.hpp"
#include "mimir/formalism/numeric_fluent.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/requirements.hpp"

namespace mimir
{

struct FunctionExpressionBoundsDeterminer
{
    std::unordered_map<FunctionSkeleton, std::pair<double, double>>& m_function_skeleton_bounds;

    FunctionExpressionBoundsDeterminer(std::unordered_map<FunctionSkeleton, std::pair<double, double>>& function_skeleton_bounds) :
        m_function_skeleton_bounds(function_skeleton_bounds)
    {
    }

    std::pair<double, double> operator()(const FunctionExpressionImpl& fexpr)
    {
        return std::visit([this](auto&& arg) -> std::pair<double, double> { return (*this)(*arg); }, fexpr.get_variant());
    }

    std::pair<double, double> operator()(const FunctionExpressionNumberImpl& fexpr)
    {
        const auto number = fexpr.get_number();
        return std::make_pair(number, number);
    }

    std::pair<double, double> operator()(const FunctionExpressionBinaryOperatorImpl& fexpr)
    {
        return evaluate_binary_bounds(fexpr.get_binary_operator(),
                                      (*this)(*fexpr.get_left_function_expression()),
                                      (*this)(*fexpr.get_right_function_expression()));
    }

    std::pair<double, double> operator()(const FunctionExpressionMultiOperatorImpl& fexpr)
    {
        const auto op = fexpr.get_multi_operator();
        return std::accumulate(std::next(fexpr.get_function_expressions().begin()),  // Start from the second expression
                               fexpr.get_function_expressions().end(),
                               (*this)(*fexpr.get_function_expressions().front()),  // Initial bounds
                               [this, op](const auto& bounds, const auto& child_expr) { return evaluate_multi_bounds(op, bounds, (*this)(*child_expr)); });
    }

    std::pair<double, double> operator()(const FunctionExpressionMinusImpl& fexpr)
    {
        const auto [lower, upper] = (*this)(*fexpr.get_function_expression());
        return std::make_pair(upper, lower);
    }

    std::pair<double, double> operator()(const FunctionExpressionFunctionImpl& fexpr)
    {
        return m_function_skeleton_bounds.at(fexpr.get_function()->get_function_skeleton());
    }
};

static std::unordered_map<FunctionSkeleton, std::pair<double, double>> compute_function_skeleton_bounds(const NumericFluentList& numeric_fluents)
{
    auto result = std::unordered_map<FunctionSkeleton, std::pair<double, double>> {};

    for (const auto& numeric_fluent : numeric_fluents)
    {
        const auto function_skeleton = numeric_fluent->get_function()->get_function_skeleton();
        const auto value = numeric_fluent->get_number();

        auto& bounds = result.try_emplace(function_skeleton, std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()).first->second;

        bounds.first = std::min(bounds.first, value);
        bounds.second = std::max(bounds.second, value);
    }

    return result;
}

static double determine_action_cost_lower_bound(Problem problem)
{
    double lower_bound = 1.;  ///< 1.0 is sufficiently large to push goal states to the front.

    auto function_skeleton_bounds = compute_function_skeleton_bounds(problem->get_numeric_fluents());

    for (const auto& action : problem->get_domain()->get_actions())
    {
        {
            const auto [lower, upper] = FunctionExpressionBoundsDeterminer(function_skeleton_bounds)(*action->get_strips_effect()->get_function_expression());
            lower_bound = std::min(lower_bound, lower);
        }
        for (const auto& conditional_effect : action->get_conditional_effects())
        {
            const auto [lower, upper] = FunctionExpressionBoundsDeterminer(function_skeleton_bounds)(*conditional_effect->get_function_expression());
            lower_bound = std::min(lower_bound, lower);
        }
    }

    return std::max(lower_bound, 0.);  ///< cap at zero.
}

BlindHeuristic::BlindHeuristic(Problem problem) :
    m_min_action_cost_value(
        (problem->get_domain()->get_requirements()->test(loki::RequirementEnum::ACTION_COSTS)) ?
            determine_action_cost_lower_bound(problem) :  ///< If action costs is enabled we approximate a lower bound on the possible action cost in a state.
            1.)                                           ///< if :action-costs is disabled, each action has cost 1.0
{
}
}
