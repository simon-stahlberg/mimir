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

#include "mimir/common/bounds.hpp"
#include "mimir/common/types.hpp"
#include "mimir/formalism/declarations.hpp"

namespace mimir::formalism
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
    using FormalismEntity = void;

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
    auto identifying_members() const { return std::tuple(get_number()); }
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
    using FormalismEntity = void;

    // moveable but not copyable
    FunctionExpressionBinaryOperatorImpl(const FunctionExpressionBinaryOperatorImpl& other) = delete;
    FunctionExpressionBinaryOperatorImpl& operator=(const FunctionExpressionBinaryOperatorImpl& other) = delete;
    FunctionExpressionBinaryOperatorImpl(FunctionExpressionBinaryOperatorImpl&& other) = default;
    FunctionExpressionBinaryOperatorImpl& operator=(FunctionExpressionBinaryOperatorImpl&& other) = default;

    Index get_index() const;
    loki::BinaryOperatorEnum get_binary_operator() const;
    FunctionExpression get_left_function_expression() const;
    FunctionExpression get_right_function_expression() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_binary_operator(), get_left_function_expression(), get_right_function_expression()); }
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
    using FormalismEntity = void;

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
    auto identifying_members() const { return std::tuple(get_multi_operator(), std::cref(get_function_expressions())); }
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
    using FormalismEntity = void;

    // moveable but not copyable
    FunctionExpressionMinusImpl(const FunctionExpressionMinusImpl& other) = delete;
    FunctionExpressionMinusImpl& operator=(const FunctionExpressionMinusImpl& other) = delete;
    FunctionExpressionMinusImpl(FunctionExpressionMinusImpl&& other) = default;
    FunctionExpressionMinusImpl& operator=(FunctionExpressionMinusImpl&& other) = default;

    Index get_index() const;
    FunctionExpression get_function_expression() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_function_expression()); }
};

/* FunctionExpressionFunction */
template<IsStaticOrFluentOrAuxiliaryTag F>
class FunctionExpressionFunctionImpl
{
private:
    Index m_index;
    Function<F> m_function;

    // Below: add additional members if needed and initialize them in the constructor

    FunctionExpressionFunctionImpl(Index index, Function<F> function);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    using FormalismEntity = void;

    // moveable but not copyable
    FunctionExpressionFunctionImpl(const FunctionExpressionFunctionImpl& other) = delete;
    FunctionExpressionFunctionImpl& operator=(const FunctionExpressionFunctionImpl& other) = delete;
    FunctionExpressionFunctionImpl(FunctionExpressionFunctionImpl&& other) = default;
    FunctionExpressionFunctionImpl& operator=(FunctionExpressionFunctionImpl&& other) = default;

    Index get_index() const;
    Function<F> get_function() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_function()); }
};

/* FunctionExpression */
// FunctionExpressionFunction<AuxiliaryTag> only appear in numeric effects.
using FunctionExpressionVariant = std::variant<FunctionExpressionNumber,
                                               FunctionExpressionBinaryOperator,
                                               FunctionExpressionMultiOperator,
                                               FunctionExpressionMinus,
                                               FunctionExpressionFunction<StaticTag>,
                                               FunctionExpressionFunction<FluentTag>>;

class FunctionExpressionImpl
{
private:
    Index m_index;
    FunctionExpressionVariant m_function_expression;

    FunctionExpressionImpl(Index index, FunctionExpressionVariant function_expression);

    // Below: add additional members if needed and initialize them in the constructor

    // TODO: we might want to store a vector indicating the argument positions relevant to the function expression to compress the keys in the grounding tables.

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    using FormalismEntity = void;

    // moveable but not copyable
    FunctionExpressionImpl(const FunctionExpressionImpl& other) = delete;
    FunctionExpressionImpl& operator=(const FunctionExpressionImpl& other) = delete;
    FunctionExpressionImpl(FunctionExpressionImpl&& other) = default;
    FunctionExpressionImpl& operator=(FunctionExpressionImpl&& other) = default;

