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
 * StaticOrFluentOrDerived
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
struct Auxiliary
{
};

template<typename T>
concept StaticOrFluentOrDerived = std::is_same_v<T, Static> || std::is_same_v<T, Fluent> || std::is_same_v<T, Derived>;

template<typename T>
concept FluentOrDerived = std::is_same_v<T, Fluent> || std::is_same_v<T, Derived>;

template<typename T>
concept StaticOrFluentOrAuxiliary = std::is_same_v<T, Static> || std::is_same_v<T, Fluent> || std::is_same_v<T, Auxiliary>;

template<typename T>
concept StaticOrFluentTag = std::is_same_v<T, Static> || std::is_same_v<T, Fluent>;

template<typename T>
concept FluentOrAuxiliary = std::is_same_v<T, Fluent> || std::is_same_v<T, Auxiliary>;

}

#endif
