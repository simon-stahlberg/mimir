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

#ifndef MIMIR_COMMON_BOUNDS_HPP_
#define MIMIR_COMMON_BOUNDS_HPP_

#include "mimir/common/types.hpp"

#include <ostream>

namespace mimir
{

template<IsArithmetic A>
class Bounds
{
public:
    Bounds() : Bounds(unbounded) {}
    
    Bounds(A lower, A upper) : m_lower(lower), m_upper(upper) {}

    /// @brief Return true iff there exist x in lhs and exists y in rhs : x = y.
    friend bool operator==(const Bounds& lhs, const Bounds& rhs) { return lhs.m_lower <= rhs.m_upper && lhs.m_upper >= rhs.m_lower; }
    /// @brief Return true iff there forall x in lhs and forall y in rhs : x != y.
    friend bool operator!=(const Bounds& lhs, const Bounds& rhs) { return !(lhs == rhs); }
    /// @brief Return true iff there exist x in lhs and exists y in rhs : x > y.
    friend bool operator>(const Bounds& lhs, const Bounds& rhs) { return lhs.m_upper > rhs.m_lower; }
    /// @brief Return true iff there exist x in lhs and exists y in rhs : x >= y.
    friend bool operator>=(const Bounds& lhs, const Bounds& rhs) { return lhs.m_upper >= rhs.m_lower; }
    /// @brief Return true iff there exist x in lhs and exists y in rhs : x < y.
    friend bool operator<(const Bounds& lhs, const Bounds& rhs) { return lhs.m_lower < rhs.m_upper; }
    /// @brief Return true iff there exist x in lhs and exists y in rhs : x <= y.
    friend bool operator<=(const Bounds& lhs, const Bounds& rhs) { return lhs.m_lower <= rhs.m_upper; }

    // Static unbounded instance
    inline static const Bounds unbounded = []
    {
        if constexpr (std::is_floating_point_v<A>)
        {
            return Bounds(-std::numeric_limits<A>::infinity(), std::numeric_limits<A>::infinity());
        }
        else
        {
            return Bounds(std::numeric_limits<A>::lowest(), std::numeric_limits<A>::max());
        }
    }();

    bool is_bounded() const { return m_lower <= m_upper; }
    bool is_unbounded() const
    {
        if constexpr (std::is_floating_point_v<A>)
        {
            return m_lower == -std::numeric_limits<A>::infinity() && m_upper == std::numeric_limits<A>::infinity();
        }
        else
        {
            return m_lower == std::numeric_limits<A>::lowest() && m_upper == std::numeric_limits<A>::max();
        }
    }

    A get_lower() const {return m_lower; }
    A get_upper() const {return m_upper; }

private:
    A m_lower;
    A m_upper;
};

template<IsArithmetic A>
inline std::ostream& operator<<(std::ostream& out, const Bounds<A>& element)
{
    out << "[" << element.get_lower() << "," << element.get_upper() << "]";
    return out;
}

}

#endif
