/*
 * Copyright (C) 2023-2025 Dominik Drexler and Simon Stahlberg
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

#ifndef MIMIR_STL_BIND_HPP
#define MIMIR_STL_BIND_HPP

#include <concepts>
#include <memory>
#include <pybind11/cast.h>
#include <pybind11/operators.h>

namespace mimir
{
template<typename T>
struct PyImmutable
{
    explicit PyImmutable(const T& obj) : obj_(obj) {}

    const T& obj_;  // Read-only reference
};

struct Mutable
{
};
struct Immutable
{
};

template<typename T>
concept MutableOrImmutable = std::is_same<T, Mutable> || std::is_same<T, Immutable>;

template<MutableOrImmutable M, typename Vector, typename holder_type = std::unique_ptr<Vector>, typename... Args>
class_<Vector, holder_type> bind_vector(M is_mutable_tag, handle scope, std::string const& name, Args&&... args)
{
    using Class_ = class_<Vector, holder_type>;
}

}

#endif