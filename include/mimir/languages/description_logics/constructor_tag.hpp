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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTOR_CATEGORY_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTOR_CATEGORY_HPP_

#include <concepts>
#include <string>

namespace mimir::dl
{

/**
 * ID classes.
 *
 * We need those to write generic code independent of concept or role.
 */

struct Concept
{
    static constexpr std::string name = "concept";
};

struct Role
{
    static constexpr std::string name = "role";
};

struct Boolean
{
    static constexpr std::string name = "boolean";
};

struct Numerical
{
    static constexpr std::string name = "numerical";
};

/**
 * Concepts
 */

template<typename T>
concept DescriptionLogicCategory = std::is_same<T, Concept>::value || std::is_same<T, Role>::value;

template<typename T>
concept FeatureCategory =
    std::is_same<T, Concept>::value || std::is_same<T, Role>::value || std::is_same<T, Boolean>::value || std::is_same<T, Numerical>::value;

}

#endif
