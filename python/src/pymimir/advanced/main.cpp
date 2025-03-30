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

#include "init_declarations.hpp"

#include <nanobind/nanobind.h>

namespace nb = nanobind;
using namespace nb::literals;

namespace mimir::bindings
{

NB_MODULE(_pymimir, m)
{
    // Create submodules before binding to avoid missing bindings
    auto advanced = m.def_submodule("advanced");

    auto common = advanced.def_submodule("common");
    auto formalism = advanced.def_submodule("formalism");
    auto graphs = advanced.def_submodule("graphs");
    auto search = advanced.def_submodule("search");
    auto datasets = advanced.def_submodule("datasets");
    auto languages = advanced.def_submodule("languages");
    auto description_logics = languages.def_submodule("description_logics");

    bind_common(common);

    bind_formalism(formalism);

    bind_graphs(graphs);

    bind_search(search);

    bind_datasets(datasets);

    bind_languages_description_logics(description_logics);
}

}
