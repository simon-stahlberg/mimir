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

#include <boost/hana.hpp>
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

template<typename T, typename Value>
concept IsRangeOver = std::ranges::range<T> && std::same_as<std::ranges::range_value_t<T>, Value>;

template<typename T, typename Value>
concept IsConvertibleRangeOver = std::ranges::range<T> && std::convertible_to<std::ranges::range_value_t<T>, Value>;

template<typename T>
concept IsBackInsertibleRange = std::ranges::forward_range<T> &&                       // Must be a forward range
                                std::default_initializable<std::remove_cvref_t<T>> &&  // Must be default-constructible
                                requires(std::remove_cvref_t<T> container, std::ranges::range_value_t<T> value) {
                                    std::back_inserter(container);           // Must support std::back_inserter
                                    *std::back_inserter(container) = value;  // Must support inserting values
                                };

// Define a concept that checks if T is one of Ts...
template<typename T, typename... Ts>
concept InTypes = (std::same_as<T, Ts> || ...);

template<typename T>
concept IsHanaMap = std::same_as<typename boost::hana::tag_of<T>::type, boost::hana::map_tag>;

// Define a concept that checks whether T is a std::variant.
template<typename T>
concept IsVariant = requires { typename std::variant_size<T>::type; };

template<typename T>
concept IsDereferencable = requires(T t) {
    { *t } -> std::convertible_to<const typename std::remove_reference_t<decltype(*t)>>;
};

}

#endif
