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

#include "mimir/formalism/ground_function_expressions.hpp"

#include "formatter.hpp"
#include "mimir/common/collections.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/ground_function.hpp"

#include <cassert>

namespace mimir::formalism
{
/* FunctionExpressionNumber */
GroundFunctionExpressionNumberImpl::GroundFunctionExpressionNumberImpl(Index index, double number) : m_index(index), m_number(number) {}

Index GroundFunctionExpressionNumberImpl::get_index() const { return m_index; }

double GroundFunctionExpressionNumberImpl::get_number() const { return m_number; }

/* FunctionExpressionBinaryOperator */
GroundFunctionExpressionBinaryOperatorImpl::GroundFunctionExpressionBinaryOperatorImpl(Index index,
                                                                                       loki::BinaryOperatorEnum binary_operator,
                                                                                       GroundFunctionExpression left_function_expression,
                                                                                       GroundFunctionExpression right_function_expression) :
    m_index(index),
    m_binary_operator(binary_operator),
    m_left_function_expression(std::move(left_function_expression)),
    m_right_function_expression(std::move(right_function_expression))
{
}

Index GroundFunctionExpressionBinaryOperatorImpl::get_index() const { return m_index; }

loki::BinaryOperatorEnum GroundFunctionExpressionBinaryOperatorImpl::get_binary_operator() const { return m_binary_operator; }

GroundFunctionExpression GroundFunctionExpressionBinaryOperatorImpl::get_left_function_expression() const { return m_left_function_expression; }

GroundFunctionExpression GroundFunctionExpressionBinaryOperatorImpl::get_right_function_expression() const { return m_right_function_expression; }

/* FunctionExpressionMultiOperator */
GroundFunctionExpressionMultiOperatorImpl::GroundFunctionExpressionMultiOperatorImpl(Index index,
                                                                                     loki::MultiOperatorEnum multi_operator,
                                                                                     GroundFunctionExpressionList function_expressions) :
    m_index(index),
    m_multi_operator(multi_operator),
    m_function_expressions(function_expressions)
{
    assert(is_all_unique(m_function_expressions));
    assert(std::is_sorted(m_function_expressions.begin(),
                          m_function_expressions.end(),
                          [](const auto& l, const auto& r)
                          {
                              return std::visit([](auto&& arg) { return arg->get_index(); }, l->get_variant())
                                     < std::visit([](auto&& arg) { return arg->get_index(); }, r->get_variant());
                          }));
}

Index GroundFunctionExpressionMultiOperatorImpl::get_index() const { return m_index; }

loki::MultiOperatorEnum GroundFunctionExpressionMultiOperatorImpl::get_multi_operator() const { return m_multi_operator; }

const GroundFunctionExpressionList& GroundFunctionExpressionMultiOperatorImpl::get_function_expressions() const { return m_function_expressions; }

/* FunctionExpressionMinus */
GroundFunctionExpressionMinusImpl::GroundFunctionExpressionMinusImpl(Index index, GroundFunctionExpression function_expression) :
    m_index(index),
    m_function_expression(std::move(function_expression))
{
}

Index GroundFunctionExpressionMinusImpl::get_index() const { return m_index; }

GroundFunctionExpression GroundFunctionExpressionMinusImpl::get_function_expression() const { return m_function_expression; }

/* FunctionExpressionFunction */
template<IsStaticOrFluentOrAuxiliaryTag F>
GroundFunctionExpressionFunctionImpl<F>::GroundFunctionExpressionFunctionImpl(Index index, GroundFunction<F> function) :
    m_index(index),
    m_function(std::move(function))
{
}

template<IsStaticOrFluentOrAuxiliaryTag F>
Index GroundFunctionExpressionFunctionImpl<F>::get_index() const
{
    return m_index;
}

template<IsStaticOrFluentOrAuxiliaryTag F>
GroundFunction<F> GroundFunctionExpressionFunctionImpl<F>::get_function() const
{
    return m_function;
}

template class GroundFunctionExpressionFunctionImpl<StaticTag>;
template class GroundFunctionExpressionFunctionImpl<FluentTag>;
template class GroundFunctionExpressionFunctionImpl<AuxiliaryTag>;

/* GroundFunctionExpression */
GroundFunctionExpressionImpl::GroundFunctionExpressionImpl(Index index, GroundFunctionExpressionVariant ground_function_expression) :
    m_index(index),
    m_ground_function_expression(ground_function_expression)
{
}

Index GroundFunctionExpressionImpl::get_index() const { return m_index; }

const GroundFunctionExpressionVariant& GroundFunctionExpressionImpl::get_variant() const { return m_ground_function_expression; }

/* Utils */

ContinuousCost evaluate(GroundFunctionExpression fexpr, const FlatDoubleList& static_numeric_variables, const FlatDoubleList& fluent_numeric_variables)
{
    return std::visit(
        [&](auto&& arg) -> ContinuousCost
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, GroundFunctionExpressionNumber>)
            {
                return arg->get_number();
            }
            else if constexpr (std::is_same_v<T, GroundFunctionExpressionBinaryOperator>)
            {
                return evaluate_binary(arg->get_binary_operator(),
                                       evaluate(arg->get_left_function_expression(), static_numeric_variables, fluent_numeric_variables),
                                       evaluate(arg->get_right_function_expression(), static_numeric_variables, fluent_numeric_variables));
            }
            else if constexpr (std::is_same_v<T, GroundFunctionExpressionMultiOperator>)
            {
                return std::accumulate(
                    std::next(arg->get_function_expressions().begin()),  // Start from the second expression
                    arg->get_function_expressions().end(),
                    evaluate(arg->get_function_expressions().front(), static_numeric_variables, fluent_numeric_variables),  // Initial bounds
                    [&](const auto& value, const auto& child_expr)
                    { return evaluate_multi(arg->get_multi_operator(), value, evaluate(child_expr, static_numeric_variables, fluent_numeric_variables)); });
            }
            else if constexpr (std::is_same_v<T, GroundFunctionExpressionMinus>)
            {
                const auto val = evaluate(arg->get_function_expression(), static_numeric_variables, fluent_numeric_variables);
                if (val == UNDEFINED_CONTINUOUS_COST)
                {
                    return UNDEFINED_CONTINUOUS_COST;
                }

                return -val;
            }
            else if constexpr (std::is_same_v<T, GroundFunctionExpressionFunction<StaticTag>>)
            {
                if (arg->get_function()->get_index() >= static_numeric_variables.size())
                {
                    return UNDEFINED_CONTINUOUS_COST;
                }

                return static_numeric_variables[arg->get_function()->get_index()];
            }
            else if constexpr (std::is_same_v<T, GroundFunctionExpressionFunction<FluentTag>>)
            {
                if (arg->get_function()->get_index() >= fluent_numeric_variables.size())
                {
                    return UNDEFINED_CONTINUOUS_COST;
                }

                return fluent_numeric_variables[arg->get_function()->get_index()];
            }
            else if constexpr (std::is_same_v<T, GroundFunctionExpressionFunction<AuxiliaryTag>>)
            {
                throw std::logic_error("evaluate(fexpr, fluent_numeric_variables): Unexpected GroundFunctionExpressionFunction<AuxiliaryTag>. Did you define a "
                                       "(composite) metric consisting of a single nullary function without defining its value in the initial state?");
            }
            else
            {
                static_assert(dependent_false<T>::value,
                              "evaluate(fexpr, fluent_numeric_variables): Missing implementation for GroundFunctionExpressionFunction type.");
            }
        },
        fexpr->get_variant());
}

