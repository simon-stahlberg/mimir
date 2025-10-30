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

#ifndef MIMIR_FORMALISM_ASSIGNMENT_HPP_
#define MIMIR_FORMALISM_ASSIGNMENT_HPP_

#include "mimir/common/declarations.hpp"
#include "mimir/formalism/declarations.hpp"

#include <limits>

namespace mimir::formalism
{

/**
 * EmptyAssignment
 */

struct EmptyAssignment
{
    static constexpr Index rank = 0;
};

/**
 * VertexAssignment
 */

struct VertexAssignment
{
    Index index;
    Index object;

    VertexAssignment() : index(MAX_INDEX), object(MAX_INDEX) {}

    VertexAssignment(Index index, Index object) : index(index), object(object) {}

    inline bool is_valid() const noexcept { return index != MAX_INDEX && object != MAX_INDEX; }
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

    inline bool is_valid() const noexcept
    {
        return (first_index < second_index) && (first_index != MAX_INDEX) && (second_index != MAX_INDEX) && (first_object != MAX_INDEX)
               && (second_object != MAX_INDEX);
    }
};

inline std::ostream& operator<<(std::ostream& os, const VertexAssignment& assignment)
{
    os << "[" << assignment.index << "/" << assignment.object << "]";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const EdgeAssignment& assignment)
{
    os << "[" << assignment.first_index << "/" << assignment.first_object << ", " << assignment.second_index << "/" << assignment.second_object << "]";
    return os;
}
}

#endif