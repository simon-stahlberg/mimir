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

namespace mimir
{

/* FunctionExpressionNumber */
class FunctionExpressionNumberImpl
{
private:
    Index m_index;
    double m_number;

    // Below: add additional members if needed and initialize them in the constructor

    FunctionExpressionNumberImpl(Index index, double number);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    FunctionExpressionNumberImpl(const FunctionExpressionNumberImpl& other) = delete;
    FunctionExpressionNumberImpl& operator=(const FunctionExpressionNumberImpl& other) = delete;
    FunctionExpressionNumberImpl(FunctionExpressionNumberImpl&& other) = default;
    FunctionExpressionNumberImpl& operator=(FunctionExpressionNumberImpl&& other) = default;

    Index get_index() const;
    double get_number() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifiable_members() const { return std::forward_as_tuple(std::as_const(m_number)); }
};

/* FunctionExpressionBinaryOperator */
class FunctionExpressionBinaryOperatorImpl
{
private:
    Index m_index;
    loki::BinaryOperatorEnum m_binary_operator;
    FunctionExpression m_left_function_expression;
    FunctionExpression m_right_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    FunctionExpressionBinaryOperatorImpl(Index index,
                                         loki::BinaryOperatorEnum binary_operator,
                                         FunctionExpression left_function_expression,
                                         FunctionExpression right_function_expression);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    FunctionExpressionBinaryOperatorImpl(const FunctionExpressionBinaryOperatorImpl& other) = delete;
    FunctionExpressionBinaryOperatorImpl& operator=(const FunctionExpressionBinaryOperatorImpl& other) = delete;
    FunctionExpressionBinaryOperatorImpl(FunctionExpressionBinaryOperatorImpl&& other) = default;
    FunctionExpressionBinaryOperatorImpl& operator=(FunctionExpressionBinaryOperatorImpl&& other) = default;

    Index get_index() const;
    loki::BinaryOperatorEnum get_binary_operator() const;
    const FunctionExpression& get_left_function_expression() const;
    const FunctionExpression& get_right_function_expression() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifiable_members() const
    {
        return std::forward_as_tuple(std::as_const(m_binary_operator), std::as_const(m_left_function_expression), std::as_const(m_right_function_expression));
    }
};

/* FunctionExpressionMultiOperator */
class FunctionExpressionMultiOperatorImpl
{
private:
    Index m_index;
    loki::MultiOperatorEnum m_multi_operator;
    FunctionExpressionList m_function_expressions;

    // Below: add additional members if needed and initialize them in the constructor

    FunctionExpressionMultiOperatorImpl(Index index, loki::MultiOperatorEnum multi_operator, FunctionExpressionList function_expressions);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    FunctionExpressionMultiOperatorImpl(const FunctionExpressionMultiOperatorImpl& other) = delete;
    FunctionExpressionMultiOperatorImpl& operator=(const FunctionExpressionMultiOperatorImpl& other) = delete;
    FunctionExpressionMultiOperatorImpl(FunctionExpressionMultiOperatorImpl&& other) = default;
    FunctionExpressionMultiOperatorImpl& operator=(FunctionExpressionMultiOperatorImpl&& other) = default;

    Index get_index() const;
    loki::MultiOperatorEnum get_multi_operator() const;
    const FunctionExpressionList& get_function_expressions() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifiable_members() const { return std::forward_as_tuple(std::as_const(m_multi_operator), std::as_const(m_function_expressions)); }
};

/* FunctionExpressionMinus */
class FunctionExpressionMinusImpl
{
private:
    Index m_index;
    FunctionExpression m_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    FunctionExpressionMinusImpl(Index index, FunctionExpression function_expression);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    FunctionExpressionMinusImpl(const FunctionExpressionMinusImpl& other) = delete;
    FunctionExpressionMinusImpl& operator=(const FunctionExpressionMinusImpl& other) = delete;
    FunctionExpressionMinusImpl(FunctionExpressionMinusImpl&& other) = default;
    FunctionExpressionMinusImpl& operator=(FunctionExpressionMinusImpl&& other) = default;

    Index get_index() const;
    const FunctionExpression& get_function_expression() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifiable_members() const { return std::forward_as_tuple(std::as_const(m_function_expression)); }
};

/* FunctionExpressionFunction */
class FunctionExpressionFunctionImpl
{
private:
    Index m_index;
    Function m_function;

    // Below: add additional members if needed and initialize them in the constructor

