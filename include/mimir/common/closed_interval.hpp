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

#ifndef MIMIR_COMMON_CLOSED_INTERVAL_HPP_
#define MIMIR_COMMON_CLOSED_INTERVAL_HPP_

#include "mimir/common/types.hpp"

#include <cmath>
#include <limits>
#include <loki/loki.hpp>
#include <ostream>
#include <type_traits>

namespace mimir
{

template<IsFloatingPoint A>
class ClosedInterval
{
public:
    constexpr ClosedInterval(A lower, A upper) : m_lower(lower), m_upper(upper) { assert(is_wellformed()); }

    friend constexpr bool operator==(const ClosedInterval& lhs, const ClosedInterval& rhs) noexcept
    {
        if (lhs.is_undefined() && rhs.is_undefined())
            return true;
        return lhs.m_lower == rhs.m_lower && lhs.m_upper == rhs.m_upper;
    }
    friend constexpr bool operator!=(const ClosedInterval& lhs, const ClosedInterval& rhs) noexcept { return !(lhs == rhs); }

    /// @brief ∃ x ∈ lhs, ∃ y ∈ rhs : x = y.
    friend constexpr bool overlaps(const ClosedInterval& lhs, const ClosedInterval& rhs) noexcept
    {
        if (lhs.is_undefined() || rhs.is_undefined())
            return false;
        return lhs.m_lower <= rhs.m_upper && lhs.m_upper >= rhs.m_lower;
    }
    /// @brief ∀ x ∈ lhs, ∀ y ∈ rhs : x != y.
    friend constexpr bool disjoint(const ClosedInterval& lhs, const ClosedInterval& rhs) noexcept
    {
        if (lhs.is_undefined() || rhs.is_undefined())
            return false;
        return !overlaps(lhs, rhs);
    }
    /// @brief ∃ x ∈ lhs, ∃ y ∈ rhs : x > y.
    friend constexpr bool possibly_after(const ClosedInterval& lhs, const ClosedInterval& rhs) noexcept
    {
        if (lhs.is_undefined() || rhs.is_undefined())
            return false;
        return lhs.m_upper > rhs.m_lower;
    }
    /// @brief ∃ x ∈ lhs, ∃ y ∈ rhs : x >= y.
    friend constexpr bool possibly_after_or_meets(const ClosedInterval& lhs, const ClosedInterval& rhs) noexcept
    {
        if (lhs.is_undefined() || rhs.is_undefined())
            return false;
        return lhs.m_upper >= rhs.m_lower;
    }
    /// @brief ∃ x ∈ lhs, ∃ y ∈ rhs : x < y.
    friend constexpr bool possibly_before(const ClosedInterval& lhs, const ClosedInterval& rhs) noexcept
    {
        if (lhs.is_undefined() || rhs.is_undefined())
            return false;
        return lhs.m_lower < rhs.m_upper;
    }
    /// @brief ∃ x ∈ lhs, ∃ y ∈ rhs : x <= y.
    friend constexpr bool possibly_before_or_meets(const ClosedInterval& lhs, const ClosedInterval& rhs) noexcept
    {
        if (lhs.is_undefined() || rhs.is_undefined())
            return false;
        return lhs.m_lower <= rhs.m_upper;
    }

    static constexpr ClosedInterval zero() noexcept { return ClosedInterval(A(0), A(0)); }
    static constexpr ClosedInterval undefined() noexcept { return ClosedInterval(-std::numeric_limits<A>::quiet_NaN(), std::numeric_limits<A>::quiet_NaN()); }
    static constexpr ClosedInterval reals() noexcept { return ClosedInterval(-std::numeric_limits<A>::infinity(), std::numeric_limits<A>::infinity()); }

