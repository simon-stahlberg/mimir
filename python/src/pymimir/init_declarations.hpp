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
#include <mimir/common/types_cista.hpp>
#include <mimir/mimir.hpp>
#include <nanobind/nanobind.h>
#include <nanobind/stl/bind_map.h>
#include <nanobind/stl/bind_vector.h>

namespace nb = nanobind;
using namespace nb::literals;
namespace mm = mimir;

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
