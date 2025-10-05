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

#include <boost/numeric/interval.hpp>
#include <cmath>
#include <limits>
#include <loki/loki.hpp>
#include <ostream>
#include <type_traits>

namespace mimir
{
namespace bn = boost::numeric;
namespace ivl = bn::interval_lib;

template<IsFloatingPoint A>
using IntervalPolicies = ivl::policies<ivl::save_state<ivl::rounded_transc_std<A>>,  // sound rounding
                                       ivl::checking_base<A>                         // no throws on empty/∞
                                       >;

template<IsFloatingPoint A>
using Interval = bn::interval<A, IntervalPolicies<A>>;

static_assert(sizeof(Interval<double>) == 16);

template<IsFloatingPoint A>
class ClosedInterval
{
public:
    /**
     * Constructors
     */

    constexpr ClosedInterval() : m_interval(Interval<A>::empty()) {}
    constexpr ClosedInterval(A lower, A upper) : m_interval(lower, upper) {}
    constexpr ClosedInterval(Interval<A> interval) : m_interval(interval) {}

    /**
     * Operators
     */

    friend bool operator==(const ClosedInterval& lhs, const ClosedInterval& rhs) noexcept
    {
        if (empty(lhs) && empty(rhs))
            return true;
        return lhs.get_lower() == rhs.get_lower() && lhs.get_upper() == rhs.get_upper();
    }

    friend bool operator!=(const ClosedInterval& lhs, const ClosedInterval& rhs) noexcept { return !(lhs == rhs); }

    friend ClosedInterval operator+(const ClosedInterval& lhs, const ClosedInterval& rhs) noexcept
    {
        return ClosedInterval(lhs.get_interval() + rhs.get_interval());
    }

    friend ClosedInterval operator-(const ClosedInterval& lhs, const ClosedInterval& rhs) noexcept
    {
        return ClosedInterval(lhs.get_interval() - rhs.get_interval());
    }

    friend ClosedInterval operator*(const ClosedInterval& lhs, const ClosedInterval& rhs) noexcept
    {
        return ClosedInterval(lhs.get_interval() * rhs.get_interval());
    }

    friend ClosedInterval operator/(const ClosedInterval& lhs, const ClosedInterval& rhs) noexcept
    {
        return ClosedInterval(lhs.get_interval() / rhs.get_interval());
    }

    friend ClosedInterval intersect(const ClosedInterval& lhs, const ClosedInterval& rhs) noexcept
    {
        return ClosedInterval(bn::intersect(lhs.m_interval, rhs.m_interval));
    }

    friend ClosedInterval hull(const ClosedInterval& lhs, const ClosedInterval& rhs) noexcept
    {
        return ClosedInterval(bn::hull(lhs.m_interval, rhs.m_interval));
    }

    /**
     * Accessors
     */

    friend bool empty(const ClosedInterval& x) noexcept { return bn::empty(x.m_interval); }

    /**
     * Getters
     */

    constexpr const Interval<A>& get_interval() const noexcept { return m_interval; }

    A get_lower() const noexcept { return lower(m_interval); }
    A get_upper() const noexcept { return upper(m_interval); }

private:
    Interval<A> m_interval;
};

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

template<IsArithmetic A>
inline bool evaluate(loki::BinaryComparatorEnum comparator, const ClosedInterval<A>& lhs, const ClosedInterval<A>& rhs)
{
    if (empty(lhs) || empty(rhs))
        return false;

    switch (comparator)
    {
        case loki::BinaryComparatorEnum::EQUAL:
        {
            // ∃ x ∈ lhs, ∃ y ∈ rhs : x = y.
            return lhs.get_lower() <= rhs.get_upper() && lhs.get_upper() >= rhs.get_lower();
        }
        case loki::BinaryComparatorEnum::GREATER:
        {
            // ∃ x ∈ lhs, ∃ y ∈ rhs : x > y.
            return lhs.get_upper() > rhs.get_lower();
        }
        case loki::BinaryComparatorEnum::GREATER_EQUAL:
        {
            // ∃ x ∈ lhs, ∃ y ∈ rhs : x >= y.
            return lhs.get_upper() >= rhs.get_lower();
        }
        case loki::BinaryComparatorEnum::LESS:
        {
            // ∃ x ∈ lhs, ∃ y ∈ rhs : x < y.
            return lhs.get_lower() < rhs.get_upper();
        }
        case loki::BinaryComparatorEnum::LESS_EQUAL:
        {
            // ∃ x ∈ lhs, ∃ y ∈ rhs : x <= y.
            return lhs.get_lower() <= rhs.get_upper();
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
    out << "[" << element.get_lower() << "," << element.get_upper() << "]";
    return out;
}

}

#endif