    inline bool is_wellformed() const noexcept { return (std::isnan(m_lower) && std::isnan(m_upper)) || (m_lower <= m_upper); }
    inline bool is_undefined() const noexcept { return std::isnan(m_lower) && std::isnan(m_upper); }
    inline bool contains_zero() const noexcept { return m_lower <= A(0) && m_upper >= A(0); }
    inline bool strictly_pos() const noexcept { return m_lower > A(0); }
    inline bool strictly_neg() const noexcept { return m_upper < A(0); }
    inline bool nonneg_only() const noexcept { return m_lower >= A(0); }
    inline bool nonpos_only() const noexcept { return m_upper <= A(0); }
    inline bool is_point_zero() const noexcept { return m_lower == A(0) && m_upper == A(0); }
    inline bool crosses_zero() const noexcept { return m_lower < A(0) && m_upper > A(0); }
    inline bool unbounded_pos() const noexcept { return std::isinf(m_upper) && m_lower >= A(0); }
    inline bool unbounded_neg() const noexcept { return std::isinf(m_lower) && m_upper <= A(0); }
    inline bool unbounded_both() const noexcept { return std::isinf(m_lower) && std::isinf(m_upper); }
    inline bool has_pos_inf() const noexcept { return std::isinf(m_upper) && m_upper > A(0); }
    inline bool has_neg_inf() const noexcept { return std::isinf(m_lower) && m_lower < A(0); }

    constexpr A get_lower() const noexcept { return m_lower; }
    constexpr A get_upper() const noexcept { return m_upper; }

    // --- Utilities ---
    friend constexpr ClosedInterval intersect(const ClosedInterval& a, const ClosedInterval& b) noexcept
    {
        if (a.is_undefined() || b.is_undefined())
            return ClosedInterval::undefined();
        const A lo = std::max(a.get_lower(), b.get_lower());
        const A hi = std::min(a.get_upper(), b.get_upper());
        return (lo <= hi) ? ClosedInterval(lo, hi) : ClosedInterval::undefined();
    }

