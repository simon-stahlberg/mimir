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

#ifndef MIMIR_FORMALISM_GROUND_NUMERIC_CONSTRAINT_HPP_
#define MIMIR_FORMALISM_GROUND_NUMERIC_CONSTRAINT_HPP_

#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"

namespace mimir::formalism
{
class GroundNumericConstraintImpl
{
private:
    Index m_index;
    loki::BinaryComparatorEnum m_binary_comparator;
    GroundFunctionExpression m_left_function_expression;
    GroundFunctionExpression m_right_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    GroundNumericConstraintImpl(Index index,
                                loki::BinaryComparatorEnum binary_comparator,
                                GroundFunctionExpression left_function_expression,
                                GroundFunctionExpression right_function_expression);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    using FormalismEntity = void;

    // moveable but not copyable
    GroundNumericConstraintImpl(const GroundNumericConstraintImpl& other) = delete;
    GroundNumericConstraintImpl& operator=(const GroundNumericConstraintImpl& other) = delete;
    GroundNumericConstraintImpl(GroundNumericConstraintImpl&& other) = default;
    GroundNumericConstraintImpl& operator=(GroundNumericConstraintImpl&& other) = default;

    Index get_index() const;
    loki::BinaryComparatorEnum get_binary_comparator() const;
    GroundFunctionExpression get_left_function_expression() const;
    GroundFunctionExpression get_right_function_expression() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_binary_comparator(), get_left_function_expression(), get_right_function_expression()); }
};

/**
 * Utils
 */

extern bool evaluate(GroundNumericConstraint effect, const FlatDoubleList& static_numeric_variables, const FlatDoubleList& fluent_numeric_variables);

/**
 * Printing
 */

std::ostream& operator<<(std::ostream& os, const GroundNumericConstraintImpl& element);

std::ostream& operator<<(std::ostream& os, GroundNumericConstraint element);

}

#endif
