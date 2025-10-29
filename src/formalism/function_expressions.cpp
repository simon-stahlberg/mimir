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

#include "mimir/formalism/function_expressions.hpp"

#include "formatter.hpp"
#include "mimir/common/collections.hpp"
#include "mimir/formalism/function.hpp"

#include <cassert>

namespace mimir::formalism
{
/* FunctionExpressionNumber */
FunctionExpressionNumberImpl::FunctionExpressionNumberImpl(Index index, double number) : m_index(index), m_number(number) {}

Index FunctionExpressionNumberImpl::get_index() const { return m_index; }

double FunctionExpressionNumberImpl::get_number() const { return m_number; }

/* FunctionExpressionBinaryOperator */
FunctionExpressionBinaryOperatorImpl::FunctionExpressionBinaryOperatorImpl(Index index,
                                                                           loki::BinaryOperatorEnum binary_operator,
                                                                           FunctionExpression left_function_expression,
                                                                           FunctionExpression right_function_expression) :
    m_index(index),
    m_binary_operator(binary_operator),
    m_left_function_expression(std::move(left_function_expression)),
    m_right_function_expression(std::move(right_function_expression))
{
    if (binary_operator == loki::BinaryOperatorEnum::MUL || binary_operator == loki::BinaryOperatorEnum::PLUS)
    {
        assert(m_left_function_expression->get_index() < m_right_function_expression->get_index());
    }
}

Index FunctionExpressionBinaryOperatorImpl::get_index() const { return m_index; }

loki::BinaryOperatorEnum FunctionExpressionBinaryOperatorImpl::get_binary_operator() const { return m_binary_operator; }

FunctionExpression FunctionExpressionBinaryOperatorImpl::get_left_function_expression() const { return m_left_function_expression; }

FunctionExpression FunctionExpressionBinaryOperatorImpl::get_right_function_expression() const { return m_right_function_expression; }

/* FunctionExpressionMultiOperator */
FunctionExpressionMultiOperatorImpl::FunctionExpressionMultiOperatorImpl(Index index,
                                                                         loki::MultiOperatorEnum multi_operator,
                                                                         FunctionExpressionList function_expressions) :
    m_index(index),
    m_multi_operator(multi_operator),
    m_function_expressions(function_expressions)
{
    assert(is_all_unique(m_function_expressions));
    assert(std::is_sorted(m_function_expressions.begin(),
                          m_function_expressions.end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
}

Index FunctionExpressionMultiOperatorImpl::get_index() const { return m_index; }

loki::MultiOperatorEnum FunctionExpressionMultiOperatorImpl::get_multi_operator() const { return m_multi_operator; }

const FunctionExpressionList& FunctionExpressionMultiOperatorImpl::get_function_expressions() const { return m_function_expressions; }

/* FunctionExpressionMinus */
FunctionExpressionMinusImpl::FunctionExpressionMinusImpl(Index index, FunctionExpression function_expression) :
    m_index(index),
    m_function_expression(std::move(function_expression))
{
}

Index FunctionExpressionMinusImpl::get_index() const { return m_index; }

FunctionExpression FunctionExpressionMinusImpl::get_function_expression() const { return m_function_expression; }

/* FunctionExpressionFunction */
template<IsStaticOrFluentOrAuxiliaryTag F>
FunctionExpressionFunctionImpl<F>::FunctionExpressionFunctionImpl(Index index, Function<F> function) : m_index(index), m_function(std::move(function))
{
}

template<IsStaticOrFluentOrAuxiliaryTag F>
Index FunctionExpressionFunctionImpl<F>::get_index() const
{
    return m_index;
}

template<IsStaticOrFluentOrAuxiliaryTag F>
Function<F> FunctionExpressionFunctionImpl<F>::get_function() const
{
    return m_function;
}

template class FunctionExpressionFunctionImpl<StaticTag>;
template class FunctionExpressionFunctionImpl<FluentTag>;
template class FunctionExpressionFunctionImpl<AuxiliaryTag>;

/* FunctionExpression */
FunctionExpressionImpl::FunctionExpressionImpl(Index index, FunctionExpressionVariant function_expression) :
    m_index(index),
    m_function_expression(function_expression)
{
}

Index FunctionExpressionImpl::get_index() const { return m_index; }

const FunctionExpressionVariant& FunctionExpressionImpl::get_variant() const { return m_function_expression; }

/**
 * Printing
 */

std::ostream& operator<<(std::ostream& out, const FunctionExpressionNumberImpl& element) { return mimir::formatter::operator<<(out, element); }

std::ostream& operator<<(std::ostream& out, const FunctionExpressionBinaryOperatorImpl& element) { return mimir::formatter::operator<<(out, element); }

std::ostream& operator<<(std::ostream& out, const FunctionExpressionMultiOperatorImpl& element) { return mimir::formatter::operator<<(out, element); }

std::ostream& operator<<(std::ostream& out, const FunctionExpressionMinusImpl& element) { return mimir::formatter::operator<<(out, element); }

template<IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, const FunctionExpressionFunctionImpl<F>& element)
{
    return mimir::formatter::operator<<(out, element);
}

template std::ostream& operator<<(std::ostream& out, const FunctionExpressionFunctionImpl<StaticTag>& element);
template std::ostream& operator<<(std::ostream& out, const FunctionExpressionFunctionImpl<FluentTag>& element);
template std::ostream& operator<<(std::ostream& out, const FunctionExpressionFunctionImpl<AuxiliaryTag>& element);

std::ostream& operator<<(std::ostream& out, const FunctionExpressionImpl& element) { return mimir::formatter::operator<<(out, element); }

}

namespace mimir::formatter
{
std::ostream& operator<<(std::ostream& out, const mimir::formalism::FunctionExpressionNumberImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, const mimir::formalism::FunctionExpressionBinaryOperatorImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, const mimir::formalism::FunctionExpressionMultiOperatorImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, const mimir::formalism::FunctionExpressionMinusImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

template<mimir::formalism::IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, const mimir::formalism::FunctionExpressionFunctionImpl<F>& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, const mimir::formalism::FunctionExpressionImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}
}