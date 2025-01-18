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

#include "mimir/formalism/numeric_constraint.hpp"

#include "formatter.hpp"

namespace mimir
{
NumericConstraintImpl::NumericConstraintImpl(Index index,
                                             loki::BinaryComparatorEnum binary_comparator,
                                             FunctionExpression function_expression_left,
                                             FunctionExpression function_expression_right) :
    m_index(index),
    m_binary_comparator(binary_comparator),
    m_function_expression_left(function_expression_left),
    m_function_expression_right(function_expression_right)
{
}

Index NumericConstraintImpl::get_index() const { return m_index; }

loki::BinaryComparatorEnum NumericConstraintImpl::get_binary_comparator() const { return m_binary_comparator; }

const FunctionExpression& NumericConstraintImpl::get_function_expression_left() const { return m_function_expression_left; }

const FunctionExpression& NumericConstraintImpl::get_function_expression_right() const { return m_function_expression_right; }

std::ostream& operator<<(std::ostream& out, const NumericConstraintImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, NumericConstraint element)
{
    out << *element;
    return out;
}
}
