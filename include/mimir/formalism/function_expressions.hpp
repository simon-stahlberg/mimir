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

#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/equal_to.hpp"
#include "mimir/formalism/hash.hpp"

namespace mimir
{

/* FunctionExpressionNumber */
class FunctionExpressionNumberImpl
{
private:
    size_t m_index;
    double m_number;

    // Below: add additional members if needed and initialize them in the constructor

    FunctionExpressionNumberImpl(size_t index, double number);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    size_t get_index() const;
    double get_number() const;
};

/* FunctionExpressionBinaryOperator */
class FunctionExpressionBinaryOperatorImpl
{
private:
    size_t m_index;
    loki::BinaryOperatorEnum m_binary_operator;
    FunctionExpression m_left_function_expression;
    FunctionExpression m_right_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    FunctionExpressionBinaryOperatorImpl(size_t index,
                                         loki::BinaryOperatorEnum binary_operator,
                                         FunctionExpression left_function_expression,
                                         FunctionExpression right_function_expression);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    size_t get_index() const;
    loki::BinaryOperatorEnum get_binary_operator() const;
    const FunctionExpression& get_left_function_expression() const;
    const FunctionExpression& get_right_function_expression() const;
};

/* FunctionExpressionMultiOperator */
class FunctionExpressionMultiOperatorImpl
{
private:
    size_t m_index;
    loki::MultiOperatorEnum m_multi_operator;
    FunctionExpressionList m_function_expressions;

    // Below: add additional members if needed and initialize them in the constructor

    FunctionExpressionMultiOperatorImpl(size_t index, loki::MultiOperatorEnum multi_operator, FunctionExpressionList function_expressions);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    size_t get_index() const;
    loki::MultiOperatorEnum get_multi_operator() const;
    const FunctionExpressionList& get_function_expressions() const;
};

/* FunctionExpressionMinus */
class FunctionExpressionMinusImpl
{
private:
    size_t m_index;
    FunctionExpression m_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    FunctionExpressionMinusImpl(size_t index, FunctionExpression function_expression);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    size_t get_index() const;
    const FunctionExpression& get_function_expression() const;
};

/* FunctionExpressionFunction */
class FunctionExpressionFunctionImpl
{
private:
    size_t m_index;
    Function m_function;

    // Below: add additional members if needed and initialize them in the constructor

    FunctionExpressionFunctionImpl(size_t index, Function function);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    size_t get_index() const;
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

template<>
struct UniquePDDLHasher<const FunctionExpressionNumberImpl&>
{
    size_t operator()(const FunctionExpressionNumberImpl& e) const;
};

template<>
struct UniquePDDLHasher<const FunctionExpressionBinaryOperatorImpl&>
{
    size_t operator()(const FunctionExpressionBinaryOperatorImpl& e) const;
};

template<>
struct UniquePDDLHasher<const FunctionExpressionMultiOperatorImpl&>
{
    size_t operator()(const FunctionExpressionMultiOperatorImpl& e) const;
};

template<>
struct UniquePDDLHasher<const FunctionExpressionMinusImpl&>
{
    size_t operator()(const FunctionExpressionMinusImpl& e) const;
};

template<>
struct UniquePDDLHasher<const FunctionExpressionFunctionImpl&>
{
    size_t operator()(const FunctionExpressionFunctionImpl& e) const;
};

template<>
struct UniquePDDLHasher<const FunctionExpressionImpl*>
{
    size_t operator()(const FunctionExpressionImpl* e) const;
};

template<>
struct UniquePDDLEqualTo<const FunctionExpressionNumberImpl&>
{
    bool operator()(const FunctionExpressionNumberImpl& l, const FunctionExpressionNumberImpl& r) const;
};

template<>
struct UniquePDDLEqualTo<const FunctionExpressionBinaryOperatorImpl&>
{
    bool operator()(const FunctionExpressionBinaryOperatorImpl& l, const FunctionExpressionBinaryOperatorImpl& r) const;
};

template<>
struct UniquePDDLEqualTo<const FunctionExpressionMultiOperatorImpl&>
{
    bool operator()(const FunctionExpressionMultiOperatorImpl& l, const FunctionExpressionMultiOperatorImpl& r) const;
};

template<>
struct UniquePDDLEqualTo<const FunctionExpressionMinusImpl&>
{
    bool operator()(const FunctionExpressionMinusImpl& l, const FunctionExpressionMinusImpl& r) const;
};

template<>
struct UniquePDDLEqualTo<const FunctionExpressionFunctionImpl&>
{
    bool operator()(const FunctionExpressionFunctionImpl& l, const FunctionExpressionFunctionImpl& r) const;
};

template<>
struct UniquePDDLEqualTo<const FunctionExpressionImpl*>
{
    bool operator()(const FunctionExpressionImpl* l, const FunctionExpressionImpl* r) const;
};

extern std::ostream& operator<<(std::ostream& out, const FunctionExpressionNumberImpl& element);
extern std::ostream& operator<<(std::ostream& out, const FunctionExpressionBinaryOperatorImpl& element);
extern std::ostream& operator<<(std::ostream& out, const FunctionExpressionMultiOperatorImpl& element);
extern std::ostream& operator<<(std::ostream& out, const FunctionExpressionMinusImpl& element);
extern std::ostream& operator<<(std::ostream& out, const FunctionExpressionFunctionImpl& element);
extern std::ostream& operator<<(std::ostream& out, const FunctionExpressionImpl& element);
}

#endif
