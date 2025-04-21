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

namespace nb = nanobind;
using namespace nb::literals;

namespace mimir
{

NB_MODULE(pymimir, m)
{
    // Create submodules before binding to avoid missing bindings
    auto advanced = m.def_submodule("advanced");
    m.attr("advanced") = advanced;

    auto common = advanced.def_submodule("common");
    advanced.attr("common") = common;
    auto formalism = advanced.def_submodule("formalism");
    advanced.attr("formalism") = formalism;
    auto graphs = advanced.def_submodule("graphs");
    advanced.attr("graphs") = graphs;
    auto search = advanced.def_submodule("search");
    advanced.attr("search") = search;
    auto datasets = advanced.def_submodule("datasets");
    advanced.attr("datasets") = datasets;
    auto languages = advanced.def_submodule("languages");
    advanced.attr("languages") = languages;
    auto description_logics = languages.def_submodule("description_logics");
    languages.attr("description_logics") = description_logics;
    auto general_policies = languages.def_submodule("general_policies");
    languages.attr("general_policies") = general_policies;

    common::bind_module_definitions(common);

    formalism::bind_module_definitions(formalism);

    graphs::bind_module_definitions(graphs);

    search::bind_module_definitions(search);

    datasets::bind_module_definitions(datasets);

    languages::dl::bind_module_definitions(description_logics);
    languages::general_policies::bind_module_definitions(general_policies);
}

}
