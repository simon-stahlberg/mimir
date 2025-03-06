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

#include <iostream>
#include <iterator>
#include <mimir/mimir.hpp>
#include <nanobind/nanobind.h>
#include <nanobind/stl/bind_map.h>     ///< TODO: implement our own with PyImmutable
#include <nanobind/stl/bind_vector.h>  ///< TODO: implement our own with PyImmutable
#include <nanobind/stl/map.h>
#include <nanobind/stl/optional.h>  ///< for optional support
#include <nanobind/stl/optional.h>
#include <nanobind/stl/shared_ptr.h>  ///< for shared ownerships
#include <nanobind/stl/string.h>      ///< for string support
#include <nanobind/stl/unordered_map.h>
#include <nanobind/stl/unordered_set.h>
#include <nanobind/stl/vector.h>

namespace nb = nanobind;
using namespace nb::literals;
namespace mm = mimir;

/**
 * Opague types are exceptions to auto stl bindings
 */

// Formalism
NB_MAKE_OPAQUE(mm::ActionList);
NB_MAKE_OPAQUE(mm::AtomList<mm::Static>);
NB_MAKE_OPAQUE(mm::AtomList<mm::Fluent>);
NB_MAKE_OPAQUE(mm::AtomList<mm::Derived>);
NB_MAKE_OPAQUE(mm::AxiomList);
NB_MAKE_OPAQUE(mm::ConditionalEffectList);
NB_MAKE_OPAQUE(mm::FunctionExpressionList);
NB_MAKE_OPAQUE(mm::FunctionSkeletonList<mm::Static>);
NB_MAKE_OPAQUE(mm::FunctionSkeletonList<mm::Fluent>);
NB_MAKE_OPAQUE(mm::FunctionSkeletonList<mm::Auxiliary>);
NB_MAKE_OPAQUE(mm::FunctionList<mm::Static>);
NB_MAKE_OPAQUE(mm::FunctionList<mm::Fluent>);
NB_MAKE_OPAQUE(mm::FunctionList<mm::Auxiliary>);
NB_MAKE_OPAQUE(mm::GroundAtomList<mm::Static>);
NB_MAKE_OPAQUE(mm::GroundAtomList<mm::Fluent>);
NB_MAKE_OPAQUE(mm::GroundAtomList<mm::Derived>);
NB_MAKE_OPAQUE(mm::GroundFunctionExpressionList);
NB_MAKE_OPAQUE(mm::GroundLiteralList<mm::Static>);
NB_MAKE_OPAQUE(mm::GroundLiteralList<mm::Fluent>);
NB_MAKE_OPAQUE(mm::GroundLiteralList<mm::Derived>);
NB_MAKE_OPAQUE(mm::GroundActionList);
NB_MAKE_OPAQUE(mm::GroundAxiomList);
NB_MAKE_OPAQUE(mm::LiteralList<mm::Static>);
NB_MAKE_OPAQUE(mm::LiteralList<mm::Fluent>);
NB_MAKE_OPAQUE(mm::LiteralList<mm::Derived>);
NB_MAKE_OPAQUE(mm::GroundFunctionValueList<mm::Static>);
NB_MAKE_OPAQUE(mm::GroundFunctionValueList<mm::Fluent>);
NB_MAKE_OPAQUE(mm::GroundFunctionValueList<mm::Auxiliary>);
NB_MAKE_OPAQUE(mm::GroundFunctionList<mm::Static>);
NB_MAKE_OPAQUE(mm::GroundFunctionList<mm::Fluent>);
NB_MAKE_OPAQUE(mm::GroundFunctionList<mm::Auxiliary>);
NB_MAKE_OPAQUE(mm::NumericConstraintList);
NB_MAKE_OPAQUE(mm::ObjectList);
NB_MAKE_OPAQUE(mm::PredicateList<mm::Static>);
NB_MAKE_OPAQUE(mm::PredicateList<mm::Fluent>);
NB_MAKE_OPAQUE(mm::PredicateList<mm::Derived>);
NB_MAKE_OPAQUE(mm::ToPredicateMap<std::string, mm::Static>);
NB_MAKE_OPAQUE(mm::ToPredicateMap<std::string, mm::Fluent>);
NB_MAKE_OPAQUE(mm::ToPredicateMap<std::string, mm::Derived>);
NB_MAKE_OPAQUE(mm::ProblemList);
NB_MAKE_OPAQUE(mm::VariableList);
NB_MAKE_OPAQUE(mm::TermList);
// Search
NB_MAKE_OPAQUE(mm::StateList);

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

void bind_common(nb::module_& m);

void bind_formalism(nb::module_& m);
void bind_graphs(nb::module_& m);

void bind_search(nb::module_& m);

void bind_datasets(nb::module_& m);

#endif
