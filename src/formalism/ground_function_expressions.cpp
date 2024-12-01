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
GroundFunctionExpressionFunctionImpl::GroundFunctionExpressionFunctionImpl(Index index, GroundFunction function) :
    m_index(index),
    m_function(std::move(function))
{
}

Index GroundFunctionExpressionFunctionImpl::get_index() const { return m_index; }

const GroundFunction& GroundFunctionExpressionFunctionImpl::get_function() const { return m_function; }

/* GroundFunctionExpression */
GroundFunctionExpressionImpl::GroundFunctionExpressionImpl(size_t index,
                                                           std::variant<GroundFunctionExpressionNumber,
                                                                        GroundFunctionExpressionBinaryOperator,
                                                                        GroundFunctionExpressionMultiOperator,
                                                                        GroundFunctionExpressionMinus,
                                                                        GroundFunctionExpressionFunction> ground_function_expression) :
    m_index(index),
    m_ground_function_expression(ground_function_expression)
{
}

size_t GroundFunctionExpressionImpl::get_index() const { return m_index; }

const std::variant<GroundFunctionExpressionNumber,
                   GroundFunctionExpressionBinaryOperator,
                   GroundFunctionExpressionMultiOperator,
                   GroundFunctionExpressionMinus,
                   GroundFunctionExpressionFunction>&
GroundFunctionExpressionImpl::get_variant() const
{
    return m_ground_function_expression;
}

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

std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionFunctionImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

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