    Index get_index() const;
    const FunctionExpressionVariant& get_variant() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_variant()); }
};

/**
 * Arithmetic operations
 */

inline ContinuousCost evaluate_binary(loki::BinaryOperatorEnum op, ContinuousCost val_left, ContinuousCost val_right)
{
    if (val_left == UNDEFINED_CONTINUOUS_COST || val_right == UNDEFINED_CONTINUOUS_COST)
    {
        return UNDEFINED_CONTINUOUS_COST;
    }

    switch (op)
    {
        case loki::BinaryOperatorEnum::DIV:
        {
            if (val_right == 0.)
                return UNDEFINED_CONTINUOUS_COST;

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

inline ContinuousCost evaluate_multi(loki::MultiOperatorEnum op, ContinuousCost val_left, ContinuousCost val_right)
{
    if (val_left == UNDEFINED_CONTINUOUS_COST || val_right == UNDEFINED_CONTINUOUS_COST)
    {
        return UNDEFINED_CONTINUOUS_COST;
    }

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

template<IsArithmetic A>
inline Bounds<A> evaluate_binary_bounds(loki::BinaryOperatorEnum op, const Bounds<A>& lhs, const Bounds<A>& rhs)
{
    const auto alternative1 = evaluate_binary(op, lhs.get_lower(), rhs.get_lower());
    const auto alternative2 = evaluate_binary(op, lhs.get_upper(), rhs.get_lower());
    const auto alternative3 = evaluate_binary(op, lhs.get_lower(), rhs.get_upper());
    const auto alternative4 = evaluate_binary(op, lhs.get_upper(), rhs.get_upper());
    auto result = std::minmax({ alternative1, alternative2, alternative3, alternative4 });
    return Bounds<A>(result.first, result.second);
}

template<IsArithmetic A>
inline Bounds<A> evaluate_multi_bounds(loki::MultiOperatorEnum op, const Bounds<A>& lhs, const Bounds<A>& rhs)
{
    const auto alternative1 = evaluate_multi(op, lhs.get_lower(), rhs.get_lower());
    const auto alternative2 = evaluate_multi(op, lhs.get_upper(), rhs.get_lower());
    const auto alternative3 = evaluate_multi(op, lhs.get_lower(), rhs.get_upper());
    const auto alternative4 = evaluate_multi(op, lhs.get_upper(), rhs.get_upper());
    auto result = std::minmax({ alternative1, alternative2, alternative3, alternative4 });
    return Bounds<A>(result.first, result.second);
}

extern std::ostream& operator<<(std::ostream& out, const FunctionExpressionNumberImpl& element);
extern std::ostream& operator<<(std::ostream& out, const FunctionExpressionBinaryOperatorImpl& element);
extern std::ostream& operator<<(std::ostream& out, const FunctionExpressionMultiOperatorImpl& element);
extern std::ostream& operator<<(std::ostream& out, const FunctionExpressionMinusImpl& element);
template<IsStaticOrFluentOrAuxiliaryTag F>
extern std::ostream& operator<<(std::ostream& out, const FunctionExpressionFunctionImpl<F>& element);
extern std::ostream& operator<<(std::ostream& out, const FunctionExpressionImpl& element);

extern std::ostream& operator<<(std::ostream& out, FunctionExpressionNumber element);
extern std::ostream& operator<<(std::ostream& out, FunctionExpressionBinaryOperator element);
extern std::ostream& operator<<(std::ostream& out, FunctionExpressionMultiOperator element);
extern std::ostream& operator<<(std::ostream& out, FunctionExpressionMinus element);
template<IsStaticOrFluentOrAuxiliaryTag F>
extern std::ostream& operator<<(std::ostream& out, FunctionExpressionFunction<F> element);
extern std::ostream& operator<<(std::ostream& out, FunctionExpression element);
}

#endif
