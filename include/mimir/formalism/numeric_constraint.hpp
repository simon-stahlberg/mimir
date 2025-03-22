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

#include "mimir/common/bounds.hpp"
#include "mimir/formalism/declarations.hpp"

#include <absl/container/flat_hash_map.h>

namespace mimir::formalism
{

class NumericConstraintImpl
{
private:
    Index m_index;
    loki::BinaryComparatorEnum m_binary_comparator;
    FunctionExpression m_left_function_expression;
    FunctionExpression m_right_function_expression;
    TermList m_terms;

    // Below: add additional members if needed and initialize them in the constructor

    NumericConstraintImpl(Index index,
                          loki::BinaryComparatorEnum binary_comparator,
                          FunctionExpression left_function_expression,
                          FunctionExpression right_function_expression,
                          TermList terms);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    using FormalismEntity = void;

    // moveable but not copyable
    NumericConstraintImpl(const NumericConstraintImpl& other) = delete;
    NumericConstraintImpl& operator=(const NumericConstraintImpl& other) = delete;
    NumericConstraintImpl(NumericConstraintImpl&& other) = default;
    NumericConstraintImpl& operator=(NumericConstraintImpl&& other) = default;

    Index get_index() const;
    loki::BinaryComparatorEnum get_binary_comparator() const;
    FunctionExpression get_left_function_expression() const;
    FunctionExpression get_right_function_expression() const;
    const TermList& get_terms() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const
    {
        return std::tuple(get_binary_comparator(), get_left_function_expression(), get_right_function_expression(), std::cref(get_terms()));
    }
};

/**
 * Utils
 */

template<IsArithmetic A>
inline bool evaluate(loki::BinaryComparatorEnum comparator, const Bounds<A>& lhs, const Bounds<A>& rhs)
{
    switch (comparator)
    {
        case loki::BinaryComparatorEnum::EQUAL:
        {
            return lhs == rhs;
        }
        case loki::BinaryComparatorEnum::GREATER:
        {
            return lhs > rhs;
        }
        case loki::BinaryComparatorEnum::GREATER_EQUAL:
        {
            return lhs >= rhs;
        }
        case loki::BinaryComparatorEnum::LESS:
        {
            return lhs < rhs;
        }
        case loki::BinaryComparatorEnum::LESS_EQUAL:
        {
            return lhs <= rhs;
        }
        default:
        {
            throw std::logic_error("evaluate(comparator, lhs, rhs): Unexpected BinaryComparatorEnum.");
        }
    }
}

/**
 * Printing
 */

extern std::ostream& operator<<(std::ostream& out, const NumericConstraintImpl& element);

extern std::ostream& operator<<(std::ostream& out, NumericConstraint element);
}

#endif
