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

#include "init_declarations.hpp"

#include <iostream>
#include <iterator>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

using namespace mimir;

namespace py = pybind11;

/**
 * Type casters
 */

template<>
struct py::detail::type_caster<FlatIndexList>
{
public:
    PYBIND11_TYPE_CASTER(FlatIndexList, _("FlatIndexList"));

    static py::handle cast(const FlatIndexList& cpp_list, py::return_value_policy, py::handle)
    {
        py::list py_list;
        for (const auto& cpp_item : cpp_list)
        {
            py_list.append(py::cast(cpp_item));
        }
        return py_list.release();
    }
};

template<>
struct py::detail::type_caster<FlatDoubleList>
{
public:
    PYBIND11_TYPE_CASTER(FlatDoubleList, _("FlatDoubleList"));

    static py::handle cast(const FlatDoubleList& cpp_list, py::return_value_policy, py::handle)
    {
        py::list py_list;
        for (const auto& cpp_item : cpp_list)
        {
            py_list.append(py::cast(cpp_item));
        }
        return py_list.release();
    }
};

template<>
struct py::detail::type_caster<FlatBitset>
{
public:
    PYBIND11_TYPE_CASTER(FlatBitset, _("FlatBitset"));

    static py::handle cast(const FlatBitset& cpp_bitset, py::return_value_policy, py::handle)
    {
        py::list py_list;
        for (const auto& cpp_item : cpp_bitset)
        {
            py_list.append(py::cast(cpp_item));
        }
        return py_list.release();
    }
};