    FunctionExpressionFunctionImpl(Index index, Function function);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    FunctionExpressionFunctionImpl(const FunctionExpressionFunctionImpl& other) = delete;
    FunctionExpressionFunctionImpl& operator=(const FunctionExpressionFunctionImpl& other) = delete;
    FunctionExpressionFunctionImpl(FunctionExpressionFunctionImpl&& other) = default;
    FunctionExpressionFunctionImpl& operator=(FunctionExpressionFunctionImpl&& other) = default;

    Index get_index() const;
    const Function& get_function() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifiable_members() const { return std::forward_as_tuple(std::as_const(m_function)); }
};

/* FunctionExpression */
class FunctionExpressionImpl
{
private:
    size_t m_index;
    std::variant<FunctionExpressionNumber,
                 FunctionExpressionBinaryOperator,
                 FunctionExpressionMultiOperator,
                 FunctionExpressionMinus,
                 FunctionExpressionFunction>
        m_function_expression;

    FunctionExpressionImpl(size_t index,
                           std::variant<FunctionExpressionNumber,
                                        FunctionExpressionBinaryOperator,
                                        FunctionExpressionMultiOperator,
                                        FunctionExpressionMinus,
                                        FunctionExpressionFunction> function_expression);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    FunctionExpressionImpl(const FunctionExpressionImpl& other) = delete;
    FunctionExpressionImpl& operator=(const FunctionExpressionImpl& other) = delete;
    FunctionExpressionImpl(FunctionExpressionImpl&& other) = default;
    FunctionExpressionImpl& operator=(FunctionExpressionImpl&& other) = default;

    size_t get_index() const;
    const std::variant<FunctionExpressionNumber,
                       FunctionExpressionBinaryOperator,
                       FunctionExpressionMultiOperator,
                       FunctionExpressionMinus,
                       FunctionExpressionFunction>&
    get_variant() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifiable_members() const { return std::forward_as_tuple(std::as_const(m_function_expression)); }
};

/**
 * Arithmetic operations
 */

inline double evaluate_binary(loki::BinaryOperatorEnum op, double val_left, double val_right)
{
    switch (op)
    {
        case loki::BinaryOperatorEnum::DIV:
        {
            if (val_right == 0.)
                throw std::logic_error("Division by zero is undefined.");
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

inline double evaluate_multi(loki::MultiOperatorEnum op, double val_left, double val_right)
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

inline std::pair<double, double>
evaluate_binary_bounds(loki::BinaryOperatorEnum op, std::pair<double, double> lower_upper_lhs, std::pair<double, double> lower_upper_rhs)
{
    const auto [lower_lhs, upper_lhs] = lower_upper_lhs;
    const auto [lower_rhs, upper_rhs] = lower_upper_rhs;
    const auto alternative1 = evaluate_binary(op, lower_lhs, lower_rhs);
    const auto alternative2 = evaluate_binary(op, upper_lhs, lower_rhs);
    const auto alternative3 = evaluate_binary(op, lower_lhs, upper_rhs);
    const auto alternative4 = evaluate_binary(op, upper_lhs, upper_rhs);
    return std::minmax({ alternative1, alternative2, alternative3, alternative4 });
}

inline std::pair<double, double>
evaluate_multi_bounds(loki::MultiOperatorEnum op, std::pair<double, double> lower_upper_lhs, std::pair<double, double> lower_upper_rhs)
{
    const auto [lower_lhs, upper_lhs] = lower_upper_lhs;
    const auto [lower_rhs, upper_rhs] = lower_upper_rhs;
    const auto alternative1 = evaluate_multi(op, lower_lhs, lower_rhs);
    const auto alternative2 = evaluate_multi(op, upper_lhs, lower_rhs);
    const auto alternative3 = evaluate_multi(op, lower_lhs, upper_rhs);
    const auto alternative4 = evaluate_multi(op, upper_lhs, upper_rhs);
    return std::minmax({ alternative1, alternative2, alternative3, alternative4 });
}

extern std::ostream& operator<<(std::ostream& out, const FunctionExpressionNumberImpl& element);
extern std::ostream& operator<<(std::ostream& out, const FunctionExpressionBinaryOperatorImpl& element);
extern std::ostream& operator<<(std::ostream& out, const FunctionExpressionMultiOperatorImpl& element);
extern std::ostream& operator<<(std::ostream& out, const FunctionExpressionMinusImpl& element);
extern std::ostream& operator<<(std::ostream& out, const FunctionExpressionFunctionImpl& element);
extern std::ostream& operator<<(std::ostream& out, const FunctionExpressionImpl& element);

extern std::ostream& operator<<(std::ostream& out, FunctionExpression element);
}

#endif
