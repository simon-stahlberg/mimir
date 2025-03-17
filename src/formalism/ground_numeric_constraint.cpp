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

#include "mimir/formalism/ground_numeric_constraint.hpp"

#include "formatter.hpp"
#include "mimir/formalism/function.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/ground_function.hpp"
#include "mimir/formalism/ground_function_expressions.hpp"

namespace mimir::formalism
{
GroundNumericConstraintImpl::GroundNumericConstraintImpl(Index index,
                                                         loki::BinaryComparatorEnum binary_comparator,
                                                         GroundFunctionExpression left_function_expression,
                                                         GroundFunctionExpression right_function_expression) :
    m_index(index),
    m_binary_comparator(binary_comparator),
    m_left_function_expression(left_function_expression),
    m_right_function_expression(right_function_expression)
{
}

Index GroundNumericConstraintImpl::get_index() const { return m_index; }

loki::BinaryComparatorEnum GroundNumericConstraintImpl::get_binary_comparator() const { return m_binary_comparator; }

GroundFunctionExpression GroundNumericConstraintImpl::get_left_function_expression() const { return m_left_function_expression; }

GroundFunctionExpression GroundNumericConstraintImpl::get_right_function_expression() const { return m_right_function_expression; }

/**
 * Utils
 */

bool evaluate(GroundNumericConstraint constraint, const FlatDoubleList& static_numeric_variables, const FlatDoubleList& fluent_numeric_variables)
{
    const auto left_value = evaluate(constraint->get_left_function_expression(), static_numeric_variables, fluent_numeric_variables);
    const auto right_value = evaluate(constraint->get_right_function_expression(), static_numeric_variables, fluent_numeric_variables);

    /* Constraint is not satisfied for NaN values. */
    if (left_value == UNDEFINED_CONTINUOUS_COST || right_value == UNDEFINED_CONTINUOUS_COST)
        return false;

    switch (constraint->get_binary_comparator())
    {
        case loki::BinaryComparatorEnum::EQUAL:
        {
            return left_value == right_value;
        }
        case loki::BinaryComparatorEnum::GREATER:
        {
            return left_value > right_value;
        }
        case loki::BinaryComparatorEnum::GREATER_EQUAL:
        {
            return left_value >= right_value;
        }
        case loki::BinaryComparatorEnum::LESS:
        {
            return left_value < right_value;
        }
        case loki::BinaryComparatorEnum::LESS_EQUAL:
        {
            return left_value <= right_value;
        }
        default:
        {
            throw std::logic_error("evaluate(effect, fluent_numeric_variables, auxiliary_numeric_variables): Unexpected loki::BinaryComparatorEnum.");
        }
    }
}

/**
 * Printing
 */

std::ostream& operator<<(std::ostream& out, const GroundNumericConstraintImpl& element)
{
    write(element, StringFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, GroundNumericConstraint element)
{
    write(*element, AddressFormatter(), out);
    return out;
}
}
