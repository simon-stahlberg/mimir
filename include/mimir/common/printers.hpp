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

#include <memory>
#include <ostream>
#include <set>
#include <tuple>
#include <unordered_set>
#include <vector>

namespace mimir
{

/**
 * Forward declarations
 */

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<const T*>& vec);

template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const std::pair<T1, T2>& pair);

template<typename... Ts>
std::ostream& operator<<(std::ostream& os, const std::tuple<Ts...>& tuple);

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec);

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::unordered_set<const T*>& set);

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::unordered_set<T>& set);

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::set<T>& set);

/**
 * Definitions
 */

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<const T*>& vec)
{
    os << "[";
    for (size_t i = 0; i < vec.size(); ++i)
    {
        if (i != 0)
            os << ", ";
        os << *vec[i];
    }
    os << "]";
    return os;
}

template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const std::pair<T1, T2>& pair)
{
    os << "<" << pair.first << "," << pair.second << ">";
    return os;
}

template<typename... Ts>
std::ostream& operator<<(std::ostream& os, const std::tuple<Ts...>& tuple)
{
    os << "<";
    std::size_t n = 0;
    std::apply([&os, &tuple, &n](const Ts&... args) { ((os << (n++ == 0 ? "" : ", ") << args), ...); }, tuple);
    os << ">";
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

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::unordered_set<const T*>& set)
{
    os << "{";
    size_t i = 0;
    for (const auto& element : set)
    {
        if (i != 0)
            os << ", ";
        os << *element;
        ++i;
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
}

#endif
