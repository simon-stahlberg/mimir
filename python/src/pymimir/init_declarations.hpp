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

#include <pybind11/attr.h>
#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // Necessary for automatic conversion of e.g. std::vectors
#include <pybind11/stl_bind.h>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

// Formalism
PYBIND11_MAKE_OPAQUE(mimir::ActionList);
PYBIND11_MAKE_OPAQUE(mimir::AtomList<mimir::Static>);
PYBIND11_MAKE_OPAQUE(mimir::AtomList<mimir::Fluent>);
PYBIND11_MAKE_OPAQUE(mimir::AtomList<mimir::Derived>);
PYBIND11_MAKE_OPAQUE(mimir::AxiomList);
PYBIND11_MAKE_OPAQUE(mimir::ConditionalEffectList);
PYBIND11_MAKE_OPAQUE(mimir::FunctionExpressionList);
PYBIND11_MAKE_OPAQUE(mimir::FunctionSkeletonList<mimir::Static>);
PYBIND11_MAKE_OPAQUE(mimir::FunctionSkeletonList<mimir::Fluent>);
PYBIND11_MAKE_OPAQUE(mimir::FunctionSkeletonList<mimir::Auxiliary>);
PYBIND11_MAKE_OPAQUE(mimir::FunctionList<mimir::Static>);
PYBIND11_MAKE_OPAQUE(mimir::FunctionList<mimir::Fluent>);
PYBIND11_MAKE_OPAQUE(mimir::FunctionList<mimir::Auxiliary>);
PYBIND11_MAKE_OPAQUE(mimir::GroundAtomList<mimir::Static>);
PYBIND11_MAKE_OPAQUE(mimir::GroundAtomList<mimir::Fluent>);
PYBIND11_MAKE_OPAQUE(mimir::GroundAtomList<mimir::Derived>);
PYBIND11_MAKE_OPAQUE(mimir::GroundFunctionExpressionList);
PYBIND11_MAKE_OPAQUE(mimir::GroundLiteralList<mimir::Static>);
PYBIND11_MAKE_OPAQUE(mimir::GroundLiteralList<mimir::Fluent>);
PYBIND11_MAKE_OPAQUE(mimir::GroundLiteralList<mimir::Derived>);
PYBIND11_MAKE_OPAQUE(mimir::GroundActionList);
PYBIND11_MAKE_OPAQUE(mimir::GroundAxiomList);
PYBIND11_MAKE_OPAQUE(mimir::LiteralList<mimir::Static>);
PYBIND11_MAKE_OPAQUE(mimir::LiteralList<mimir::Fluent>);
PYBIND11_MAKE_OPAQUE(mimir::LiteralList<mimir::Derived>);
PYBIND11_MAKE_OPAQUE(mimir::GroundFunctionValueList<mimir::Static>);
PYBIND11_MAKE_OPAQUE(mimir::GroundFunctionValueList<mimir::Fluent>);
PYBIND11_MAKE_OPAQUE(mimir::GroundFunctionValueList<mimir::Auxiliary>);
PYBIND11_MAKE_OPAQUE(mimir::ObjectList);
PYBIND11_MAKE_OPAQUE(mimir::PredicateList<mimir::Static>);
PYBIND11_MAKE_OPAQUE(mimir::PredicateList<mimir::Fluent>);
PYBIND11_MAKE_OPAQUE(mimir::PredicateList<mimir::Derived>);
PYBIND11_MAKE_OPAQUE(mimir::ToPredicateMap<std::string, mimir::Static>);
PYBIND11_MAKE_OPAQUE(mimir::ToPredicateMap<std::string, mimir::Fluent>);
PYBIND11_MAKE_OPAQUE(mimir::ToPredicateMap<std::string, mimir::Derived>);
PYBIND11_MAKE_OPAQUE(mimir::ProblemList);
PYBIND11_MAKE_OPAQUE(mimir::VariableList);
PYBIND11_MAKE_OPAQUE(mimir::TermList);
// Search
PYBIND11_MAKE_OPAQUE(mimir::StateList);

//
// init - declarations:
//
void init_formalism(pybind11::module_& m);
void init_graphs(pybind11::module_& m);
void init_search(pybind11::module_& m);

#endif  // MIMIR_INIT_DECLARATIONS_HPP
