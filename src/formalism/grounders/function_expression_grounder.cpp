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

#include "mimir/formalism/grounders/function_expression_grounder.hpp"

#include "mimir/common/concepts.hpp"
#include "mimir/formalism/grounders/function_grounder.hpp"
#include "mimir/formalism/repositories.hpp"

namespace mimir
{

FunctionExpressionGrounder::FunctionExpressionGrounder(std::shared_ptr<FunctionGrounder> function_grounder) : m_function_grounder(std::move(function_grounder))
{
}

GroundFunctionExpression FunctionExpressionGrounder::ground(FunctionExpression fexpr, const ObjectList& binding)
{
    const auto problem = m_function_grounder->get_problem();
    auto& pddl_repositories = *m_function_grounder->get_pddl_repositories();

    return std::visit(
        [&](auto&& arg) -> GroundFunctionExpression
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, FunctionExpressionNumber>)
            {
                return pddl_repositories.get_or_create_ground_function_expression(
                    pddl_repositories.get_or_create_ground_function_expression_number(arg->get_number()));
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionBinaryOperator>)
            {
                const auto op = arg->get_binary_operator();
                const auto ground_lhs = ground(arg->get_left_function_expression(), binding);
                const auto ground_rhs = ground(arg->get_right_function_expression(), binding);

                if (std::holds_alternative<GroundFunctionExpressionNumber>(ground_lhs->get_variant())
                    && std::holds_alternative<GroundFunctionExpressionNumber>(ground_rhs->get_variant()))
                {
                    return pddl_repositories.get_or_create_ground_function_expression(pddl_repositories.get_or_create_ground_function_expression_number(
                        evaluate_binary(op,
                                        std::get<GroundFunctionExpressionNumber>(ground_lhs->get_variant())->get_number(),
                                        std::get<GroundFunctionExpressionNumber>(ground_rhs->get_variant())->get_number())));
                }

                return pddl_repositories.get_or_create_ground_function_expression(
                    pddl_repositories.get_or_create_ground_function_expression_binary_operator(op, ground_lhs, ground_rhs));
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionMultiOperator>)
            {
                const auto op = arg->get_multi_operator();
                auto fexpr_numbers = GroundFunctionExpressionList {};
                auto fexpr_others = GroundFunctionExpressionList {};
                for (const auto& child_fexpr : arg->get_function_expressions())
                {
                    const auto ground_child_fexpr = ground(child_fexpr, binding);
                    std::holds_alternative<GroundFunctionExpressionNumber>(ground_child_fexpr->get_variant()) ? fexpr_numbers.push_back(ground_child_fexpr) :
                                                                                                                fexpr_others.push_back(ground_child_fexpr);
                }

                if (!fexpr_numbers.empty())
                {
                    const auto value =
                        std::accumulate(std::next(fexpr_numbers.begin()),  // Start from the second expression
                                        fexpr_numbers.end(),
                                        std::get<GroundFunctionExpressionNumber>(fexpr_numbers.front()->get_variant())->get_number(),  // Initial bounds
                                        [op](const auto& value, const auto& child_expr) {
                                            return evaluate_multi(op, value, std::get<GroundFunctionExpressionNumber>(child_expr->get_variant())->get_number());
                                        });

                    fexpr_others.push_back(
                        pddl_repositories.get_or_create_ground_function_expression(pddl_repositories.get_or_create_ground_function_expression_number(value)));
                }

                return pddl_repositories.get_or_create_ground_function_expression(
                    pddl_repositories.get_or_create_ground_function_expression_multi_operator(op, fexpr_others));
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionMinus>)
            {
                const auto ground_fexpr = ground(arg->get_function_expression(), binding);

                return std::holds_alternative<GroundFunctionExpressionNumber>(ground_fexpr->get_variant()) ?
                           pddl_repositories.get_or_create_ground_function_expression(pddl_repositories.get_or_create_ground_function_expression_number(
                               -std::get<GroundFunctionExpressionNumber>(ground_fexpr->get_variant())->get_number())) :
                           ground_fexpr;
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionFunction<Fluent>>)
            {
                return pddl_repositories.get_or_create_ground_function_expression(
                    pddl_repositories.template get_or_create_ground_function_expression_function<Fluent>(
                        m_function_grounder->ground(arg->get_function(), binding)));
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionFunction<Static>>)
            {
                return pddl_repositories.get_or_create_ground_function_expression(pddl_repositories.get_or_create_ground_function_expression_number(
                    problem->get_function_value<Static>(m_function_grounder->ground(arg->get_function(), binding))));
            }
            else
            {
                static_assert(dependent_false<T>::value,
                              "NumericConstraintGrounder::ground(fexpr, binding): Missing implementation for GroundFunctionExpression type.");
            }
        },
        fexpr->get_variant());
}

const std::shared_ptr<FunctionGrounder>& FunctionExpressionGrounder::get_function_grounder() const { return m_function_grounder; }
}
