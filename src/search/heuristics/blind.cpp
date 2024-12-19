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
#include "mimir/formalism/ground_function.hpp"
#include "mimir/formalism/numeric_fluent.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/requirements.hpp"

namespace mimir
{
/*
struct FunctionExpressionBoundsDeterminer
{
std::unordered_map<FunctionSkeleton, double>& m_lowest_value_per_function_skeleton;
std::unordered_map<FunctionSkeleton, double>& m_highest_value_per_function_skeleton;

FunctionExpressionBoundsDeterminer(std::unordered_map<FunctionSkeleton, double>& lowest_value_per_function_skeleton,
                                       std::unordered_map<FunctionSkeleton, double>& highest_value_per_function_skeleton) :
    m_lowest_value_per_function_skeleton(lowest_value_per_function_skeleton),
    m_highest_value_per_function_skeleton(highest_value_per_function_skeleton)
{
}

std::pair<double, double> operator()(const FunctionExpressionImpl& fexpr)
{
    return std::visit([this](auto&& arg) -> std::pair<double, double> { return (*this)(*arg); }, fexpr.get_variant());
}

std::pair<double, double> operator()(const FunctionExpressionNumberImpl& fexpr)
{
    // Is sound!
    const auto number = fexpr.get_number();
    return std::make_pair(number, number);
}

std::pair<double, double> operator()(const FunctionExpressionBinaryOperatorImpl& fexpr)
{
    // Is not sound!
    auto [lower_first, upper_first] = (*this)(*fexpr.get_left_function_expression());
    const auto [lower_second, upper_second] = (*this)(*fexpr.get_right_function_expression());
    switch (fexpr.get_binary_operator())
    {
        case loki::BinaryOperatorEnum::PLUS:
        {
            return std::make_pair(lower_first + lower_second, upper_first + upper_second);
        }
        case loki::BinaryOperatorEnum::MINUS:
        {
            return std::make_pair(lower_first - upper_second, upper_first - lower_second);
        }
        case loki::BinaryOperatorEnum::MUL:
        {
            std::make_pair(lower_first * lower_second, upper_first * upper_second);
        }
        case loki::BinaryOperatorEnum::DIV:
        {
            return std::make_pair(lower_first / upper_second, upper_first / lower_second);
        }
        default:
        {
            throw std::runtime_error("Unexpected loki::BinaryOperatorEnum.");
        }
    }
}

std::pair<double, double> operator()(const FunctionExpressionMultiOperatorImpl& fexpr)
{
    // Multi operator can only be PLUS or MUL
    auto [lower, upper] = (*this)(*fexpr.get_function_expressions().front());
    for (size_t i = 1; i < fexpr.get_function_expressions().size(); ++i)
    {
        auto [lower_next, upper_next] = (*this)(*fexpr.get_function_expressions()[i]);
        lower = evaluate_multi(fexpr.get_multi_operator(), lower, lower_next);
        upper = evaluate_multi(fexpr.get_multi_operator(), upper, upper_next);
    }
    return std::make_pair(lower, upper);
}

std::pair<double, double> operator()(const FunctionExpressionMinusImpl& fexpr)
{
    const auto [lower, upper] = (*this)(*fexpr.get_function_expression());
    return std::make_pair(upper, lower);
}

std::pair<double, double> operator()(const FunctionExpressionFunctionImpl& fexpr)
{
    const auto number = m_lowest_value_per_function_skeleton.at(fexpr.get_function()->get_function_skeleton());
    return std::make_pair(number, number);
}
};
*/

BlindHeuristic::BlindHeuristic(Problem problem) :
    m_min_action_cost_value((problem->get_domain()->get_requirements()->test(loki::RequirementEnum::ACTION_COSTS)) ? 0. : 1.)
{
    if (problem->get_domain()->get_requirements()->test(loki::RequirementEnum::ACTION_COSTS))
    {
        // TODO: try to infer tighter lower bounds on the m_min_action_cost_value
        // The code below currently doesnt work because multiplication is much more complicated
        // due to the fact that an even number of negative numbers results in a positive number...

        /*
        double lower_bound = 1.;

        // Determine lowest and highest value that each function value could ever be.
        auto lowest_value_per_function_skeleton = std::unordered_map<FunctionSkeleton, double>();
        auto highest_value_per_function_skeleton = std::unordered_map<FunctionSkeleton, double>();
        for (const auto& numeric_fluent : problem->get_numeric_fluents())
        {
            const auto function_skeleton = numeric_fluent->get_function()->get_function_skeleton();
            lowest_value_per_function_skeleton.emplace(function_skeleton, std::numeric_limits<double>::infinity());
            highest_value_per_function_skeleton.emplace(function_skeleton, 0.);
        }

        for (const auto& numeric_fluent : problem->get_numeric_fluents())
        {
            const auto function_skeleton = numeric_fluent->get_function()->get_function_skeleton();
            const auto number = numeric_fluent->get_number();
            lowest_value_per_function_skeleton.at(function_skeleton) = std::min(lowest_value_per_function_skeleton.at(function_skeleton), number);
            highest_value_per_function_skeleton.at(function_skeleton) = std::max(highest_value_per_function_skeleton.at(function_skeleton), number);
        }

        // Determine lowest possible action cost value
        for (const auto& action : problem->get_domain()->get_actions())
        {
            {
                const auto [lower, upper] = FunctionExpressionBoundsDeterminer(lowest_value_per_function_skeleton, highest_value_per_function_skeleton)(
                    *action->get_strips_effect()->get_function_expression());
                lower_bound = std::min(lower_bound, lower);
            }
            for (const auto& conditional_effect : action->get_conditional_effects())
            {
                const auto [lower, upper] = FunctionExpressionBoundsDeterminer(lowest_value_per_function_skeleton, highest_value_per_function_skeleton)(
                    *conditional_effect->get_function_expression());
                lower_bound = std::min(lower_bound, lower);
            }
        }

        // Finally cap it at 0.
        lower_bound = std::max(lower_bound, 0.);
        */
    }
}
}
