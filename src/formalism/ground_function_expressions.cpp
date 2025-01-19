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

#include "mimir/formalism/ground_function_expressions.hpp"

#include "formatter.hpp"
#include "mimir/common/collections.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/ground_function.hpp"

#include <cassert>

namespace mimir
{
/* FunctionExpressionNumber */
GroundFunctionExpressionNumberImpl::GroundFunctionExpressionNumberImpl(Index index, double number) : m_index(index), m_number(number) {}

Index GroundFunctionExpressionNumberImpl::get_index() const { return m_index; }

double GroundFunctionExpressionNumberImpl::get_number() const { return m_number; }

/* FunctionExpressionBinaryOperator */
GroundFunctionExpressionBinaryOperatorImpl::GroundFunctionExpressionBinaryOperatorImpl(Index index,
                                                                                       loki::BinaryOperatorEnum binary_operator,
                                                                                       GroundFunctionExpression left_function_expression,
                                                                                       GroundFunctionExpression right_function_expression) :
    m_index(index),
    m_binary_operator(binary_operator),
    m_left_function_expression(std::move(left_function_expression)),
    m_right_function_expression(std::move(right_function_expression))
{
}

Index GroundFunctionExpressionBinaryOperatorImpl::get_index() const { return m_index; }

loki::BinaryOperatorEnum GroundFunctionExpressionBinaryOperatorImpl::get_binary_operator() const { return m_binary_operator; }

const GroundFunctionExpression& GroundFunctionExpressionBinaryOperatorImpl::get_left_function_expression() const { return m_left_function_expression; }

const GroundFunctionExpression& GroundFunctionExpressionBinaryOperatorImpl::get_right_function_expression() const { return m_right_function_expression; }

/* FunctionExpressionMultiOperator */
GroundFunctionExpressionMultiOperatorImpl::GroundFunctionExpressionMultiOperatorImpl(Index index,
                                                                                     loki::MultiOperatorEnum multi_operator,
                                                                                     GroundFunctionExpressionList function_expressions) :
    m_index(index),
    m_multi_operator(multi_operator),
    m_function_expressions(function_expressions)
{
    assert(is_all_unique(m_function_expressions));
    assert(std::is_sorted(m_function_expressions.begin(),
                          m_function_expressions.end(),
                          [](const auto& l, const auto& r)
                          {
                              return std::visit([](auto&& arg) { return arg->get_index(); }, l->get_variant())
                                     < std::visit([](auto&& arg) { return arg->get_index(); }, r->get_variant());
                          }));
}

Index GroundFunctionExpressionMultiOperatorImpl::get_index() const { return m_index; }

loki::MultiOperatorEnum GroundFunctionExpressionMultiOperatorImpl::get_multi_operator() const { return m_multi_operator; }

const GroundFunctionExpressionList& GroundFunctionExpressionMultiOperatorImpl::get_function_expressions() const { return m_function_expressions; }

/* FunctionExpressionMinus */
GroundFunctionExpressionMinusImpl::GroundFunctionExpressionMinusImpl(Index index, GroundFunctionExpression function_expression) :
    m_index(index),
    m_function_expression(std::move(function_expression))
{
}

Index GroundFunctionExpressionMinusImpl::get_index() const { return m_index; }

const GroundFunctionExpression& GroundFunctionExpressionMinusImpl::get_function_expression() const { return m_function_expression; }

/* FunctionExpressionFunction */
template<FunctionTag F>
GroundFunctionExpressionFunctionImpl<F>::GroundFunctionExpressionFunctionImpl(Index index, GroundFunction<F> function) :
    m_index(index),
    m_function(std::move(function))
{
}

template<FunctionTag F>
Index GroundFunctionExpressionFunctionImpl<F>::get_index() const
{
    return m_index;
}

template<FunctionTag F>
const GroundFunction<F>& GroundFunctionExpressionFunctionImpl<F>::get_function() const
{
    return m_function;
}

template class GroundFunctionExpressionFunctionImpl<Static>;
template class GroundFunctionExpressionFunctionImpl<Fluent>;
template class GroundFunctionExpressionFunctionImpl<Auxiliary>;

/* GroundFunctionExpression */
GroundFunctionExpressionImpl::GroundFunctionExpressionImpl(size_t index, GroundFunctionExpressionVariant ground_function_expression) :
    m_index(index),
    m_ground_function_expression(ground_function_expression)
{
}

size_t GroundFunctionExpressionImpl::get_index() const { return m_index; }

const GroundFunctionExpressionVariant& GroundFunctionExpressionImpl::get_variant() const { return m_ground_function_expression; }

/* Utils */

ContinuousCost evaluate(GroundFunctionExpression fexpr, const FlatDoubleList& fluent_numeric_variables, const FlatDoubleList& auxiliary_numeric_variables)
{
    return std::visit(
        [&](auto&& arg)
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, GroundFunctionExpressionNumber>)
            {
                return arg->get_number();
            }
            else if constexpr (std::is_same_v<T, GroundFunctionExpressionBinaryOperator>)
            {
                return evaluate_binary(arg->get_binary_operator(),
                                       evaluate(arg->get_left_function_expression(), fluent_numeric_variables, auxiliary_numeric_variables),
                                       evaluate(arg->get_right_function_expression(), fluent_numeric_variables, auxiliary_numeric_variables));
            }
            else if constexpr (std::is_same_v<T, GroundFunctionExpressionMultiOperator>)
            {
                return std::accumulate(
                    std::next(arg->get_function_expressions().begin()),  // Start from the second expression
                    arg->get_function_expressions().end(),
                    evaluate(arg->get_function_expressions().front(), fluent_numeric_variables, auxiliary_numeric_variables),  // Initial bounds
                    [&](const auto& value, const auto& child_expr)
                    { return evaluate_multi(arg->get_multi_operator(), value, evaluate(child_expr, fluent_numeric_variables, auxiliary_numeric_variables)); });
            }
            else if constexpr (std::is_same_v<T, GroundFunctionExpressionMinus>)
            {
                return -evaluate(arg->get_function_expression(), fluent_numeric_variables, auxiliary_numeric_variables);
            }
            else if constexpr (std::is_same_v<T, GroundFunctionExpressionFunction<Fluent>>)
            {
                if (arg->get_function()->get_index() >= fluent_numeric_variables.size())
                {
                    throw std::logic_error("evaluate(fexpr, fluent_numeric_variables, auxiliary_numeric_variables): undefined fluent function value "
                                           + to_string(arg->get_function()) + ".");
                }

                return fluent_numeric_variables[arg->get_function()->get_index()];
            }
            else if constexpr (std::is_same_v<T, GroundFunctionExpressionFunction<Auxiliary>>)
            {
                if (arg->get_function()->get_index() >= auxiliary_numeric_variables.size())
                {
                    throw std::logic_error("evaluate(fexpr, fluent_numeric_variables, auxiliary_numeric_variables): undefined auxiliary function value "
                                           + to_string(arg->get_function()) + ".");
                }

                return auxiliary_numeric_variables[arg->get_function()->get_index()];
            }
            else
            {
                static_assert(dependent_false<T>::value, "evaluate(...): Missing implementation for GroundFunctionExpressionFunction type.");
            }
        },
        fexpr->get_variant());
}

