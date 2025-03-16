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
#include <string>
#include <type_traits>

namespace mimir::formalism
{

/// @brief `Static` is a tag to declare that the semantic interpretation of a type remains constants across all states.
struct Static
{
    static constexpr std::string name = "static";
};
/// @brief `Fluent` is a tag to declare that the semantic interpretation of a type can change across states due to actions.
struct Fluent
{
    static constexpr std::string name = "fluent";
};
/// @brief `Derived` is a tag to declare that the semantic interpretation of a type can change across states due to axioms.
struct Derived
{
    static constexpr std::string name = "derived";
};
/// @brief `Auxiliary` is a tag indicating that the value of this type is used for metric evaluation and does not contribute to defining the state. For example,
/// total-cost in cost-sensitive planning.
struct Auxiliary
{
    static constexpr std::string name = "auxiliary";
};

template<typename T>
concept StaticOrFluentOrDerived = std::is_same_v<T, Static> || std::is_same_v<T, Fluent> || std::is_same_v<T, Derived>;

template<typename T>
concept FluentOrDerived = std::is_same_v<T, Fluent> || std::is_same_v<T, Derived>;

template<typename T>
concept StaticOrFluentOrAuxiliary = std::is_same_v<T, Static> || std::is_same_v<T, Fluent> || std::is_same_v<T, Auxiliary>;

template<typename T>
concept StaticOrFluent = std::is_same_v<T, Static> || std::is_same_v<T, Fluent>;

template<typename T>
concept FluentOrAuxiliary = std::is_same_v<T, Fluent> || std::is_same_v<T, Auxiliary>;

}

#endif
