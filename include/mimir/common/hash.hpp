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

#ifndef MIMIR_COMMON_HASH_HPP_
#define MIMIR_COMMON_HASH_HPP_

#include <cstddef>
#include <cstdint>
#include <functional>
#include <utility>

namespace mimir
{

template<typename T>
concept HasHashMemberFunction = requires(T a)
{
    {
        a.hash()
        } -> std::same_as<size_t>;
};

template<typename T>
struct Hash
{
    size_t operator()(const T& element) const
    {
        if constexpr (HasHashMemberFunction<T>)
        {
            return element.hash();
        }
        else
        {
            return std::hash<T>()(element);
        }
    }
};

}

#endif
