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
    constexpr Bounds() : Bounds(unbounded) {}

    constexpr Bounds(A lower, A upper) : m_lower(lower), m_upper(upper) {}

    friend constexpr bool operator==(const Bounds& lhs, const Bounds& rhs) noexcept { return lhs.m_lower == rhs.m_lower && lhs.m_upper == rhs.m_upper; }
    friend constexpr bool operator!=(const Bounds& lhs, const Bounds& rhs) noexcept { return !(lhs == rhs); }

    /// @brief ∃ x ∈ lhs, ∃ y ∈ rhs : x = y.
    friend constexpr bool overlaps(const Bounds& lhs, const Bounds& rhs) noexcept { return lhs.m_lower <= rhs.m_upper && lhs.m_upper >= rhs.m_lower; }
    /// @brief ∀ x ∈ lhs, ∀ y ∈ rhs : x != y.
    friend constexpr bool disjoint(const Bounds& lhs, const Bounds& rhs) noexcept { return !overlaps(lhs, rhs); }
    /// @brief ∃ x ∈ lhs, ∃ y ∈ rhs : x > y.
    friend constexpr bool possibly_after(const Bounds& lhs, const Bounds& rhs) noexcept { return lhs.m_upper > rhs.m_lower; }
    /// @brief ∃ x ∈ lhs, ∃ y ∈ rhs : x >= y.
    friend constexpr bool possibly_after_or_meets(const Bounds& lhs, const Bounds& rhs) noexcept { return lhs.m_upper >= rhs.m_lower; }
    /// @brief ∃ x ∈ lhs, ∃ y ∈ rhs : x < y.
    friend constexpr bool possibly_before(const Bounds& lhs, const Bounds& rhs) noexcept { return lhs.m_lower < rhs.m_upper; }
    /// @brief ∃ x ∈ lhs, ∃ y ∈ rhs : x <= y.
    friend constexpr bool possibly_before_or_meets(const Bounds& lhs, const Bounds& rhs) noexcept { return lhs.m_lower <= rhs.m_upper; }

    static constexpr Bounds unbounded_bounds() noexcept
    {
        if constexpr (std::is_floating_point_v<A>)
        {
            return Bounds(-std::numeric_limits<A>::infinity(), std::numeric_limits<A>::infinity());
        }
        else
        {
            return Bounds(std::numeric_limits<A>::lowest(), std::numeric_limits<A>::max());
        }
    }

    inline static constexpr Bounds unbounded = unbounded_bounds();

    constexpr bool is_bounded() const noexcept { return m_lower <= m_upper; }
    constexpr bool is_unbounded() const noexcept
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

    constexpr A get_lower() const noexcept { return m_lower; }
    constexpr A get_upper() const noexcept { return m_upper; }

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
