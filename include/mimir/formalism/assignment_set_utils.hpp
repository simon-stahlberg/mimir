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

#ifndef MIMIR_FORMALISM_ASSIGNMENT_SET_UTILS_HPP_
#define MIMIR_FORMALISM_ASSIGNMENT_SET_UTILS_HPP_

#include "mimir/common/types.hpp"

#include <limits>

namespace mimir
{

/**
 * We keep all the stuff in the header to allow inlining because this is used in tight loops!
 */

/// @brief Encapsulate assignment of objects to variables of atoms.
struct Assignment
{
    // We use this as special value and when adding 1 we obtain 0.
    static const Index MAX_VALUE = std::numeric_limits<Index>::max();

    Index first_index;
    Index first_object;
    Index second_index;
    Index second_object;

    Assignment(Index index, Index object);

    Assignment(Index first_index, Index first_object, Index second_index, Index second_object);

    size_t size() const { return (first_object != MAX_VALUE ? 1 : 0) + (second_object != MAX_VALUE ? 1 : 0); }
};

inline size_t get_assignment_rank(const Assignment& assignment, size_t arity, size_t num_objects)
{
    const auto first = 1;
    const auto second = first * (arity + 1);
    const auto third = second * (arity + 1);
    const auto fourth = third * (num_objects + 1);
    const auto rank = (first * (assignment.first_index + 1))      //
                      + (second * (assignment.second_index + 1))  //
                      + (third * (assignment.first_object + 1))   //
                      + (fourth * (assignment.second_object + 1));
    return rank;
}

inline size_t num_assignments(size_t arity, size_t num_objects)
{
    const auto first = 1;
    const auto second = first * (arity + 1);
    const auto third = second * (arity + 1);
    const auto fourth = third * (num_objects + 1);
    const auto max = (first * arity)          //
                     + (second * arity)       //
                     + (third * num_objects)  //
                     + (fourth * num_objects);
    return max + 1;
}

}

#endif
