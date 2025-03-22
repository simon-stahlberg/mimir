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

#ifndef MIMIR_PYTHON_INIT_DECLARATIONS_HPP
#define MIMIR_PYTHON_INIT_DECLARATIONS_HPP

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
NB_MAKE_OPAQUE(mm::formalism::ActionList);
NB_MAKE_OPAQUE(mm::formalism::AtomList<mm::formalism::StaticTag>);
NB_MAKE_OPAQUE(mm::formalism::AtomList<mm::formalism::FluentTag>);
NB_MAKE_OPAQUE(mm::formalism::AtomList<mm::formalism::DerivedTag>);
NB_MAKE_OPAQUE(mm::formalism::AxiomList);
NB_MAKE_OPAQUE(mm::formalism::ConditionalEffectList);
NB_MAKE_OPAQUE(mm::formalism::FunctionExpressionList);
NB_MAKE_OPAQUE(mm::formalism::FunctionSkeletonList<mm::formalism::StaticTag>);
NB_MAKE_OPAQUE(mm::formalism::FunctionSkeletonList<mm::formalism::FluentTag>);
NB_MAKE_OPAQUE(mm::formalism::FunctionSkeletonList<mm::formalism::AuxiliaryTag>);
NB_MAKE_OPAQUE(mm::formalism::FunctionList<mm::formalism::StaticTag>);
NB_MAKE_OPAQUE(mm::formalism::FunctionList<mm::formalism::FluentTag>);
NB_MAKE_OPAQUE(mm::formalism::FunctionList<mm::formalism::AuxiliaryTag>);
NB_MAKE_OPAQUE(mm::formalism::GroundAtomList<mm::formalism::StaticTag>);
NB_MAKE_OPAQUE(mm::formalism::GroundAtomList<mm::formalism::FluentTag>);
NB_MAKE_OPAQUE(mm::formalism::GroundAtomList<mm::formalism::DerivedTag>);
NB_MAKE_OPAQUE(mm::formalism::GroundFunctionExpressionList);
NB_MAKE_OPAQUE(mm::formalism::GroundLiteralList<mm::formalism::StaticTag>);
NB_MAKE_OPAQUE(mm::formalism::GroundLiteralList<mm::formalism::FluentTag>);
NB_MAKE_OPAQUE(mm::formalism::GroundLiteralList<mm::formalism::DerivedTag>);
NB_MAKE_OPAQUE(mm::formalism::GroundActionList);
NB_MAKE_OPAQUE(mm::formalism::GroundAxiomList);
NB_MAKE_OPAQUE(mm::formalism::LiteralList<mm::formalism::StaticTag>);
NB_MAKE_OPAQUE(mm::formalism::LiteralList<mm::formalism::FluentTag>);
NB_MAKE_OPAQUE(mm::formalism::LiteralList<mm::formalism::DerivedTag>);
NB_MAKE_OPAQUE(mm::formalism::GroundFunctionValueList<mm::formalism::StaticTag>);
NB_MAKE_OPAQUE(mm::formalism::GroundFunctionValueList<mm::formalism::FluentTag>);
NB_MAKE_OPAQUE(mm::formalism::GroundFunctionValueList<mm::formalism::AuxiliaryTag>);
NB_MAKE_OPAQUE(mm::formalism::GroundFunctionList<mm::formalism::StaticTag>);
NB_MAKE_OPAQUE(mm::formalism::GroundFunctionList<mm::formalism::FluentTag>);
NB_MAKE_OPAQUE(mm::formalism::GroundFunctionList<mm::formalism::AuxiliaryTag>);
NB_MAKE_OPAQUE(mm::formalism::NumericConstraintList);
NB_MAKE_OPAQUE(mm::formalism::ObjectList);
NB_MAKE_OPAQUE(mm::formalism::PredicateList<mm::formalism::StaticTag>);
NB_MAKE_OPAQUE(mm::formalism::PredicateList<mm::formalism::FluentTag>);
NB_MAKE_OPAQUE(mm::formalism::PredicateList<mm::formalism::DerivedTag>);
NB_MAKE_OPAQUE(mm::formalism::ToPredicateMap<std::string, mm::formalism::StaticTag>);
NB_MAKE_OPAQUE(mm::formalism::ToPredicateMap<std::string, mm::formalism::FluentTag>);
NB_MAKE_OPAQUE(mm::formalism::ToPredicateMap<std::string, mm::formalism::DerivedTag>);
NB_MAKE_OPAQUE(mm::formalism::ProblemList);
NB_MAKE_OPAQUE(mm::formalism::VariableList);
NB_MAKE_OPAQUE(mm::formalism::TermList);
// Search
NB_MAKE_OPAQUE(mm::search::StateList);
// Languages
NB_MAKE_OPAQUE(mm::languages::dl::ConstructorList<mm::languages::dl::ConceptTag>);
NB_MAKE_OPAQUE(mm::languages::dl::ConstructorList<mm::languages::dl::RoleTag>);
NB_MAKE_OPAQUE(mm::languages::dl::ConstructorList<mm::languages::dl::BooleanTag>);
NB_MAKE_OPAQUE(mm::languages::dl::ConstructorList<mm::languages::dl::NumericalTag>);

namespace mimir::bindings
{

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

extern void bind_common(nb::module_& m);

extern void bind_formalism(nb::module_& m);
extern void bind_graphs(nb::module_& m);

extern void bind_search(nb::module_& m);

extern void bind_datasets(nb::module_& m);

extern void bind_languages_description_logics(nb::module_& m);

}

#endif
