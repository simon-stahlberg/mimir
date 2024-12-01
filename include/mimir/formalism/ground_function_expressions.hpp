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

#include "mimir/formalism/declarations.hpp"

namespace mimir
{

/* FunctionExpressionNumber */
class GroundFunctionExpressionNumberImpl
{
private:
    Index m_index;
    double m_number;

    // Below: add additional members if needed and initialize them in the constructor

    GroundFunctionExpressionNumberImpl(Index index, double number);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    GroundFunctionExpressionNumberImpl(const GroundFunctionExpressionNumberImpl& other) = delete;
    GroundFunctionExpressionNumberImpl& operator=(const GroundFunctionExpressionNumberImpl& other) = delete;
    GroundFunctionExpressionNumberImpl(GroundFunctionExpressionNumberImpl&& other) = default;
    GroundFunctionExpressionNumberImpl& operator=(GroundFunctionExpressionNumberImpl&& other) = default;

    Index get_index() const;
    double get_number() const;
};

/* FunctionExpressionBinaryOperator */
class GroundFunctionExpressionBinaryOperatorImpl
{
private:
    Index m_index;
    loki::BinaryOperatorEnum m_binary_operator;
    GroundFunctionExpression m_left_function_expression;
    GroundFunctionExpression m_right_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    GroundFunctionExpressionBinaryOperatorImpl(Index index,
                                               loki::BinaryOperatorEnum binary_operator,
                                               GroundFunctionExpression left_function_expression,
                                               GroundFunctionExpression right_function_expression);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    GroundFunctionExpressionBinaryOperatorImpl(const GroundFunctionExpressionBinaryOperatorImpl& other) = delete;
    GroundFunctionExpressionBinaryOperatorImpl& operator=(const GroundFunctionExpressionBinaryOperatorImpl& other) = delete;
    GroundFunctionExpressionBinaryOperatorImpl(GroundFunctionExpressionBinaryOperatorImpl&& other) = default;
    GroundFunctionExpressionBinaryOperatorImpl& operator=(GroundFunctionExpressionBinaryOperatorImpl&& other) = default;

    Index get_index() const;
    loki::BinaryOperatorEnum get_binary_operator() const;
    const GroundFunctionExpression& get_left_function_expression() const;
    const GroundFunctionExpression& get_right_function_expression() const;
};

/* FunctionExpressionMultiOperator */
class GroundFunctionExpressionMultiOperatorImpl
{
private:
    Index m_index;
    loki::MultiOperatorEnum m_multi_operator;
    GroundFunctionExpressionList m_function_expressions;

    // Below: add additional members if needed and initialize them in the constructor

    GroundFunctionExpressionMultiOperatorImpl(Index index, loki::MultiOperatorEnum multi_operator, GroundFunctionExpressionList function_expressions);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    GroundFunctionExpressionMultiOperatorImpl(const GroundFunctionExpressionMultiOperatorImpl& other) = delete;
    GroundFunctionExpressionMultiOperatorImpl& operator=(const GroundFunctionExpressionMultiOperatorImpl& other) = delete;
    GroundFunctionExpressionMultiOperatorImpl(GroundFunctionExpressionMultiOperatorImpl&& other) = default;
    GroundFunctionExpressionMultiOperatorImpl& operator=(GroundFunctionExpressionMultiOperatorImpl&& other) = default;

    Index get_index() const;
    loki::MultiOperatorEnum get_multi_operator() const;
    const GroundFunctionExpressionList& get_function_expressions() const;
};

/* FunctionExpressionMinus */
class GroundFunctionExpressionMinusImpl
{
private:
    Index m_index;
    GroundFunctionExpression m_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    GroundFunctionExpressionMinusImpl(Index index, GroundFunctionExpression function_expression);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    GroundFunctionExpressionMinusImpl(const GroundFunctionExpressionMinusImpl& other) = delete;
    GroundFunctionExpressionMinusImpl& operator=(const GroundFunctionExpressionMinusImpl& other) = delete;
    GroundFunctionExpressionMinusImpl(GroundFunctionExpressionMinusImpl&& other) = default;
    GroundFunctionExpressionMinusImpl& operator=(GroundFunctionExpressionMinusImpl&& other) = default;

    Index get_index() const;
    const GroundFunctionExpression& get_function_expression() const;
};

/* FunctionExpressionFunction */
class GroundFunctionExpressionFunctionImpl
{
private:
    Index m_index;
    GroundFunction m_function;

    // Below: add additional members if needed and initialize them in the constructor

    GroundFunctionExpressionFunctionImpl(Index index, GroundFunction function);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    GroundFunctionExpressionFunctionImpl(const GroundFunctionExpressionFunctionImpl& other) = delete;
    GroundFunctionExpressionFunctionImpl& operator=(const GroundFunctionExpressionFunctionImpl& other) = delete;
    GroundFunctionExpressionFunctionImpl(GroundFunctionExpressionFunctionImpl&& other) = default;
    GroundFunctionExpressionFunctionImpl& operator=(GroundFunctionExpressionFunctionImpl&& other) = default;

    Index get_index() const;
    const GroundFunction& get_function() const;
};

/* GroundFunctionExpression */
class GroundFunctionExpressionImpl
{
private:
    size_t m_index;
    std::variant<GroundFunctionExpressionNumber,
                 GroundFunctionExpressionBinaryOperator,
                 GroundFunctionExpressionMultiOperator,
                 GroundFunctionExpressionMinus,
                 GroundFunctionExpressionFunction>
        m_ground_function_expression;

    GroundFunctionExpressionImpl(size_t index,
                                 std::variant<GroundFunctionExpressionNumber,
                                              GroundFunctionExpressionBinaryOperator,
                                              GroundFunctionExpressionMultiOperator,
                                              GroundFunctionExpressionMinus,
                                              GroundFunctionExpressionFunction> ground_function_expression);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    GroundFunctionExpressionImpl(const GroundFunctionExpressionImpl& other) = delete;
    GroundFunctionExpressionImpl& operator=(const GroundFunctionExpressionImpl& other) = delete;
    GroundFunctionExpressionImpl(GroundFunctionExpressionImpl&& other) = default;
    GroundFunctionExpressionImpl& operator=(GroundFunctionExpressionImpl&& other) = default;

    size_t get_index() const;
    const std::variant<GroundFunctionExpressionNumber,
                       GroundFunctionExpressionBinaryOperator,
                       GroundFunctionExpressionMultiOperator,
                       GroundFunctionExpressionMinus,
                       GroundFunctionExpressionFunction>&
    get_variant() const;
};

extern std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionNumberImpl& element);
extern std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionBinaryOperatorImpl& element);
extern std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionMultiOperatorImpl& element);
extern std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionMinusImpl& element);
extern std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionFunctionImpl& element);
extern std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionImpl& element);

extern std::ostream& operator<<(std::ostream& out, GroundFunctionExpression element);

}

#endif
