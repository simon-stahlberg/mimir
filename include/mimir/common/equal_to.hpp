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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_COMMON_EQUAL_TO_HPP_
#define MIMIR_COMMON_EQUAL_TO_HPP_

#include "mimir/common/concepts.hpp"

#include <functional>
#include <span>

namespace mimir
{

/// @brief Compare two spans for equality.
/// @tparam T
/// @param lhs
/// @param rhs
/// @return
template<typename T>
bool operator==(const std::span<T>& lhs, const std::span<T>& rhs)
{
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template<typename T>
concept IsShallowComparable = requires(T a)
{
    {
        a.is_equal_shallow(a)
        } -> std::same_as<size_t>;
};

template<typename T>
concept IsDeepComparable = requires(T a)
{
    {
        a.is_equal_deep(a)
        } -> std::same_as<size_t>;
};

template<typename T, bool Deref = false, bool Deep = false>
struct EqualTo
{
    bool operator()(const T& l, const T& r) const
    {
        if constexpr (Deref && IsDereferencable<T>)
        {
            if (!(l && r))
            {
                throw std::logic_error("EqualTo<T, Deref>::operator(): Tried to dereference a nullptr.");
            }
            using DereferencedType = std::decay_t<decltype(*l)>;
            return mimir::EqualTo<DereferencedType, Deref>()(*l, *r);
        }
        else if constexpr (Deep && IsDeepComparable<T>)
        {
            return l.is_equal_deep(r);
        }
        else if constexpr (!Deep && IsShallowComparable<T>)
        {
            return l.is_equal_shallow(r);
        }
        else
        {
            return std::equal_to<T>()(l, r);
        }
    }
};

}

#endif