ContinuousCost evaluate(GroundFunctionExpression fexpr, const FlatDoubleList& fluent_numeric_variables)
{
    return std::visit(
        [&](auto&& arg) -> ContinuousCost
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, GroundFunctionExpressionNumber>)
            {
                return arg->get_number();
            }
            else if constexpr (std::is_same_v<T, GroundFunctionExpressionBinaryOperator>)
            {
                return evaluate_binary(arg->get_binary_operator(),
                                       evaluate(arg->get_left_function_expression(), fluent_numeric_variables),
                                       evaluate(arg->get_right_function_expression(), fluent_numeric_variables));
            }
            else if constexpr (std::is_same_v<T, GroundFunctionExpressionMultiOperator>)
            {
                return std::accumulate(std::next(arg->get_function_expressions().begin()),  // Start from the second expression
                                       arg->get_function_expressions().end(),
                                       evaluate(arg->get_function_expressions().front(), fluent_numeric_variables),  // Initial bounds
                                       [&](const auto& value, const auto& child_expr)
                                       { return evaluate_multi(arg->get_multi_operator(), value, evaluate(child_expr, fluent_numeric_variables)); });
            }
            else if constexpr (std::is_same_v<T, GroundFunctionExpressionMinus>)
            {
                return -evaluate(arg->get_function_expression(), fluent_numeric_variables);
            }
            else if constexpr (std::is_same_v<T, GroundFunctionExpressionFunction<Fluent>>)
            {
                if (arg->get_function()->get_index() >= fluent_numeric_variables.size())
                {
                    throw std::logic_error("evaluate(fexpr, fluent_numeric_variables, auxiliary_numeric_variables): undefined fluent function value "
                                           + to_string(arg->get_function()) + ".");
                }

                return fluent_numeric_variables[arg->get_function()->get_index()];
            }
            else if constexpr (std::is_same_v<T, GroundFunctionExpressionFunction<Auxiliary>>)
            {
                throw std::logic_error(
                    "evaluate(fexpr, fluent_numeric_variables, auxiliary_numeric_variables): Unexpected GroundFunctionExpressionFunction<Auxiliary>.");
            }
            else
            {
                static_assert(dependent_false<T>::value, "evaluate(...): Missing implementation for GroundFunctionExpressionFunction type.");
            }
        },
        fexpr->get_variant());
}

/* Printing */
std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionNumberImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionBinaryOperatorImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionMultiOperatorImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionMinusImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

template<FunctionTag F>
std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionFunctionImpl<F>& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionFunctionImpl<Static>& element);
template std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionFunctionImpl<Fluent>& element);
template std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionFunctionImpl<Auxiliary>& element);

std::ostream& operator<<(std::ostream& out, GroundFunctionExpressionNumber element)
{
    out << *element;
    return out;
}

std::ostream& operator<<(std::ostream& out, GroundFunctionExpressionBinaryOperator element)
{
    out << *element;
    return out;
}

std::ostream& operator<<(std::ostream& out, GroundFunctionExpressionMultiOperator element)
{
    out << *element;
    return out;
}

std::ostream& operator<<(std::ostream& out, GroundFunctionExpressionMinus element)
{
    out << *element;
    return out;
}

template<FunctionTag F>
std::ostream& operator<<(std::ostream& out, GroundFunctionExpressionFunction<F> element)
{
    out << *element;
    return out;
}

template std::ostream& operator<<(std::ostream& out, GroundFunctionExpressionFunction<Static> element);
template std::ostream& operator<<(std::ostream& out, GroundFunctionExpressionFunction<Fluent> element);
template std::ostream& operator<<(std::ostream& out, GroundFunctionExpressionFunction<Auxiliary> element);

std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, GroundFunctionExpression element)
{
    std::visit([&](const auto& arg) { out << arg; }, element->get_variant());
    return out;
}
}
