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

#ifndef MIMIR_COMMON_PRINTERS_HPP_
#define MIMIR_COMMON_PRINTERS_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/common/deref_observer_ptr.hpp"
#include "mimir/common/deref_shared_ptr.hpp"

#include <array>
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

struct DotPrinterTag
{
};
struct ConsolePrinterTag
{
};

/**
 * Utility to write elements to a string using operator<< overload.
 */

template<typename T>
std::string to_string(const T& element)
{
    auto os = std::stringstream();
    os << element;
    return os.str();
}

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

template<typename T>
std::ostream& operator<<(std::ostream& os, const DerefObserverPtr<T>& ptr);

template<typename T>
std::ostream& operator<<(std::ostream& os, const DerefSharedPtr<T>& ptr);

/**
 * Definitions
 */

template<typename T, size_t N>
std::ostream& operator<<(std::ostream& os, const std::array<T, N>& arr)
{
    os << "<";
    for (size_t i = 0; i < arr.size(); ++i)
    {
        if (i != 0)
            os << ", ";
        os << arr[i];
    }
    os << ">";
    return os;
}

template<typename Key, typename T, typename Compare, typename Allocator>
std::ostream& operator<<(std::ostream& os, const std::map<Key, T, Compare, Allocator>& map)
{
    os << "{";
    for (auto it = map.begin(); it != map.end(); ++it)
    {
        if (it != map.begin())
            os << ", ";
        os << *it;
    }
    os << "}";
    return os;
}

template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const std::pair<T1, T2>& pair)
{
    os << "<" << pair.first << "," << pair.second << ">";
    return os;
}

template<typename Key, typename Compare, typename Allocator>
std::ostream& operator<<(std::ostream& os, const std::set<Key, Compare, Allocator>& set)
{
    os << "{";
    size_t i = 0;
    for (const auto& element : set)
    {
        if (i != 0)
            os << ", ";
        os << element;
        ++i;
    }
    os << "}";
    return os;
}

template<typename... Ts>
std::ostream& operator<<(std::ostream& os, const std::tuple<Ts...>& tuple)
{
    os << "<";
    std::size_t n = 0;
    std::apply([&os, &n](const Ts&... args) { ((os << (n++ == 0 ? "" : ", ") << args), ...); }, tuple);
    os << ">";
    return os;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
std::ostream& operator<<(std::ostream& os, const std::unordered_map<Key, T, Hash, KeyEqual, Allocator>& map)
{
    os << "{";
    for (auto it = map.begin(); it != map.end(); ++it)
    {
        if (it != map.begin())
            os << ", ";
        os << *it;
    }
    os << "}";
    return os;
}

template<typename Key, typename Hash, typename KeyEqual, typename Allocator>
std::ostream& operator<<(std::ostream& os, const std::unordered_set<Key, Hash, KeyEqual, Allocator>& set)
{
    os << "{";
    size_t i = 0;
    for (const auto& element : set)
    {
        if (i != 0)
            os << ", ";
        os << element;
        ++i;
    }
    os << "}";
    return os;
}

template<typename T, typename Allocator>
std::ostream& operator<<(std::ostream& os, const std::vector<T, Allocator>& vec)
{
    os << "[";
    for (size_t i = 0; i < vec.size(); ++i)
    {
        if (i != 0)
            os << ", ";
        os << vec[i];
    }
    os << "]";
    return os;
}

template<typename T1>
std::ostream& operator<<(std::ostream& os, const std::variant<T1>& variant)
{
    std::visit([&](auto&& arg) { os << arg; }, variant);
    return os;
}

template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const std::variant<T1, T2>& variant)
{
    std::visit([&](auto&& arg) { os << arg; }, variant);
    return os;
}

template<typename T1, typename T2, typename T3>
std::ostream& operator<<(std::ostream& os, const std::variant<T1, T2, T3>& variant)
{
    std::visit([&](auto&& arg) { os << arg; }, variant);
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

                              os << "{ " << KeyType::name << " : " << value << " }, ";
                          });
    os << " }";
    return os;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const DerefObserverPtr<T>& ptr)
{
    os << *ptr;
    return os;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const DerefSharedPtr<T>& ptr)
{
    os << *ptr;
    return os;
}

}

#endif
