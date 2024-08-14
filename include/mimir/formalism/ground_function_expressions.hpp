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
#include "mimir/formalism/equal_to.hpp"
#include "mimir/formalism/hash.hpp"

namespace mimir
{

/* FunctionExpressionNumber */
class GroundFunctionExpressionNumberImpl
{
private:
    size_t m_index;
    double m_number;

    // Below: add additional members if needed and initialize them in the constructor

    GroundFunctionExpressionNumberImpl(size_t index, double number);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    size_t get_index() const;
    double get_number() const;
};

/* FunctionExpressionBinaryOperator */
class GroundFunctionExpressionBinaryOperatorImpl
{
private:
    size_t m_index;
    loki::BinaryOperatorEnum m_binary_operator;
    GroundFunctionExpression m_left_function_expression;
    GroundFunctionExpression m_right_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    GroundFunctionExpressionBinaryOperatorImpl(size_t index,
                                               loki::BinaryOperatorEnum binary_operator,
                                               GroundFunctionExpression left_function_expression,
                                               GroundFunctionExpression right_function_expression);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    size_t get_index() const;
    loki::BinaryOperatorEnum get_binary_operator() const;
    const GroundFunctionExpression& get_left_function_expression() const;
    const GroundFunctionExpression& get_right_function_expression() const;
};

/* FunctionExpressionMultiOperator */
class GroundFunctionExpressionMultiOperatorImpl
{
private:
    size_t m_index;
    loki::MultiOperatorEnum m_multi_operator;
    GroundFunctionExpressionList m_function_expressions;

    // Below: add additional members if needed and initialize them in the constructor

    GroundFunctionExpressionMultiOperatorImpl(size_t index, loki::MultiOperatorEnum multi_operator, GroundFunctionExpressionList function_expressions);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    size_t get_index() const;
    loki::MultiOperatorEnum get_multi_operator() const;
    const GroundFunctionExpressionList& get_function_expressions() const;
};

/* FunctionExpressionMinus */
class GroundFunctionExpressionMinusImpl
{
private:
    size_t m_index;
    GroundFunctionExpression m_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    GroundFunctionExpressionMinusImpl(size_t index, GroundFunctionExpression function_expression);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    size_t get_index() const;
    const GroundFunctionExpression& get_function_expression() const;
};

/* FunctionExpressionFunction */
class GroundFunctionExpressionFunctionImpl
{
private:
    size_t m_index;
    GroundFunction m_function;

    // Below: add additional members if needed and initialize them in the constructor

    GroundFunctionExpressionFunctionImpl(size_t index, GroundFunction function);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    size_t get_index() const;
    const GroundFunction& get_function() const;
};

template<>
struct UniquePDDLHasher<const GroundFunctionExpressionNumberImpl&>
{
    size_t operator()(const GroundFunctionExpressionNumberImpl& e) const;
};

template<>
struct UniquePDDLHasher<const GroundFunctionExpressionBinaryOperatorImpl&>
{
    size_t operator()(const GroundFunctionExpressionBinaryOperatorImpl& e) const;
};

template<>
struct UniquePDDLHasher<const GroundFunctionExpressionMultiOperatorImpl&>
{
    size_t operator()(const GroundFunctionExpressionMultiOperatorImpl& e) const;
};

template<>
struct UniquePDDLHasher<const GroundFunctionExpressionMinusImpl&>
{
    size_t operator()(const GroundFunctionExpressionMinusImpl& e) const;
};

template<>
struct UniquePDDLHasher<const GroundFunctionExpressionFunctionImpl&>
{
    size_t operator()(const GroundFunctionExpressionFunctionImpl& e) const;
};

template<>
struct UniquePDDLHasher<const GroundFunctionExpressionImpl*>
{
    size_t operator()(const GroundFunctionExpressionImpl* e) const;
};

template<>
struct UniquePDDLEqualTo<const GroundFunctionExpressionNumberImpl&>
{
    bool operator()(const GroundFunctionExpressionNumberImpl& l, const GroundFunctionExpressionNumberImpl& r) const;
};

template<>
struct UniquePDDLEqualTo<const GroundFunctionExpressionBinaryOperatorImpl&>
{
    bool operator()(const GroundFunctionExpressionBinaryOperatorImpl& l, const GroundFunctionExpressionBinaryOperatorImpl& r) const;
};

template<>
struct UniquePDDLEqualTo<const GroundFunctionExpressionMultiOperatorImpl&>
{
    bool operator()(const GroundFunctionExpressionMultiOperatorImpl& l, const GroundFunctionExpressionMultiOperatorImpl& r) const;
};

template<>
struct UniquePDDLEqualTo<const GroundFunctionExpressionMinusImpl&>
{
    bool operator()(const GroundFunctionExpressionMinusImpl& l, const GroundFunctionExpressionMinusImpl& r) const;
};

template<>
struct UniquePDDLEqualTo<const GroundFunctionExpressionFunctionImpl&>
{
    bool operator()(const GroundFunctionExpressionFunctionImpl& l, const GroundFunctionExpressionFunctionImpl& r) const;
};

template<>
struct UniquePDDLEqualTo<const GroundFunctionExpressionImpl*>
{
    bool operator()(const GroundFunctionExpressionImpl* l, const GroundFunctionExpressionImpl* r) const;
};

extern std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionNumberImpl& element);
extern std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionBinaryOperatorImpl& element);
extern std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionMultiOperatorImpl& element);
extern std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionMinusImpl& element);
extern std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionFunctionImpl& element);
extern std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionImpl& element);

}

#endif
