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

#ifndef MIMIR_FORMALISM_GROUND_FUNCTION_EXPRESSIONS_HPP_
#define MIMIR_FORMALISM_GROUND_FUNCTION_EXPRESSIONS_HPP_

#include "mimir/formalism/ground_function.hpp"

#include <loki/loki.hpp>
#include <string>
#include <variant>
#include <vector>

namespace mimir
{
/**
 * Type alises
 */

class GroundFunctionExpressionNumberImpl;
using GroundFunctionExpressionNumber = const GroundFunctionExpressionNumberImpl*;
class GroundFunctionExpressionBinaryOperatorImpl;
using GroundFunctionExpressionBinaryOperator = const GroundFunctionExpressionBinaryOperatorImpl*;
class GroundFunctionExpressionMultiOperatorImpl;
using GroundFunctionExpressionMultiOperator = const GroundFunctionExpressionMultiOperatorImpl*;
class GroundFunctionExpressionMinusImpl;
using GroundFunctionExpressionMinus = const GroundFunctionExpressionMinusImpl*;
class GroundFunctionExpressionFunctionImpl;
using GroundFunctionExpressionImpl = std::variant<GroundFunctionExpressionNumberImpl,
                                                  GroundFunctionExpressionBinaryOperatorImpl,
                                                  GroundFunctionExpressionMultiOperatorImpl,
                                                  GroundFunctionExpressionMinusImpl,
                                                  GroundFunctionExpressionFunctionImpl>;
using GroundFunctionExpression = const GroundFunctionExpressionImpl*;
using GroundFunctionExpressionList = std::vector<GroundFunctionExpression>;

/* FunctionExpressionNumber */
class GroundFunctionExpressionNumberImpl : public loki::Base<GroundFunctionExpressionNumberImpl>
{
private:
    double m_number;

    // Below: add additional members if needed and initialize them in the constructor

    GroundFunctionExpressionNumberImpl(int identifier, double number);

    // Give access to the constructor.
    friend class loki::PDDLFactory<GroundFunctionExpressionImpl, loki::Hash<GroundFunctionExpressionImpl*>, loki::EqualTo<GroundFunctionExpressionImpl*>>;

    bool is_structurally_equivalent_to_impl(const GroundFunctionExpressionNumberImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<GroundFunctionExpressionNumberImpl>;

public:
    double get_number() const;
};

/* FunctionExpressionBinaryOperator */
class GroundFunctionExpressionBinaryOperatorImpl : public loki::Base<GroundFunctionExpressionBinaryOperatorImpl>
{
private:
    loki::BinaryOperatorEnum m_binary_operator;
    GroundFunctionExpression m_left_function_expression;
    GroundFunctionExpression m_right_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    GroundFunctionExpressionBinaryOperatorImpl(int identifier,
                                               loki::BinaryOperatorEnum binary_operator,
                                               GroundFunctionExpression left_function_expression,
                                               GroundFunctionExpression right_function_expression);

    // Give access to the constructor.
    friend class loki::PDDLFactory<GroundFunctionExpressionImpl, loki::Hash<GroundFunctionExpressionImpl*>, loki::EqualTo<GroundFunctionExpressionImpl*>>;

    bool is_structurally_equivalent_to_impl(const GroundFunctionExpressionBinaryOperatorImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<GroundFunctionExpressionBinaryOperatorImpl>;

public:
    loki::BinaryOperatorEnum get_binary_operator() const;
    const GroundFunctionExpression& get_left_function_expression() const;
    const GroundFunctionExpression& get_right_function_expression() const;
};

/* FunctionExpressionMultiOperator */
class GroundFunctionExpressionMultiOperatorImpl : public loki::Base<GroundFunctionExpressionMultiOperatorImpl>
{
private:
    loki::MultiOperatorEnum m_multi_operator;
    GroundFunctionExpressionList m_function_expressions;

    // Below: add additional members if needed and initialize them in the constructor

    GroundFunctionExpressionMultiOperatorImpl(int identifier, loki::MultiOperatorEnum multi_operator, GroundFunctionExpressionList function_expressions);

    // Give access to the constructor.
    friend class loki::PDDLFactory<GroundFunctionExpressionImpl, loki::Hash<GroundFunctionExpressionImpl*>, loki::EqualTo<GroundFunctionExpressionImpl*>>;

    bool is_structurally_equivalent_to_impl(const GroundFunctionExpressionMultiOperatorImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<GroundFunctionExpressionMultiOperatorImpl>;

public:
    loki::MultiOperatorEnum get_multi_operator() const;
    const GroundFunctionExpressionList& get_function_expressions() const;
};

/* FunctionExpressionMinus */
class GroundFunctionExpressionMinusImpl : public loki::Base<GroundFunctionExpressionMinusImpl>
{
private:
    GroundFunctionExpression m_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    GroundFunctionExpressionMinusImpl(int identifier, GroundFunctionExpression function_expression);

    // Give access to the constructor.
    friend class loki::PDDLFactory<GroundFunctionExpressionImpl, loki::Hash<GroundFunctionExpressionImpl*>, loki::EqualTo<GroundFunctionExpressionImpl*>>;

    bool is_structurally_equivalent_to_impl(const GroundFunctionExpressionMinusImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<GroundFunctionExpressionMinusImpl>;

public:
    const GroundFunctionExpression& get_function_expression() const;
};

/* FunctionExpressionFunction */
class GroundFunctionExpressionFunctionImpl : public loki::Base<GroundFunctionExpressionFunctionImpl>
{
private:
    GroundFunction m_function;

    // Below: add additional members if needed and initialize them in the constructor

    GroundFunctionExpressionFunctionImpl(int identifier, GroundFunction function);

    // Give access to the constructor.
    friend class loki::PDDLFactory<GroundFunctionExpressionImpl, loki::Hash<GroundFunctionExpressionImpl*>, loki::EqualTo<GroundFunctionExpressionImpl*>>;

    bool is_structurally_equivalent_to_impl(const GroundFunctionExpressionFunctionImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<GroundFunctionExpressionFunctionImpl>;

public:
    const GroundFunction& get_function() const;
};

}

#endif
