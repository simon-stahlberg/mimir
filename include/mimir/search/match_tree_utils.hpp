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

#ifndef MIMIR_SEARCH_MATCH_TREE_HPP_
#define MIMIR_SEARCH_MATCH_TREE_HPP_

#include "mimir/formalism/ground_conjunctive_condition.hpp"
#include "mimir/formalism/ground_effects.hpp"
#include "mimir/formalism/ground_numeric_constraint.hpp"

namespace mimir
{

int less_compare(const FlatIndexList& pos_lhs, const FlatIndexList& neg_lhs, const FlatIndexList& pos_rhs, const FlatIndexList& neg_rhs)
{
    auto pos_it_lhs = pos_lhs.begin();
    auto neg_it_lhs = neg_lhs.begin();
    auto pos_it_rhs = pos_rhs.begin();
    auto neg_it_rhs = neg_rhs.begin();

    auto pos_end_lhs = pos_lhs.end();
    auto neg_end_lhs = neg_lhs.end();
    auto pos_end_rhs = pos_rhs.end();
    auto neg_end_rhs = neg_rhs.end();

    // Merge-style comparison for four lists
    while (pos_it_lhs != pos_end_lhs || neg_it_lhs != neg_end_lhs || pos_it_rhs != pos_end_rhs || neg_it_rhs != neg_end_rhs)
    {
        // Fetch current values (or max value if iterator is exhausted)
        Index pos_val_lhs = (pos_it_lhs != pos_end_lhs) ? *pos_it_lhs : std::numeric_limits<Index>::max();
        Index neg_val_lhs = (neg_it_lhs != neg_end_lhs) ? *neg_it_lhs : std::numeric_limits<Index>::max();
        Index pos_val_rhs = (pos_it_rhs != pos_end_rhs) ? *pos_it_rhs : std::numeric_limits<Index>::max();
        Index neg_val_rhs = (neg_it_rhs != neg_end_rhs) ? *neg_it_rhs : std::numeric_limits<Index>::max();

        // Find the smallest index among all iterators
        Index smallest = std::min({ pos_val_lhs, neg_val_lhs, pos_val_rhs, neg_val_rhs });

        // Compare smallest index from LHS and RHS
        bool lhs_has = (pos_val_lhs == smallest || neg_val_lhs == smallest);
        bool rhs_has = (pos_val_rhs == smallest || neg_val_rhs == smallest);

        if (lhs_has && !rhs_has)
            return -1;  // LHS has an element that RHS doesn't
        if (rhs_has && !lhs_has)
            return 1;  // RHS has an element that LHS doesn't

        // Move iterators that matched the smallest value
        if (pos_val_lhs == smallest)
            ++pos_it_lhs;
        if (neg_val_lhs == smallest)
            ++neg_it_lhs;
        if (pos_val_rhs == smallest)
            ++pos_it_rhs;
        if (neg_val_rhs == smallest)
            ++neg_it_rhs;
    }

    return 0;  // Lists are identical
}

int less_compare(const FlatExternalPtrList<const GroundNumericConstraintImpl>& lhs, const FlatExternalPtrList<const GroundNumericConstraintImpl>& rhs)
{
    auto it_lhs = lhs.begin();
    auto it_rhs = rhs.begin();

    auto end_lhs = lhs.end();
    auto end_rhs = rhs.end();

    // Merge-style comparison for two lists
    while (it_lhs != end_lhs || it_rhs != end_rhs)
    {
        // Fetch current values (or max value if iterator is exhausted)
        Index val_lhs = (it_lhs != end_lhs) ? (**it_lhs).get_index() : std::numeric_limits<Index>::max();
        Index val_rhs = (it_rhs != end_rhs) ? (**it_rhs).get_index() : std::numeric_limits<Index>::max();

        // Find the smallest index among all iterators
        Index smallest = std::min(val_lhs, val_rhs);

        // Compare smallest index from LHS and RHS
        bool lhs_has = (val_lhs == smallest);
        bool rhs_has = (val_rhs == smallest);

        if (lhs_has && !rhs_has)
            return -1;  // LHS has an element that RHS doesn't
        if (rhs_has && !lhs_has)
            return 1;  // RHS has an element that LHS doesn't

        // Move iterators that matched the smallest value
        if (val_lhs == smallest)
            ++it_lhs;
        if (val_rhs == smallest)
            ++it_rhs;
    }

    return 0;  // Lists are identical
}

bool less_compare(const GroundConjunctiveCondition& lhs, const GroundConjunctiveCondition& rhs)
{
    /* 1. Fluent atoms */
    auto first = less_compare(lhs.get_positive_precondition<Fluent>(),
                              lhs.get_negative_precondition<Fluent>(),
                              rhs.get_positive_precondition<Fluent>(),
                              rhs.get_negative_precondition<Fluent>());

    if (first != 0)
        return first == -1;

    /* 2. Numeric constraint */
    auto second = less_compare(lhs.get_numeric_constraints(), rhs.get_numeric_constraints());

    if (second != 0)
        return second == -1;

    /* 3. Derived atoms */
    auto third = less_compare(lhs.get_positive_precondition<Derived>(),
                              lhs.get_negative_precondition<Derived>(),
                              rhs.get_positive_precondition<Derived>(),
                              rhs.get_negative_precondition<Derived>());

    return third == -1;
}

/// @brief Sorts the elements by their condition.
///
/// @tparam T
/// @param elements
template<typename T>
void sort_elements_by_lexicographically_by_conjunctive_condition(std::vector<T>& elements)
{
    std::sort(elements.begin(),
              elements.end(),
              [](auto&& lhs, auto&& rhs) { return less_compare(lhs->get_conjunctive_condition(), rhs->get_conjunctive_condition()); })
}

}

#endif
