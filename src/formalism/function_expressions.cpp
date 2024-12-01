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

#include "mimir/formalism/function_expressions.hpp"

#include "formatter.hpp"
#include "mimir/common/collections.hpp"
#include "mimir/formalism/function.hpp"

#include <cassert>

namespace mimir
{
/* FunctionExpressionNumber */
FunctionExpressionNumberImpl::FunctionExpressionNumberImpl(Index index, double number) : m_index(index), m_number(number) {}

Index FunctionExpressionNumberImpl::get_index() const { return m_index; }

double FunctionExpressionNumberImpl::get_number() const { return m_number; }

/* FunctionExpressionBinaryOperator */
FunctionExpressionBinaryOperatorImpl::FunctionExpressionBinaryOperatorImpl(Index index,
                                                                           loki::BinaryOperatorEnum binary_operator,
                                                                           FunctionExpression left_function_expression,
                                                                           FunctionExpression right_function_expression) :
    m_index(index),
    m_binary_operator(binary_operator),
    m_left_function_expression(std::move(left_function_expression)),
    m_right_function_expression(std::move(right_function_expression))
{
    if (binary_operator == loki::BinaryOperatorEnum::MUL || binary_operator == loki::BinaryOperatorEnum::PLUS)
    {
        assert(m_left_function_expression->get_index() < m_right_function_expression->get_index());
    }
}

Index FunctionExpressionBinaryOperatorImpl::get_index() const { return m_index; }

loki::BinaryOperatorEnum FunctionExpressionBinaryOperatorImpl::get_binary_operator() const { return m_binary_operator; }

const FunctionExpression& FunctionExpressionBinaryOperatorImpl::get_left_function_expression() const { return m_left_function_expression; }

const FunctionExpression& FunctionExpressionBinaryOperatorImpl::get_right_function_expression() const { return m_right_function_expression; }

/* FunctionExpressionMultiOperator */
FunctionExpressionMultiOperatorImpl::FunctionExpressionMultiOperatorImpl(Index index,
                                                                         loki::MultiOperatorEnum multi_operator,
                                                                         FunctionExpressionList function_expressions) :
    m_index(index),
    m_multi_operator(multi_operator),
    m_function_expressions(function_expressions)
{
    assert(is_all_unique(m_function_expressions));
    assert(std::is_sorted(m_function_expressions.begin(),
                          m_function_expressions.end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
}

Index FunctionExpressionMultiOperatorImpl::get_index() const { return m_index; }

loki::MultiOperatorEnum FunctionExpressionMultiOperatorImpl::get_multi_operator() const { return m_multi_operator; }

const FunctionExpressionList& FunctionExpressionMultiOperatorImpl::get_function_expressions() const { return m_function_expressions; }

/* FunctionExpressionMinus */
FunctionExpressionMinusImpl::FunctionExpressionMinusImpl(Index index, FunctionExpression function_expression) :
    m_index(index),
    m_function_expression(std::move(function_expression))
{
}

Index FunctionExpressionMinusImpl::get_index() const { return m_index; }

const FunctionExpression& FunctionExpressionMinusImpl::get_function_expression() const { return m_function_expression; }

/* FunctionExpressionFunction */
FunctionExpressionFunctionImpl::FunctionExpressionFunctionImpl(Index index, Function function) : m_index(index), m_function(std::move(function)) {}

Index FunctionExpressionFunctionImpl::get_index() const { return m_index; }

const Function& FunctionExpressionFunctionImpl::get_function() const { return m_function; }

/* FunctionExpression */
FunctionExpressionImpl::FunctionExpressionImpl(size_t index,
                                               std::variant<FunctionExpressionNumber,
                                                            FunctionExpressionBinaryOperator,
                                                            FunctionExpressionMultiOperator,
                                                            FunctionExpressionMinus,
                                                            FunctionExpressionFunction> function_expression) :
    m_index(index),
    m_function_expression(function_expression)
{
}

size_t FunctionExpressionImpl::get_index() const { return m_index; }

const std::
    variant<FunctionExpressionNumber, FunctionExpressionBinaryOperator, FunctionExpressionMultiOperator, FunctionExpressionMinus, FunctionExpressionFunction>&
    FunctionExpressionImpl::get_variant() const
{
    return m_function_expression;
}

std::ostream& operator<<(std::ostream& out, const FunctionExpressionNumberImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, const FunctionExpressionBinaryOperatorImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, const FunctionExpressionMultiOperatorImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, const FunctionExpressionMinusImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, const FunctionExpressionFunctionImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, const FunctionExpressionImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, FunctionExpression element)
{
    std::visit([&](const auto& arg) { out << *arg; }, element->get_variant());
    return out;
}

}
