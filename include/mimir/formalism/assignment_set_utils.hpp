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
#include "mimir/formalism/declarations.hpp"

#include <limits>

namespace mimir::formalism
{

/**
 * We keep all the stuff in the header to allow inlining because this is used in tight loops!
 */

/**
 * VertexAssignment
 */

struct VertexAssignment
{
    Index index;
    Index object;

    VertexAssignment() : index(MAX_INDEX), object(MAX_INDEX) {}

    VertexAssignment(Index index, Index object) : index(index), object(object) {}

    VertexAssignment(const VertexAssignment& assignment, const IndexList& remapping)
    {
        assert(assignment.is_valid());

        index = remapping.at(assignment.index);
        object = assignment.object;

        if (index == MAX_INDEX)
        {
            object = MAX_INDEX;
        }
    }

    /// @brief Return true iff the index and object are set.
    bool is_valid() const { return index != MAX_INDEX && object != MAX_INDEX; }
};

/**
 * EdgeAssignment
 */

/// @brief Encapsulate assignment of objects to variables of atoms.
struct EdgeAssignment
{
    Index first_index;
    Index first_object;
    Index second_index;
    Index second_object;

    EdgeAssignment() : first_index(MAX_INDEX), first_object(MAX_INDEX), second_index(MAX_INDEX), second_object(MAX_INDEX) {}

    EdgeAssignment(Index first_index, Index first_object, Index second_index, Index second_object) :
        first_index(first_index),
        first_object(first_object),
        second_index(second_index),
        second_object(second_object)
    {
    }

    EdgeAssignment(const EdgeAssignment& assignment, const IndexList& remapping)
    {
        assert(assignment.is_valid());

        first_index = remapping.at(assignment.first_index);
        second_index = remapping.at(assignment.second_index);
        first_object = assignment.first_object;
        second_object = assignment.second_object;

        if (first_index == MAX_INDEX)
        {
            first_object = MAX_INDEX;
        }
        if (second_index == MAX_INDEX)
        {
            second_object = MAX_INDEX;
        }

        if (first_index > second_index)
        {
            std::swap(first_index, second_index);
            std::swap(first_object, second_object);
        }
    }

    /// @brief Return true iff both indices and objects are set.
    bool is_valid() const
    {
        return (first_index < second_index) && (first_index != MAX_INDEX) && (second_index != MAX_INDEX) && (first_object != MAX_INDEX)
               && (second_object != MAX_INDEX);
    }
};

/**
 * Joint ranking function between Vertex and Edge.
 */

inline size_t get_empty_assignment_rank() { return 0; }

inline size_t get_assignment_rank(const VertexAssignment& assignment, size_t arity, size_t num_objects)
{
    const auto first = 1;
    const auto second = first * (arity + 1);
    const auto rank = (first * (assignment.index + 1))  //
                      + (second * (assignment.object + 1));
    return rank;
}

inline size_t get_assignment_rank(const EdgeAssignment& assignment, size_t arity, size_t num_objects)
{
    const auto first = 1;
    const auto second = first * (arity + 1);
    const auto third = second * (num_objects + 1);
    const auto fourth = third * (arity + 1);
    const auto rank = (first * (assignment.first_index + 1))      //
                      + (second * (assignment.first_object + 1))  //
                      + (third * (assignment.second_index + 1))   //
                      + (fourth * (assignment.second_object + 1));
    return rank;
}

inline size_t num_assignments(size_t arity, size_t num_objects)
{
    const auto first = 1;
    const auto second = first * (arity + 1);
    const auto third = second * (num_objects + 1);
    const auto fourth = third * (arity + 1);
    const auto max = (first * arity)           //
                     + (second * num_objects)  //
                     + (third * arity)         //
                     + (fourth * num_objects);
    return max + 1;
}

}

#endif
