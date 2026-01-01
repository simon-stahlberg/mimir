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

#include "mimir/common/declarations.hpp"

#include <boost/numeric/interval.hpp>
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
    using IntervalPolicies =
        boost::numeric::interval_lib::policies<boost::numeric::interval_lib::save_state<boost::numeric::interval_lib::rounded_transc_std<A>>,  // sound rounding
                                               boost::numeric::interval_lib::checking_base<A>  // no throws on empty/∞
                                               >;

    using Interval = boost::numeric::interval<A, IntervalPolicies>;

    /**
     * Constructors
     */

    ClosedInterval() : m_interval(Interval::empty()) {}
    ClosedInterval(A lower, A upper) : m_interval(lower, upper) {}
    ClosedInterval(Interval interval) : m_interval(interval) {}

    /**
     * Operators
     */

    friend bool operator==(const ClosedInterval& lhs, const ClosedInterval& rhs) noexcept
    {
        if (empty(lhs) && empty(rhs))
            return true;
        if (empty(lhs) ^ empty(rhs))
            return false;
        return lower(lhs) == lower(rhs) && upper(lhs) == upper(rhs);
    }

    friend bool operator!=(const ClosedInterval& lhs, const ClosedInterval& rhs) noexcept { return !(lhs == rhs); }

    friend ClosedInterval operator+(const ClosedInterval& lhs, const ClosedInterval& rhs) noexcept
    {
        return ClosedInterval(boost::numeric::operator+(lhs.get_interval(), rhs.get_interval()));
    }

    friend ClosedInterval operator-(const ClosedInterval& lhs, const ClosedInterval& rhs) noexcept
    {
        return ClosedInterval(boost::numeric::operator-(lhs.get_interval(), rhs.get_interval()));
    }

    friend ClosedInterval operator-(const ClosedInterval& el) noexcept { return ClosedInterval(boost::numeric::operator-(el.get_interval())); }

    friend ClosedInterval operator*(const ClosedInterval& lhs, const ClosedInterval& rhs) noexcept
    {
        return ClosedInterval(boost::numeric::operator*(lhs.get_interval(), rhs.get_interval()));
    }

    friend ClosedInterval operator/(const ClosedInterval& lhs, const ClosedInterval& rhs) noexcept
    {
        return ClosedInterval(boost::numeric::operator/(lhs.get_interval(), rhs.get_interval()));
    }

    friend ClosedInterval intersect(const ClosedInterval& lhs, const ClosedInterval& rhs) noexcept
    {
        return ClosedInterval(boost::numeric::intersect(lhs.m_interval, rhs.m_interval));
    }

    friend ClosedInterval hull(const ClosedInterval& lhs, const ClosedInterval& rhs) noexcept
    {
        return ClosedInterval(boost::numeric::hull(lhs.m_interval, rhs.m_interval));
    }

    /**
     * Accessors
     */

    friend bool empty(const ClosedInterval& x) noexcept { return boost::numeric::empty(x.m_interval); }

    /**
     * Getters
     */

    constexpr const Interval& get_interval() const noexcept { return m_interval; }

    friend A lower(const ClosedInterval& el) noexcept { return lower(el.get_interval()); }
    friend A upper(const ClosedInterval& el) noexcept { return upper(el.get_interval()); }

private:
    Interval m_interval;
};

static_assert(sizeof(ClosedInterval<double>) == 16);

/**
 * ClosedInterval arithmetics
 */

template<IsFloatingPoint A>
inline ClosedInterval<A> evaluate(loki::BinaryOperatorEnum op, const ClosedInterval<A>& lhs, const ClosedInterval<A>& rhs)
{
    if (empty(lhs) || empty(rhs))
        return ClosedInterval<A>();  // undefined sentinel

    switch (op)
    {
        case loki::BinaryOperatorEnum::DIV:
        {
            return lhs / rhs;
        }

        case loki::BinaryOperatorEnum::MINUS:
        {
            return lhs - rhs;
        }

        case loki::BinaryOperatorEnum::MUL:
        {
            return lhs * rhs;
        }

        case loki::BinaryOperatorEnum::PLUS:
        {
            return lhs + rhs;
        }

        default:
            throw std::logic_error("Evaluation of binary operator is undefined.");
    }
}

template<IsFloatingPoint A>
inline ClosedInterval<A> evaluate(loki::MultiOperatorEnum op, const ClosedInterval<A>& lhs, const ClosedInterval<A>& rhs)
{
    if (empty(lhs) || empty(rhs))
        return ClosedInterval<A>();

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

template<IsFloatingPoint A>
inline bool evaluate(loki::BinaryComparatorEnum comparator, const ClosedInterval<A>& lhs, const ClosedInterval<A>& rhs)
{
    if (empty(lhs) || empty(rhs))
        return false;

    switch (comparator)
    {
        case loki::BinaryComparatorEnum::EQUAL:
        {
            // ∃ x ∈ lhs, ∃ y ∈ rhs : x = y.
            return lower(lhs) <= upper(rhs) && upper(lhs) >= lower(rhs);
        }
        case loki::BinaryComparatorEnum::UNEQUAL:
        {
            // ∃ x ∈ lhs, ∃ y ∈ rhs : x ≠ y
            // False iff both are singletons with the same value
            return !(lower(lhs) == upper(lhs) && lower(rhs) == upper(rhs) && lower(lhs) == lower(rhs));
        }
        case loki::BinaryComparatorEnum::GREATER:
        {
            // ∃ x ∈ lhs, ∃ y ∈ rhs : x > y.
            return upper(lhs) > lower(rhs);
        }
        case loki::BinaryComparatorEnum::GREATER_EQUAL:
        {
            // ∃ x ∈ lhs, ∃ y ∈ rhs : x >= y.
            return upper(lhs) >= lower(rhs);
        }
        case loki::BinaryComparatorEnum::LESS:
        {
            // ∃ x ∈ lhs, ∃ y ∈ rhs : x < y.
            return lower(lhs) < upper(rhs);
        }
        case loki::BinaryComparatorEnum::LESS_EQUAL:
        {
            // ∃ x ∈ lhs, ∃ y ∈ rhs : x <= y.
            return lower(lhs) <= upper(rhs);
        }
        default:
        {
            throw std::logic_error("evaluate(comparator, lhs, rhs): Unexpected BinaryComparatorEnum.");
        }
    }
}

/**
 * Pretty printing
 */

template<IsFloatingPoint A>
inline std::ostream& operator<<(std::ostream& out, const ClosedInterval<A>& element)
{
    out << "[" << lower(element) << "," << upper(element) << "]";
    return out;
}

}

#endif