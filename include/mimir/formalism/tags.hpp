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

namespace mimir::formalism
{

/// @brief `StaticTag` is a tag to declare that the semantic interpretation of a type remains constants across all states.
struct StaticTag
{
    static constexpr const char* name = "static";
};
/// @brief `FluentTag` is a tag to declare that the semantic interpretation of a type can change across states due to actions.
struct FluentTag
{
    static constexpr const char* name = "fluent";
};
/// @brief `DerivedTag` is a tag to declare that the semantic interpretation of a type can change across states due to axioms.
struct DerivedTag
{
    static constexpr const char* name = "derived";
};
/// @brief `AuxiliaryTag` is a tag indicating that the value of this type is used for metric evaluation and does not contribute to defining the state. For
/// example, total-cost in cost-sensitive planning.
struct AuxiliaryTag
{
    static constexpr const char* name = "auxiliary";
};

template<typename T>
concept IsStaticOrFluentOrDerivedTag = std::is_same_v<T, StaticTag> || std::is_same_v<T, FluentTag> || std::is_same_v<T, DerivedTag>;

template<typename T>
concept IsFluentOrDerivedTag = std::is_same_v<T, FluentTag> || std::is_same_v<T, DerivedTag>;

template<typename T>
concept IsStaticOrFluentOrAuxiliaryTag = std::is_same_v<T, StaticTag> || std::is_same_v<T, FluentTag> || std::is_same_v<T, AuxiliaryTag>;

template<typename T>
concept IsStaticOrFluentTag = std::is_same_v<T, StaticTag> || std::is_same_v<T, FluentTag>;

template<typename T>
concept IsFluentOrAuxiliaryTag = std::is_same_v<T, FluentTag> || std::is_same_v<T, AuxiliaryTag>;

}

#endif
