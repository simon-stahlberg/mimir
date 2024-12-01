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

#include <array>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mimir
{

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

template<typename T, size_t K>
std::ostream& operator<<(std::ostream& os, const std::array<T, K>& arr);

template<typename K, typename V>
std::ostream& operator<<(std::ostream& os, const std::map<K, V>& map);

template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const std::pair<T1, T2>& pair);

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::set<T>& set);

template<typename... Ts>
std::ostream& operator<<(std::ostream& os, const std::tuple<Ts...>& tuple);

template<typename K, typename V>
std::ostream& operator<<(std::ostream& os, const std::unordered_map<K, V>& map);

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::unordered_set<T>& set);

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec);

/**
 * Definitions
 */

template<typename T, size_t K>
std::ostream& operator<<(std::ostream& os, const std::array<T, K>& arr)
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

template<typename K, typename V>
std::ostream& operator<<(std::ostream& os, const std::map<K, V>& map)
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

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::set<T>& set)
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

template<typename K, typename V>
std::ostream& operator<<(std::ostream& os, const std::unordered_map<K, V>& map)
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

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::unordered_set<T>& set)
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

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec)
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
}

#endif
