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

#ifndef MIMIR_FORMALISM_TAGS_HPP_
#define MIMIR_FORMALISM_TAGS_HPP_

#include <concepts>
#include <type_traits>

namespace mimir
{

/**
 * PredicateTag
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
concept PredicateTag = std::is_same_v<T, Static> || std::is_same_v<T, Fluent> || std::is_same_v<T, Derived>;

template<typename T>
concept DynamicPredicateTag = std::is_same_v<T, Fluent> || std::is_same_v<T, Derived>;

/**
 * FunctionTag
 */

/// @brief `Static` never appear in a condition.
/// Hence, their values stay fixed and can be stored in the `Problem`.

/// @brief `Fluent` appear at least once in an action effect. They must appear in conditions as well, or otherwise, they are considered Auxiliary.
/// They must be stored in the state.

/// @brief `Auxiliary` appear in effects but never in conditions.
/// They are typically used to define search metrics, which are stored in `SearchNode`
struct Auxiliary
{
};

template<typename T>
concept FunctionTag = std::is_same_v<T, Static> || std::is_same_v<T, Fluent> || std::is_same_v<T, Auxiliary>;

template<typename T>
concept StaticOrFluentTag = std::is_same_v<T, Static> || std::is_same_v<T, Fluent>;

template<typename T>
concept DynamicFunctionTag = std::is_same_v<T, Fluent> || std::is_same_v<T, Auxiliary>;

}

#endif
