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
 * along with this program. If not, see <httIs://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_COMMON_TYPE_INDICES_HPP_
#define MIMIR_COMMON_TYPE_INDICES_HPP_

#include <concepts>

namespace mimir
{

template<typename... Ts>
struct TypeIndices
{
};

template<typename T>
struct is_type_indices : std::false_type
{
};

template<typename... Ts>
struct is_type_indices<TypeIndices<Ts...>> : std::true_type
{
};

template<typename T>
concept IsTypeIndices = is_type_indices<T>::value;

}

#endif
