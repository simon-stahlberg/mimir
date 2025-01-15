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

#ifndef MIMIR_FORMALISM_NUMERIC_CONSTRAINT_HPP_
#define MIMIR_FORMALISM_NUMERIC_CONSTRAINT_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir
{

class NumericConstraintImpl
{
private:
    loki::BinaryComparatorEnum m_comparator;
    FunctionExpression m_function_expression_left;
    FunctionExpression m_function_expression_right;

    // Below: add additional members if needed and initialize them in the constructor

    NumericConstraintImpl(Index index,
                          loki::BinaryComparatorEnum comparator,
                          FunctionExpression function_expression_left,
                          FunctionExpression function_expression_right);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    NumericConstraintImpl(const NumericConstraintImpl& other) = delete;
    NumericConstraintImpl& operator=(const NumericConstraintImpl& other) = delete;
    NumericConstraintImpl(NumericConstraintImpl&& other) = default;
    NumericConstraintImpl& operator=(NumericConstraintImpl&& other) = default;

    Index get_index() const;
    loki::BinaryComparatorEnum get_comparator() const;
    const FunctionExpression& get_function_expression_left() const;
    const FunctionExpression& get_function_expression_right() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifiable_members() const
    {
        return std::forward_as_tuple(std::as_const(m_comparator), std::as_const(m_function_expression_left), std::as_const(m_function_expression_right));
    }
};

extern std::ostream& operator<<(std::ostream& out, const NumericConstraintImpl& element);

extern std::ostream& operator<<(std::ostream& out, NumericConstraint element);
}

#endif