    friend constexpr ClosedInterval hull(const ClosedInterval& a, const ClosedInterval& b) noexcept
    {
        if (a.is_undefined())
            return b;
        if (b.is_undefined())
            return a;
        const A lo = std::min(a.get_lower(), b.get_lower());
        const A hi = std::max(a.get_upper(), b.get_upper());
        return ClosedInterval(lo, hi);
    }

private:
    A m_lower;
    A m_upper;
};

/**
 * ClosedInterval arithmetics
 */

template<IsFloatingPoint A>
inline ClosedInterval<A> evaluate(loki::BinaryOperatorEnum op, const ClosedInterval<A>& lhs, const ClosedInterval<A>& rhs)
{
    if (lhs.is_undefined() || rhs.is_undefined())
        return ClosedInterval<A>::undefined();  // undefined sentinel

    switch (op)
    {
        case loki::BinaryOperatorEnum::DIV:
        {
            // 0 not in rhs -> standard 4-corner envelope
            if (!rhs.contains_zero())
            {
                const A a = lhs.get_lower() / rhs.get_lower();
                const A b = lhs.get_lower() / rhs.get_upper();
                const A c = lhs.get_upper() / rhs.get_lower();
                const A d = lhs.get_upper() / rhs.get_upper();
                const auto mm = std::minmax({ a, b, c, d });
                return ClosedInterval<A>(mm.first, mm.second);
            }

            // rhs == {0}
            else if (rhs.is_point_zero())
                return ClosedInterval<A>::undefined();

            // rhs touches 0 on + side: (0, b] with b>0
            else if (rhs.get_lower() == A(0) && rhs.get_upper() > A(0))
            {
                if (lhs.is_point_zero())
                    return ClosedInterval<A>::zero();
                if (lhs.strictly_pos())
                    return ClosedInterval<A>(lhs.get_lower() / rhs.get_upper(), std::numeric_limits<A>::infinity());
                if (lhs.strictly_neg())
                    return ClosedInterval<A>(-std::numeric_limits<A>::infinity(), lhs.get_upper() / rhs.get_upper());
                return ClosedInterval<A>::reals();
            }

            // rhs touches 0 on - side: [a, 0) with a<0
            else if (rhs.get_lower() < A(0) && rhs.get_upper() == A(0))
            {
                if (lhs.is_point_zero())
                    return ClosedInterval<A>::zero();
                if (lhs.strictly_pos())
                    return ClosedInterval<A>(-std::numeric_limits<A>::infinity(), lhs.get_lower() / rhs.get_lower());
                if (lhs.strictly_neg())
                    return ClosedInterval<A>(lhs.get_upper() / rhs.get_lower(), std::numeric_limits<A>::infinity());
                return ClosedInterval<A>::reals();
            }

            // rhs spans negative & positive (a<0<b): unless lhs=={0}, whole line
            else if (lhs.is_point_zero())
                return ClosedInterval<A>::zero();

            return ClosedInterval<A>::reals();
        }

        case loki::BinaryOperatorEnum::MINUS:
        {
            const A lo = lhs.get_lower() - rhs.get_upper();  // l - r'
            const A hi = lhs.get_upper() - rhs.get_lower();  // r - l'
            return { std::isnan(lo) ? -std::numeric_limits<A>::infinity() : lo, std::isnan(hi) ? std::numeric_limits<A>::infinity() : hi };
        }

        case loki::BinaryOperatorEnum::MUL:
        {
            if (lhs.is_point_zero() || rhs.is_point_zero())
                return ClosedInterval<A>(A(0), A(0));

            const A c1 = lhs.get_lower() * rhs.get_lower();
            const A c2 = lhs.get_lower() * rhs.get_upper();
            const A c3 = lhs.get_upper() * rhs.get_lower();
            const A c4 = lhs.get_upper() * rhs.get_upper();

            A lo = std::numeric_limits<A>::infinity();
            A hi = -std::numeric_limits<A>::infinity();
            auto consider = [&](A v)
            {
                if (!std::isnan(v))
                {
                    lo = std::min(lo, v);
                    hi = std::max(hi, v);
                }
            };
            consider(c1);
            consider(c2);
            consider(c3);
            consider(c4);

            if (hi < lo)  // all corners were NaN (e.g., 0*inf only)
                return ClosedInterval<A>::reals();

            return ClosedInterval<A>(lo, hi);
        }

        case loki::BinaryOperatorEnum::PLUS:
        {
            const A lo = lhs.get_lower() + rhs.get_lower();
            const A hi = lhs.get_upper() + rhs.get_upper();
            return ClosedInterval<A>(std::isnan(lo) ? -std::numeric_limits<A>::infinity() : lo, std::isnan(hi) ? std::numeric_limits<A>::infinity() : hi);
        }

        default:
            throw std::logic_error("Evaluation of binary operator is undefined.");
    }
}

template<IsFloatingPoint A>
inline ClosedInterval<A> evaluate(loki::MultiOperatorEnum op, const ClosedInterval<A>& lhs, const ClosedInterval<A>& rhs)
{
    if (lhs.is_undefined() || rhs.is_undefined())
        return ClosedInterval<A>::undefined();

    switch (op)
    {
        case loki::MultiOperatorEnum::MUL:
        {
            return evaluate(loki::BinaryOperatorEnum::MUL, lhs, rhs);
        }

        case loki::MultiOperatorEnum::PLUS:
        {
            return evaluate(loki::BinaryOperatorEnum::PLUS, lhs, rhs);
        }

        default:
            throw std::logic_error("Evaluation of multi operator is undefined.");
    }
}

/**
 * Pretty printing
 */

template<IsFloatingPoint A>
inline std::ostream& operator<<(std::ostream& out, const ClosedInterval<A>& element)
{
    out << "[" << element.get_lower() << "," << element.get_upper() << "]";
    return out;
}

}

#endif