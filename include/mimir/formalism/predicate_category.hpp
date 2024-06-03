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

#ifndef MIMIR_FORMALISM_PREDICATE_CATEGORY_HPP_
#define MIMIR_FORMALISM_PREDICATE_CATEGORY_HPP_

#include <concepts>
#include <type_traits>
#include <unordered_map>

namespace mimir
{

/**
 * Dispatchers
 */

struct Static
{
};
struct Fluent
{
};
struct Derived
{
};

template<typename T>
concept PredicateCategory = std::is_same_v<T, Static> || std::is_same_v<T, Fluent> || std::is_same_v<T, Derived>;

template<typename T>
concept DynamicPredicateCategory = std::is_same_v<T, Fluent> || std::is_same_v<T, Derived>;

/**
 * Container to map predicate categorized types to some value.
 */

template<template<PredicateCategory> class T, PredicateCategory P, typename Value = T<P>>
struct PredicateCategorizedMapPair
{
    using KeyType = T<P>;
    using ValueType = Value;
};

template<typename T>
concept IsPredicateCategorizedMapPair = requires {
    typename T::KeyType::Category;                              // Ensure Category is a type member of KeyType
    requires PredicateCategory<typename T::KeyType::Category>;  // Check that Category is one of the allowed types
};

template<IsPredicateCategorizedMapPair... PredicateCategorizedMapPairs>
class PredicateMapContainer
{
private:
    std::tuple<std::unordered_map<typename PredicateCategorizedMapPairs::KeyType, typename PredicateCategorizedMapPairs::ValueType>...> storage;

public:
    /// @brief Access the respective map
    template<template<PredicateCategory> class T, PredicateCategory P, typename Value = T<P>>
    std::unordered_map<T<P>, Value>& get()
    {
        return std::get<std::unordered_map<T<P>, Value>>(storage);
    }
};

}

#endif
