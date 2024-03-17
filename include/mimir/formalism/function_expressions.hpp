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

#ifndef MIMIR_FORMALISM_FUNCTION_EXPRESSIONS_HPP_
#define MIMIR_FORMALISM_FUNCTION_EXPRESSIONS_HPP_

#include "mimir/formalism/declarations.hpp"

#include <loki/pddl/factory.hpp>
#include <loki/pddl/function_expressions.hpp>
#include <string>

namespace mimir
{
/* FunctionExpressionNumber */
class FunctionExpressionNumberImpl : public loki::Base<FunctionExpressionNumberImpl>
{
private:
    double m_number;

    // Below: add additional members if needed and initialize them in the constructor

    FunctionExpressionNumberImpl(int identifier, double number);

    // Give access to the constructor.
    template<typename>
    friend class loki::PDDLFactory;

    bool is_structurally_equivalent_to_impl(const FunctionExpressionNumberImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<FunctionExpressionNumberImpl>;

public:
    double get_number() const;
};

/* FunctionExpressionBinaryOperator */
class FunctionExpressionBinaryOperatorImpl : public loki::Base<FunctionExpressionBinaryOperatorImpl>
{
private:
    loki::pddl::BinaryOperatorEnum m_binary_operator;
    FunctionExpression m_left_function_expression;
    FunctionExpression m_right_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    FunctionExpressionBinaryOperatorImpl(int identifier,
                                         loki::pddl::BinaryOperatorEnum binary_operator,
                                         FunctionExpression left_function_expression,
                                         FunctionExpression right_function_expression);

    // Give access to the constructor.
    template<typename>
    friend class loki::PDDLFactory;

    bool is_structurally_equivalent_to_impl(const FunctionExpressionBinaryOperatorImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<FunctionExpressionBinaryOperatorImpl>;

public:
    loki::pddl::BinaryOperatorEnum get_binary_operator() const;
    const FunctionExpression& get_left_function_expression() const;
    const FunctionExpression& get_right_function_expression() const;
};

/* FunctionExpressionMultiOperator */
class FunctionExpressionMultiOperatorImpl : public loki::Base<FunctionExpressionMultiOperatorImpl>
{
private:
    loki::pddl::MultiOperatorEnum m_multi_operator;
    FunctionExpressionList m_function_expressions;

    // Below: add additional members if needed and initialize them in the constructor

    FunctionExpressionMultiOperatorImpl(int identifier, loki::pddl::MultiOperatorEnum multi_operator, FunctionExpressionList function_expressions);

    // Give access to the constructor.
    template<typename>
    friend class loki::PDDLFactory;

    bool is_structurally_equivalent_to_impl(const FunctionExpressionMultiOperatorImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<FunctionExpressionMultiOperatorImpl>;

public:
    loki::pddl::MultiOperatorEnum get_multi_operator() const;
    const FunctionExpressionList& get_function_expressions() const;
};

/* FunctionExpressionMinus */
class FunctionExpressionMinusImpl : public loki::Base<FunctionExpressionMinusImpl>
{
private:
    FunctionExpression m_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    FunctionExpressionMinusImpl(int identifier, FunctionExpression function_expression);

    // Give access to the constructor.
    template<typename>
    friend class loki::PDDLFactory;

    bool is_structurally_equivalent_to_impl(const FunctionExpressionMinusImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<FunctionExpressionMinusImpl>;

public:
    const FunctionExpression& get_function_expression() const;
};

/* FunctionExpressionFunction */
class FunctionExpressionFunctionImpl : public loki::Base<FunctionExpressionFunctionImpl>
{
private:
    Function m_function;

    // Below: add additional members if needed and initialize them in the constructor

    FunctionExpressionFunctionImpl(int identifier, Function function);

    // Give access to the constructor.
    template<typename>
    friend class loki::PDDLFactory;

    bool is_structurally_equivalent_to_impl(const FunctionExpressionFunctionImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<FunctionExpressionFunctionImpl>;

public:
    const Function& get_function() const;
};
}

#endif
