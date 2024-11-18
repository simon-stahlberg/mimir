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
concept PredicateTag = std::is_same_v<T, Static> || std::is_same_v<T, Fluent> || std::is_same_v<T, Derived>;

template<typename T>
concept DynamicPredicateTag = std::is_same_v<T, Fluent> || std::is_same_v<T, Derived>;

}

#endif
