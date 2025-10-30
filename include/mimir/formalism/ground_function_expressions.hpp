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

#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"

namespace mimir::formalism
{

/* FunctionExpressionNumber */
class GroundFunctionExpressionNumberImpl
{
private:
    Index m_index;
    double m_number;

    // Below: add additional members if needed and initialize them in the constructor

    GroundFunctionExpressionNumberImpl(Index index, double number);

    static auto identifying_args(double number) noexcept { return std::tuple(number); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    using FormalismEntity = void;

    // moveable but not copyable
    GroundFunctionExpressionNumberImpl(const GroundFunctionExpressionNumberImpl& other) = delete;
    GroundFunctionExpressionNumberImpl& operator=(const GroundFunctionExpressionNumberImpl& other) = delete;
    GroundFunctionExpressionNumberImpl(GroundFunctionExpressionNumberImpl&& other) = default;
    GroundFunctionExpressionNumberImpl& operator=(GroundFunctionExpressionNumberImpl&& other) = default;

    Index get_index() const;
    double get_number() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_number()); }
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

    static auto identifying_args(loki::BinaryOperatorEnum binary_operator,
                                 GroundFunctionExpression left_function_expression,
                                 GroundFunctionExpression right_function_expression) noexcept
    {
        return std::tuple(binary_operator, left_function_expression, right_function_expression);
    }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    using FormalismEntity = void;

    // moveable but not copyable
    GroundFunctionExpressionBinaryOperatorImpl(const GroundFunctionExpressionBinaryOperatorImpl& other) = delete;
    GroundFunctionExpressionBinaryOperatorImpl& operator=(const GroundFunctionExpressionBinaryOperatorImpl& other) = delete;
    GroundFunctionExpressionBinaryOperatorImpl(GroundFunctionExpressionBinaryOperatorImpl&& other) = default;
    GroundFunctionExpressionBinaryOperatorImpl& operator=(GroundFunctionExpressionBinaryOperatorImpl&& other) = default;

    Index get_index() const;
    loki::BinaryOperatorEnum get_binary_operator() const;
    GroundFunctionExpression get_left_function_expression() const;
    GroundFunctionExpression get_right_function_expression() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_binary_operator(), get_left_function_expression(), get_right_function_expression()); }
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

    static auto identifying_args(loki::MultiOperatorEnum multi_operator, const GroundFunctionExpressionList& function_expressions) noexcept
    {
        return std::tuple(multi_operator, std::cref(function_expressions));
    }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    using FormalismEntity = void;

    // moveable but not copyable
    GroundFunctionExpressionMultiOperatorImpl(const GroundFunctionExpressionMultiOperatorImpl& other) = delete;
    GroundFunctionExpressionMultiOperatorImpl& operator=(const GroundFunctionExpressionMultiOperatorImpl& other) = delete;
    GroundFunctionExpressionMultiOperatorImpl(GroundFunctionExpressionMultiOperatorImpl&& other) = default;
    GroundFunctionExpressionMultiOperatorImpl& operator=(GroundFunctionExpressionMultiOperatorImpl&& other) = default;

    Index get_index() const;
    loki::MultiOperatorEnum get_multi_operator() const;
    const GroundFunctionExpressionList& get_function_expressions() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_multi_operator(), std::cref(get_function_expressions())); }
};

/* FunctionExpressionMinus */
class GroundFunctionExpressionMinusImpl
{
private:
    Index m_index;
    GroundFunctionExpression m_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    GroundFunctionExpressionMinusImpl(Index index, GroundFunctionExpression function_expression);

    static auto identifying_args(GroundFunctionExpression function_expression) noexcept { return std::tuple(function_expression); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    using FormalismEntity = void;

    // moveable but not copyable
    GroundFunctionExpressionMinusImpl(const GroundFunctionExpressionMinusImpl& other) = delete;
    GroundFunctionExpressionMinusImpl& operator=(const GroundFunctionExpressionMinusImpl& other) = delete;
    GroundFunctionExpressionMinusImpl(GroundFunctionExpressionMinusImpl&& other) = default;
    GroundFunctionExpressionMinusImpl& operator=(GroundFunctionExpressionMinusImpl&& other) = default;

    Index get_index() const;
    GroundFunctionExpression get_function_expression() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_function_expression()); }
};

/* FunctionExpressionFunction */
template<IsStaticOrFluentOrAuxiliaryTag F>
class GroundFunctionExpressionFunctionImpl
{
private:
    Index m_index;
    GroundFunction<F> m_function;

    // Below: add additional members if needed and initialize them in the constructor

    GroundFunctionExpressionFunctionImpl(Index index, GroundFunction<F> function);

    static auto identifying_args(GroundFunction<F> function) noexcept { return std::tuple(function); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    using FormalismEntity = void;

    // moveable but not copyable
    GroundFunctionExpressionFunctionImpl(const GroundFunctionExpressionFunctionImpl& other) = delete;
    GroundFunctionExpressionFunctionImpl& operator=(const GroundFunctionExpressionFunctionImpl& other) = delete;
    GroundFunctionExpressionFunctionImpl(GroundFunctionExpressionFunctionImpl&& other) = default;
    GroundFunctionExpressionFunctionImpl& operator=(GroundFunctionExpressionFunctionImpl&& other) = default;

    Index get_index() const;
    GroundFunction<F> get_function() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_function()); }
};

/* GroundFunctionExpression */
using GroundFunctionExpressionVariant = std::variant<GroundFunctionExpressionNumber,
                                                     GroundFunctionExpressionBinaryOperator,
                                                     GroundFunctionExpressionMultiOperator,
                                                     GroundFunctionExpressionMinus,
                                                     GroundFunctionExpressionFunction<StaticTag>,
                                                     GroundFunctionExpressionFunction<FluentTag>,
                                                     GroundFunctionExpressionFunction<AuxiliaryTag>>;

class GroundFunctionExpressionImpl
{
private:
    Index m_index;
    GroundFunctionExpressionVariant m_ground_function_expression;

    GroundFunctionExpressionImpl(Index index, GroundFunctionExpressionVariant ground_function_expression);

    static auto identifying_args(GroundFunctionExpressionVariant ground_function_expression) noexcept { return std::tuple(ground_function_expression); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    using FormalismEntity = void;

    // moveable but not copyable
    GroundFunctionExpressionImpl(const GroundFunctionExpressionImpl& other) = delete;
    GroundFunctionExpressionImpl& operator=(const GroundFunctionExpressionImpl& other) = delete;
    GroundFunctionExpressionImpl(GroundFunctionExpressionImpl&& other) = default;
    GroundFunctionExpressionImpl& operator=(GroundFunctionExpressionImpl&& other) = default;

    Index get_index() const;
    const GroundFunctionExpressionVariant& get_variant() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_variant()); }
};

/* Utils */

extern ContinuousCost evaluate(GroundFunctionExpression fexpr, const FlatDoubleList& static_numeric_variables, const FlatDoubleList& fluent_numeric_variables);

}

#endif
