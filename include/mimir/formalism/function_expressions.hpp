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

#ifndef MIMIR_FORMALISM_FUNCTION_EXPRESSIONS_HPP_
#define MIMIR_FORMALISM_FUNCTION_EXPRESSIONS_HPP_

#include "mimir/formalism/function.hpp"

#include <loki/loki.hpp>
#include <string>
#include <variant>
#include <vector>

namespace mimir
{
/**
 * Type aliases
 */

class FunctionExpressionNumberImpl;
using FunctionExpressionNumber = const FunctionExpressionNumberImpl*;
class FunctionExpressionBinaryOperatorImpl;
using FunctionExpressionBinaryOperator = const FunctionExpressionBinaryOperatorImpl*;
class FunctionExpressionMultiOperatorImpl;
using FunctionExpressionMultiOperator = const FunctionExpressionMultiOperatorImpl*;
class FunctionExpressionMinusImpl;
using FunctionExpressionMinus = const FunctionExpressionMinusImpl*;
class FunctionExpressionFunctionImpl;
using FunctionExpressionImpl = std::variant<FunctionExpressionNumberImpl,
                                            FunctionExpressionBinaryOperatorImpl,
                                            FunctionExpressionMultiOperatorImpl,
                                            FunctionExpressionMinusImpl,
                                            FunctionExpressionFunctionImpl>;
using FunctionExpression = const FunctionExpressionImpl*;
using FunctionExpressionList = std::vector<FunctionExpression>;

/* FunctionExpressionNumber */
class FunctionExpressionNumberImpl : public loki::Base<FunctionExpressionNumberImpl>
{
private:
    double m_number;

    // Below: add additional members if needed and initialize them in the constructor

    FunctionExpressionNumberImpl(int identifier, double number);

    // Give access to the constructor.
    friend class loki::PDDLFactory<FunctionExpressionImpl, loki::Hash<FunctionExpressionImpl*>, loki::EqualTo<FunctionExpressionImpl*>>;

    bool is_structurally_equivalent_to_impl(const FunctionExpressionNumberImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<FunctionExpressionNumberImpl>;

public:
    double get_number() const;
};

/* FunctionExpressionBinaryOperator */
class FunctionExpressionBinaryOperatorImpl : public loki::Base<FunctionExpressionBinaryOperatorImpl>
{
private:
    loki::BinaryOperatorEnum m_binary_operator;
    FunctionExpression m_left_function_expression;
    FunctionExpression m_right_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    FunctionExpressionBinaryOperatorImpl(int identifier,
                                         loki::BinaryOperatorEnum binary_operator,
                                         FunctionExpression left_function_expression,
                                         FunctionExpression right_function_expression);

    // Give access to the constructor.
    friend class loki::PDDLFactory<FunctionExpressionImpl, loki::Hash<FunctionExpressionImpl*>, loki::EqualTo<FunctionExpressionImpl*>>;

    bool is_structurally_equivalent_to_impl(const FunctionExpressionBinaryOperatorImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<FunctionExpressionBinaryOperatorImpl>;

public:
    loki::BinaryOperatorEnum get_binary_operator() const;
    const FunctionExpression& get_left_function_expression() const;
    const FunctionExpression& get_right_function_expression() const;
};

/* FunctionExpressionMultiOperator */
class FunctionExpressionMultiOperatorImpl : public loki::Base<FunctionExpressionMultiOperatorImpl>
{
private:
    loki::MultiOperatorEnum m_multi_operator;
    FunctionExpressionList m_function_expressions;

    // Below: add additional members if needed and initialize them in the constructor

    FunctionExpressionMultiOperatorImpl(int identifier, loki::MultiOperatorEnum multi_operator, FunctionExpressionList function_expressions);

    // Give access to the constructor.
    friend class loki::PDDLFactory<FunctionExpressionImpl, loki::Hash<FunctionExpressionImpl*>, loki::EqualTo<FunctionExpressionImpl*>>;

    bool is_structurally_equivalent_to_impl(const FunctionExpressionMultiOperatorImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<FunctionExpressionMultiOperatorImpl>;

public:
    loki::MultiOperatorEnum get_multi_operator() const;
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
    friend class loki::PDDLFactory<FunctionExpressionImpl, loki::Hash<FunctionExpressionImpl*>, loki::EqualTo<FunctionExpressionImpl*>>;

    bool is_structurally_equivalent_to_impl(const FunctionExpressionMinusImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

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
    friend class loki::PDDLFactory<FunctionExpressionImpl, loki::Hash<FunctionExpressionImpl*>, loki::EqualTo<FunctionExpressionImpl*>>;

    bool is_structurally_equivalent_to_impl(const FunctionExpressionFunctionImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<FunctionExpressionFunctionImpl>;

public:
    const Function& get_function() const;
};

/**
 * Arithmetic operations
 */

inline double evaluate_binary(const loki::BinaryOperatorEnum& op, const double val_left, const double val_right)
{
    switch (op)
    {
        case loki::BinaryOperatorEnum::DIV:
        {
            return val_left / val_right;
        }
        case loki::BinaryOperatorEnum::MINUS:
        {
            return val_left - val_right;
        }
        case loki::BinaryOperatorEnum::MUL:
        {
            return val_left * val_right;
        }
        case loki::BinaryOperatorEnum::PLUS:
        {
            return val_left + val_right;
        }
        default:
        {
            throw std::logic_error("Evaluation of binary operator is undefined.");
        }
    }
}

inline double evaluate_multi(const loki::MultiOperatorEnum& op, const double val_left, const double val_right)
{
    switch (op)
    {
        case loki::MultiOperatorEnum::MUL:
        {
            return val_left * val_right;
        }
        case loki::MultiOperatorEnum::PLUS:
        {
            return val_left + val_right;
        }
        default:
        {
            throw std::logic_error("Evaluation of multi operator is undefined.");
        }
    }
}

}

#endif
