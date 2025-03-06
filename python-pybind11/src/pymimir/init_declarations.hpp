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

#ifndef MIMIR_INIT_DECLARATIONS_HPP
#define MIMIR_INIT_DECLARATIONS_HPP

#include "mimir/mimir.hpp"

#include <iostream>
#include <iterator>
#include <pybind11/attr.h>
#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // Necessary for automatic conversion of e.g. std::vectors
#include <pybind11/stl_bind.h>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;
namespace mm = mimir;

/**
 * Opague types
 */

// Formalism
PYBIND11_MAKE_OPAQUE(mm::ActionList);
PYBIND11_MAKE_OPAQUE(mm::AtomList<mm::Static>);
PYBIND11_MAKE_OPAQUE(mm::AtomList<mm::Fluent>);
PYBIND11_MAKE_OPAQUE(mm::AtomList<mm::Derived>);
PYBIND11_MAKE_OPAQUE(mm::AxiomList);
PYBIND11_MAKE_OPAQUE(mm::ConditionalEffectList);
PYBIND11_MAKE_OPAQUE(mm::FunctionExpressionList);
PYBIND11_MAKE_OPAQUE(mm::FunctionSkeletonList<mm::Static>);
PYBIND11_MAKE_OPAQUE(mm::FunctionSkeletonList<mm::Fluent>);
PYBIND11_MAKE_OPAQUE(mm::FunctionSkeletonList<mm::Auxiliary>);
PYBIND11_MAKE_OPAQUE(mm::FunctionList<mm::Static>);
PYBIND11_MAKE_OPAQUE(mm::FunctionList<mm::Fluent>);
PYBIND11_MAKE_OPAQUE(mm::FunctionList<mm::Auxiliary>);
PYBIND11_MAKE_OPAQUE(mm::GroundAtomList<mm::Static>);
PYBIND11_MAKE_OPAQUE(mm::GroundAtomList<mm::Fluent>);
PYBIND11_MAKE_OPAQUE(mm::GroundAtomList<mm::Derived>);
PYBIND11_MAKE_OPAQUE(mm::GroundFunctionExpressionList);
PYBIND11_MAKE_OPAQUE(mm::GroundLiteralList<mm::Static>);
PYBIND11_MAKE_OPAQUE(mm::GroundLiteralList<mm::Fluent>);
PYBIND11_MAKE_OPAQUE(mm::GroundLiteralList<mm::Derived>);
PYBIND11_MAKE_OPAQUE(mm::GroundActionList);
PYBIND11_MAKE_OPAQUE(mm::GroundAxiomList);
PYBIND11_MAKE_OPAQUE(mm::LiteralList<mm::Static>);
PYBIND11_MAKE_OPAQUE(mm::LiteralList<mm::Fluent>);
PYBIND11_MAKE_OPAQUE(mm::LiteralList<mm::Derived>);
PYBIND11_MAKE_OPAQUE(mm::GroundFunctionValueList<mm::Static>);
PYBIND11_MAKE_OPAQUE(mm::GroundFunctionValueList<mm::Fluent>);
PYBIND11_MAKE_OPAQUE(mm::GroundFunctionValueList<mm::Auxiliary>);
PYBIND11_MAKE_OPAQUE(mm::ObjectList);
PYBIND11_MAKE_OPAQUE(mm::PredicateList<mm::Static>);
PYBIND11_MAKE_OPAQUE(mm::PredicateList<mm::Fluent>);
PYBIND11_MAKE_OPAQUE(mm::PredicateList<mm::Derived>);
PYBIND11_MAKE_OPAQUE(mm::ToPredicateMap<std::string, mm::Static>);
PYBIND11_MAKE_OPAQUE(mm::ToPredicateMap<std::string, mm::Fluent>);
PYBIND11_MAKE_OPAQUE(mm::ToPredicateMap<std::string, mm::Derived>);
PYBIND11_MAKE_OPAQUE(mm::ProblemList);
PYBIND11_MAKE_OPAQUE(mm::VariableList);
PYBIND11_MAKE_OPAQUE(mm::TermList);
// Search
PYBIND11_MAKE_OPAQUE(mm::StateList);

/**
 * Type casters
 */

template<>
struct py::detail::type_caster<mm::FlatIndexList>
{
public:
    PYBIND11_TYPE_CASTER(mm::FlatIndexList, _("FlatIndexList"));

    // Python -> C++
    bool load(py::handle src, bool)
    {
        if (!py::isinstance<py::sequence>(src))
            return false;

        auto seq = py::reinterpret_borrow<py::sequence>(src);
        value.clear();
        // value.reserve(seq.size());
        for (const auto& item : seq)
        {
            value.push_back(py::cast<mm::Index>(item));
        }
        return true;
    }

    // C++ -> Python
    static py::handle cast(const mm::FlatIndexList& cpp_list, py::return_value_policy, py::handle)
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
struct py::detail::type_caster<mm::FlatDoubleList>
{
public:
    PYBIND11_TYPE_CASTER(mm::FlatDoubleList, _("FlatDoubleList"));

    // Python -> C++
    bool load(py::handle src, bool convert)
    {
        if (!py::isinstance<py::sequence>(src))
            return false;

        auto seq = py::reinterpret_borrow<py::sequence>(src);
        value.clear();
        value.reserve(seq.size());
        for (const auto& item : seq)
        {
            value.push_back(py::cast<double>(item));
        }
        return true;
    }

    // C++ -> Python
    static py::handle cast(const mm::FlatDoubleList& cpp_list, py::return_value_policy, py::handle)
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
struct py::detail::type_caster<mm::FlatBitset>
{
public:
    PYBIND11_TYPE_CASTER(mm::FlatBitset, _("FlatBitset"));

    // Python -> C++
    bool load(py::handle src, bool)
    {
        if (!py::isinstance<py::sequence>(src))
            return false;

        auto seq = py::reinterpret_borrow<py::sequence>(src);
        value.unset_all();
        for (const auto& item : seq)
        {
            value.set(py::cast<bool>(item));
        }
        return true;
    }

    // C++ -> Python
    static py::handle cast(const mm::FlatBitset& cpp_bitset, py::return_value_policy, py::handle)
    {
        py::list py_list;
        for (const auto& cpp_item : cpp_bitset)
        {
            py_list.append(py::cast(cpp_item));
        }
        return py_list.release();
    }
};

/**
 * Constness
 */

template<typename T>
struct PyImmutable
{
    explicit PyImmutable(const T& obj) : obj_(obj) {}

    const T& obj_;  // Read-only reference
};

/**
 * init - declarations:
 */

void init_formalism(py::module_& m);
void init_graphs(py::module_& m);

void init_search(py::module_& m);

void init_datasets(py::module_& m);

#endif
