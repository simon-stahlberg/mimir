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

#ifndef MIMIR_COMMON_CONCEPTS_HPP_
#define MIMIR_COMMON_CONCEPTS_HPP_

#include <concepts>
#include <cstddef>
#include <memory>
#include <ranges>
#include <type_traits>
#include <variant>

namespace mimir
{

template<typename T>
struct dependent_false : std::false_type
{
};

template<typename T>
concept IsArithmetic = std::is_arithmetic_v<T>;

template<typename T>
concept IsIntegral = std::is_integral_v<T>;

template<typename T>
concept IsUnsignedIntegral = std::is_integral_v<T> && std::is_unsigned_v<T>;

template<typename T, typename Value>
concept IsRangeOver = std::ranges::range<T> && std::same_as<std::ranges::range_value_t<T>, Value>;

}

#endif
