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

namespace mimir::formalism
{

NumericConstraintImpl::NumericConstraintImpl(Index index,
                                             loki::BinaryComparatorEnum binary_comparator,
                                             FunctionExpression left_function_expression,
                                             FunctionExpression right_function_expression,
                                             TermList terms) :
    m_index(index),
    m_binary_comparator(binary_comparator),
    m_left_function_expression(left_function_expression),
    m_right_function_expression(right_function_expression),
    m_terms(std::move(terms))
{
}

Index NumericConstraintImpl::get_index() const { return m_index; }

loki::BinaryComparatorEnum NumericConstraintImpl::get_binary_comparator() const { return m_binary_comparator; }

FunctionExpression NumericConstraintImpl::get_left_function_expression() const { return m_left_function_expression; }

FunctionExpression NumericConstraintImpl::get_right_function_expression() const { return m_right_function_expression; }

const TermList& NumericConstraintImpl::get_terms() const { return m_terms; }

std::ostream& operator<<(std::ostream& out, const NumericConstraintImpl& element)
{
    write(element, StringFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, NumericConstraint element)
{
    write(*element, AddressFormatter(), out);
    return out;
}
}
