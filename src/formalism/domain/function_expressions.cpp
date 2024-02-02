/*
 * Copyright (C) 2023 Dominik Drexler
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

#include <mimir/formalism/domain/function_expressions.hpp>

#include <mimir/formalism/domain/function.hpp>

#include <loki/common/hash.hpp>
#include <loki/common/collections.hpp>
#include <loki/common/pddl/visitors.hpp>

#include <cassert>


namespace mimir 
{
    /* FunctionExpressionNumber */
    FunctionExpressionNumberImpl::FunctionExpressionNumberImpl(int identifier, double number)
        : Base(identifier), m_number(number) { }

    bool FunctionExpressionNumberImpl::is_structurally_equivalent_to_impl(const FunctionExpressionNumberImpl& other) const {
        if (this != &other) {
            return m_number == other.m_number;
        }
        return true;
    }

    size_t FunctionExpressionNumberImpl::hash_impl() const {
        return std::hash<double>()(m_number);
    }

    void FunctionExpressionNumberImpl::str_impl(std::ostringstream& out, const loki::FormattingOptions& /*options*/) const {
        out << m_number;
    }

    double FunctionExpressionNumberImpl::get_number() const {
        return m_number;
    }


    /* FunctionExpressionBinaryOperator */
    FunctionExpressionBinaryOperatorImpl::FunctionExpressionBinaryOperatorImpl(int identifier,
        loki::pddl::BinaryOperatorEnum binary_operator,
        FunctionExpression left_function_expression,
        FunctionExpression right_function_expression)
        : Base(identifier)
        , m_binary_operator(binary_operator)
        , m_left_function_expression(std::move(left_function_expression))
        , m_right_function_expression(std::move(right_function_expression)) { }

    bool FunctionExpressionBinaryOperatorImpl::is_structurally_equivalent_to_impl(const FunctionExpressionBinaryOperatorImpl& other) const {
        if (this != &other) {
            return (m_binary_operator == other.m_binary_operator)
                && (m_left_function_expression == other.m_left_function_expression)
                && (m_right_function_expression == other.m_right_function_expression);
        }
        return true;
    }

    size_t FunctionExpressionBinaryOperatorImpl::hash_impl() const {
        return hash_combine(m_binary_operator, m_left_function_expression, m_right_function_expression);
    }

    void FunctionExpressionBinaryOperatorImpl::str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const {
        out << "(" << to_string(m_binary_operator) << " ";
        std::visit(loki::pddl::StringifyVisitor(out, options), *m_left_function_expression);
        out << " ";
        std::visit(loki::pddl::StringifyVisitor(out, options), *m_right_function_expression);
        out << ")";
    }

    loki::pddl::BinaryOperatorEnum FunctionExpressionBinaryOperatorImpl::get_binary_operator() const {
        return m_binary_operator;
    }

    const FunctionExpression& FunctionExpressionBinaryOperatorImpl::get_left_function_expression() const {
        return m_left_function_expression;
    }

    const FunctionExpression& FunctionExpressionBinaryOperatorImpl::get_right_function_expression() const {
        return m_right_function_expression;
    }


    /* FunctionExpressionMultiOperator */
    FunctionExpressionMultiOperatorImpl::FunctionExpressionMultiOperatorImpl(int identifier,
        loki::pddl::MultiOperatorEnum multi_operator,
        FunctionExpressionList function_expressions)
        : Base(identifier), m_multi_operator(multi_operator), m_function_expressions(function_expressions) { }

    bool FunctionExpressionMultiOperatorImpl::is_structurally_equivalent_to_impl(const FunctionExpressionMultiOperatorImpl& other) const {
        if (this != &other) {
            return (m_multi_operator == other.m_multi_operator)
                && (get_sorted_vector(m_function_expressions) == get_sorted_vector(other.m_function_expressions));
        }
        return true;
    }

    size_t FunctionExpressionMultiOperatorImpl::hash_impl() const {
        return loki::hash_combine(m_multi_operator, loki::hash_container(loki::get_sorted_vector(m_function_expressions)));
    }

    void FunctionExpressionMultiOperatorImpl::str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const {
        out << "(" << to_string(m_multi_operator);
        assert(!m_function_expressions.empty());
        for (const auto& function_expression : m_function_expressions) {
            out << " ";
            std::visit(loki::pddl::StringifyVisitor(out, options), *function_expression);
        }
        out << ")";
    }

    loki::pddl::MultiOperatorEnum FunctionExpressionMultiOperatorImpl::get_multi_operator() const{
        return m_multi_operator;
    }

    const FunctionExpressionList& FunctionExpressionMultiOperatorImpl::get_function_expressions() const {
        return m_function_expressions;
    }


    /* FunctionExpressionMinus */
    FunctionExpressionMinusImpl::FunctionExpressionMinusImpl(int identifier, FunctionExpression function_expression)
        : Base(identifier), m_function_expression(std::move(function_expression)) { }

    bool FunctionExpressionMinusImpl::is_structurally_equivalent_to_impl(const FunctionExpressionMinusImpl& other) const {
        if (this != &other) {
            return m_function_expression == other.m_function_expression;
        }
        return true;
    }

    size_t FunctionExpressionMinusImpl::hash_impl() const {
        return hash_combine(m_function_expression);
    }

    void FunctionExpressionMinusImpl::str_impl(std::ostringstream& out, const loki::FormattingOptions& /*options*/) const {
        out << "(- " << m_function_expression << ")";
    }

    const FunctionExpression& FunctionExpressionMinusImpl::get_function_expression() const {
        return m_function_expression;
    }


    /* FunctionExpressionFunction */
    FunctionExpressionFunctionImpl::FunctionExpressionFunctionImpl(int identifier, Function function)
        : Base(identifier), m_function(std::move(function)) { }

    bool FunctionExpressionFunctionImpl::is_structurally_equivalent_to_impl(const FunctionExpressionFunctionImpl& other) const {
        if (this != &other) {
            return m_function == other.m_function;
        }
        return true;
    }

    size_t FunctionExpressionFunctionImpl::hash_impl() const {
        return hash_combine(m_function);
    }

    void FunctionExpressionFunctionImpl::str_impl(std::ostringstream& out, const loki::FormattingOptions& /*options*/) const {
        out << *m_function;
    }

    const Function& FunctionExpressionFunctionImpl::get_function() const {
        return m_function;
    }
}


namespace std 
{
    bool less<mimir::FunctionExpression>::operator()(
        const mimir::FunctionExpression& left_function_expression,
        const mimir::FunctionExpression& right_function_expression) const {
        return *left_function_expression < *right_function_expression;
    }

    std::size_t hash<mimir::FunctionExpressionNumberImpl>::operator()(const mimir::FunctionExpressionNumberImpl& function_expression) const {
        return function_expression.hash();
    }

    std::size_t hash<mimir::FunctionExpressionBinaryOperatorImpl>::operator()(const mimir::FunctionExpressionBinaryOperatorImpl& function_expression) const {
        return function_expression.hash();
    }

    std::size_t hash<mimir::FunctionExpressionMultiOperatorImpl>::operator()(const mimir::FunctionExpressionMultiOperatorImpl& function_expression) const {
        return function_expression.hash();
    }

    std::size_t hash<mimir::FunctionExpressionMinusImpl>::operator()(const mimir::FunctionExpressionMinusImpl& function_expression) const {
        return function_expression.hash();
    }

    std::size_t hash<mimir::FunctionExpressionFunctionImpl>::operator()(const mimir::FunctionExpressionFunctionImpl& function_expression) const {
        return function_expression.hash();
    }
}