/* Printing */
std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionNumberImpl& element)
{
    write(element, StringFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionBinaryOperatorImpl& element)
{
    write(element, StringFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionMultiOperatorImpl& element)
{
    write(element, StringFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionMinusImpl& element)
{
    write(element, StringFormatter(), out);
    return out;
}

template<IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionFunctionImpl<F>& element)
{
    write(element, StringFormatter(), out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionFunctionImpl<StaticTag>& element);
template std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionFunctionImpl<FluentTag>& element);
template std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionFunctionImpl<AuxiliaryTag>& element);

std::ostream& operator<<(std::ostream& out, GroundFunctionExpressionNumber element)
{
    write(*element, AddressFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, GroundFunctionExpressionBinaryOperator element)
{
    write(*element, AddressFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, GroundFunctionExpressionMultiOperator element)
{
    write(*element, AddressFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, GroundFunctionExpressionMinus element)
{
    write(*element, AddressFormatter(), out);
    return out;
}

template<IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, GroundFunctionExpressionFunction<F> element)
{
    write(*element, AddressFormatter(), out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, GroundFunctionExpressionFunction<StaticTag> element);
template std::ostream& operator<<(std::ostream& out, GroundFunctionExpressionFunction<FluentTag> element);
template std::ostream& operator<<(std::ostream& out, GroundFunctionExpressionFunction<AuxiliaryTag> element);

std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionImpl& element)
{
    write(element, StringFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, GroundFunctionExpression element)
{
    std::visit([&](const auto& arg) { out << arg; }, element->get_variant());
    return out;
}
}
