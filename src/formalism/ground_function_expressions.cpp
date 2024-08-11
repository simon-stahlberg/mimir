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

#include "mimir/common/collections.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/formalism/ground_function.hpp"

#include <cassert>

namespace mimir
{
/* FunctionExpressionNumber */
GroundFunctionExpressionNumberImpl::GroundFunctionExpressionNumberImpl(size_t index, double number) : Base(index), m_number(number) {}

bool GroundFunctionExpressionNumberImpl::is_structurally_equivalent_to_impl(const GroundFunctionExpressionNumberImpl& other) const
{
    if (this != &other)
    {
        return m_number == other.m_number;
    }
    return true;
}

size_t GroundFunctionExpressionNumberImpl::hash_impl() const { return std::hash<double>()(m_number); }

void GroundFunctionExpressionNumberImpl::str_impl(std::ostream& out, const loki::FormattingOptions& /*options*/) const { out << m_number; }

double GroundFunctionExpressionNumberImpl::get_number() const { return m_number; }

/* FunctionExpressionBinaryOperator */
GroundFunctionExpressionBinaryOperatorImpl::GroundFunctionExpressionBinaryOperatorImpl(size_t index,
                                                                                       loki::BinaryOperatorEnum binary_operator,
                                                                                       GroundFunctionExpression left_function_expression,
                                                                                       GroundFunctionExpression right_function_expression) :
    Base(index),
    m_binary_operator(binary_operator),
    m_left_function_expression(std::move(left_function_expression)),
    m_right_function_expression(std::move(right_function_expression))
{
}

bool GroundFunctionExpressionBinaryOperatorImpl::is_structurally_equivalent_to_impl(const GroundFunctionExpressionBinaryOperatorImpl& other) const
{
    if (this != &other)
    {
        return (m_binary_operator == other.m_binary_operator) && (m_left_function_expression == other.m_left_function_expression)
               && (m_right_function_expression == other.m_right_function_expression);
    }
    return true;
}

size_t GroundFunctionExpressionBinaryOperatorImpl::hash_impl() const
{
    return HashCombiner()(m_binary_operator, m_left_function_expression, m_right_function_expression);
}

void GroundFunctionExpressionBinaryOperatorImpl::str_impl(std::ostream& out, const loki::FormattingOptions& options) const
{
    out << "(" << to_string(m_binary_operator) << " ";
    std::visit(loki::StringifyVisitor(out, options), *m_left_function_expression);
    out << " ";
    std::visit(loki::StringifyVisitor(out, options), *m_right_function_expression);
    out << ")";
}

loki::BinaryOperatorEnum GroundFunctionExpressionBinaryOperatorImpl::get_binary_operator() const { return m_binary_operator; }

const GroundFunctionExpression& GroundFunctionExpressionBinaryOperatorImpl::get_left_function_expression() const { return m_left_function_expression; }

const GroundFunctionExpression& GroundFunctionExpressionBinaryOperatorImpl::get_right_function_expression() const { return m_right_function_expression; }

/* FunctionExpressionMultiOperator */
GroundFunctionExpressionMultiOperatorImpl::GroundFunctionExpressionMultiOperatorImpl(size_t index,
                                                                                     loki::MultiOperatorEnum multi_operator,
                                                                                     GroundFunctionExpressionList function_expressions) :
    Base(index),
    m_multi_operator(multi_operator),
    m_function_expressions(function_expressions)
{
    assert(is_all_unique(m_function_expressions));

    /* Canonize. */
    std::sort(m_function_expressions.begin(),
              m_function_expressions.end(),
              [](const auto& l, const auto& r)
              { return std::visit([](const auto& arg) { return arg.get_index(); }, *l) < std::visit([](const auto& arg) { return arg.get_index(); }, *r); });
}

bool GroundFunctionExpressionMultiOperatorImpl::is_structurally_equivalent_to_impl(const GroundFunctionExpressionMultiOperatorImpl& other) const
{
    if (this != &other)
    {
        return (m_multi_operator == other.m_multi_operator) && (m_function_expressions == other.m_function_expressions);
    }
    return true;
}

size_t GroundFunctionExpressionMultiOperatorImpl::hash_impl() const { return HashCombiner()(m_multi_operator, m_function_expressions); }

void GroundFunctionExpressionMultiOperatorImpl::str_impl(std::ostream& out, const loki::FormattingOptions& options) const
{
    out << "(" << to_string(m_multi_operator);
    assert(!m_function_expressions.empty());
    for (const auto& function_expression : m_function_expressions)
    {
        out << " ";
        std::visit(loki::StringifyVisitor(out, options), *function_expression);
    }
    out << ")";
}

loki::MultiOperatorEnum GroundFunctionExpressionMultiOperatorImpl::get_multi_operator() const { return m_multi_operator; }

const GroundFunctionExpressionList& GroundFunctionExpressionMultiOperatorImpl::get_function_expressions() const { return m_function_expressions; }

/* FunctionExpressionMinus */
GroundFunctionExpressionMinusImpl::GroundFunctionExpressionMinusImpl(size_t index, GroundFunctionExpression function_expression) :
    Base(index),
    m_function_expression(std::move(function_expression))
{
}

bool GroundFunctionExpressionMinusImpl::is_structurally_equivalent_to_impl(const GroundFunctionExpressionMinusImpl& other) const
{
    if (this != &other)
    {
        return m_function_expression == other.m_function_expression;
    }
    return true;
}

size_t GroundFunctionExpressionMinusImpl::hash_impl() const { return HashCombiner()(m_function_expression); }

void GroundFunctionExpressionMinusImpl::str_impl(std::ostream& out, const loki::FormattingOptions& options) const
{
    out << "(- ";
    std::visit(loki::StringifyVisitor(out, options), *m_function_expression);
    out << ")";
}

const GroundFunctionExpression& GroundFunctionExpressionMinusImpl::get_function_expression() const { return m_function_expression; }

/* FunctionExpressionFunction */
GroundFunctionExpressionFunctionImpl::GroundFunctionExpressionFunctionImpl(size_t index, GroundFunction function) : Base(index), m_function(std::move(function))
{
}

bool GroundFunctionExpressionFunctionImpl::is_structurally_equivalent_to_impl(const GroundFunctionExpressionFunctionImpl& other) const
{
    if (this != &other)
    {
        return m_function == other.m_function;
    }
    return true;
}

size_t GroundFunctionExpressionFunctionImpl::hash_impl() const { return HashCombiner()(m_function); }

void GroundFunctionExpressionFunctionImpl::str_impl(std::ostream& out, const loki::FormattingOptions& options) const { m_function->str(out, options); }

const GroundFunction& GroundFunctionExpressionFunctionImpl::get_function() const { return m_function; }
}
