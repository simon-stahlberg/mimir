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

#ifndef MIMIR_COMMON_FORMATTER_HPP_
#define MIMIR_COMMON_FORMATTER_HPP_

#include "mimir/common/declarations.hpp"

#include <array>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace mimir
{

/**
 * Forward declarations
 */

template<typename T, size_t N>
std::ostream& operator<<(std::ostream& os, const std::array<T, N>& arr);

template<typename Key, typename T, typename Compare, typename Allocator>
std::ostream& operator<<(std::ostream& os, const std::map<Key, T, Compare, Allocator>& map);

template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const std::pair<T1, T2>& pair);

template<typename Key, typename Compare, typename Allocator>
std::ostream& operator<<(std::ostream& os, const std::set<Key, Compare, Allocator>& set);

template<typename... Ts>
std::ostream& operator<<(std::ostream& os, const std::tuple<Ts...>& tuple);

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
std::ostream& operator<<(std::ostream& os, const std::unordered_map<Key, T, Hash, KeyEqual, Allocator>& map);

template<typename Key, typename Hash, typename KeyEqual, typename Allocator>
std::ostream& operator<<(std::ostream& os, const std::unordered_set<Key, Hash, KeyEqual, Allocator>& set);

template<typename T, typename Allocator>
std::ostream& operator<<(std::ostream& os, const std::vector<T, Allocator>& vec);

template<typename T1>
std::ostream& operator<<(std::ostream& os, const std::variant<T1>& variant);

template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const std::variant<T1, T2>& variant);

template<typename T1, typename T2, typename T3>
std::ostream& operator<<(std::ostream& os, const std::variant<T1, T2, T3>& variant);

template<IsHanaMap Map>
std::ostream& operator<<(std::ostream& os, const Map& map);

/**
 * ADL-enabled stream helper: finds operator<< in the type's namespace
 */

template<class T>
std::ostream& print(std::ostream& os, const T& t)
{
    return os << t;
}

/**
 * Helpers to materialize strings
 */

template<typename T>
std::string to_string(const T& element)
{
    std::stringstream ss;

    if constexpr (requires { *element; })
    {
        if (element)
            ss << *element;
        else
            ss << "<nullptr>";
    }
    else
    {
        ss << element;
    }

    return ss.str();
}

template<std::ranges::input_range Range>
std::vector<std::string> to_strings(const Range& range)
{
    auto result = std::vector<std::string> {};
    if constexpr (std::ranges::sized_range<Range>)
        result.reserve(std::ranges::size(range));
    for (const auto& element : range)
        result.push_back(to_string(element));
    return result;
}

/**
 * Definitions
 */

template<typename T, size_t N>
std::ostream& operator<<(std::ostream& os, const std::array<T, N>& arr)
{
    fmt::print(os, "<{}>", fmt::join(to_strings(arr), ", "));
    return os;
}

template<typename Key, typename T, typename Compare, typename Allocator>
std::ostream& operator<<(std::ostream& os, const std::map<Key, T, Compare, Allocator>& map)
{
    fmt::print(os, "{{{}}}", fmt::join(to_strings(map), ", "));
    return os;
}

template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const std::pair<T1, T2>& pair)
{
    fmt::print(os, "<{},{}>", to_string(pair.first), to_string(pair.second));
    return os;
}

template<typename Key, typename Compare, typename Allocator>
std::ostream& operator<<(std::ostream& os, const std::set<Key, Compare, Allocator>& set)
{
    fmt::print(os, "{{{}}}", fmt::join(to_strings(set), ", "));
    return os;
}

template<typename... Ts>
std::ostream& operator<<(std::ostream& os, const std::tuple<Ts...>& tuple)
{
    os << "<";
    if constexpr (sizeof...(Ts) > 0)
    {
        std::size_t n = 0;
        std::apply([&os, &n](const Ts&... args) { ((os << (n++ == 0 ? "" : ", ") << to_string(args)), ...); }, tuple);
    }
    os << ">";
    return os;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
std::ostream& operator<<(std::ostream& os, const std::unordered_map<Key, T, Hash, KeyEqual, Allocator>& map)
{
    fmt::print(os, "{{{}}}", fmt::join(to_strings(map), ", "));
    return os;
}

template<typename Key, typename Hash, typename KeyEqual, typename Allocator>
std::ostream& operator<<(std::ostream& os, const std::unordered_set<Key, Hash, KeyEqual, Allocator>& set)
{
    fmt::print(os, "{{{}}}", fmt::join(to_strings(set), ", "));
    return os;
}

template<typename T, typename Allocator>
std::ostream& operator<<(std::ostream& os, const std::vector<T, Allocator>& vec)
{
    fmt::print(os, "[{}]", fmt::join(to_strings(vec), ", "));
    return os;
}

template<typename T1>
std::ostream& operator<<(std::ostream& os, const std::variant<T1>& variant)
{
    std::visit([&](auto&& arg) { os << to_string(arg); }, variant);
    return os;
}

template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const std::variant<T1, T2>& variant)
{
    std::visit([&](auto&& arg) { os << to_string(arg); }, variant);
    return os;
}

template<typename T1, typename T2, typename T3>
std::ostream& operator<<(std::ostream& os, const std::variant<T1, T2, T3>& variant)
{
    std::visit([&](auto&& arg) { os << to_string(arg); }, variant);
    return os;
}

template<IsHanaMap Map>
std::ostream& operator<<(std::ostream& os, const Map& map)
{
    os << "{ ";
    boost::hana::for_each(map,
                          [&os](auto&& pair)
                          {
                              const auto& key = boost::hana::first(pair);
                              const auto& value = boost::hana::second(pair);

                              using KeyType = typename decltype(+key)::type;

                              os << "{ " << KeyType::name << " : " << to_string(value) << " }, ";
                          });
    os << " }";
    return os;
}

}

#endif
