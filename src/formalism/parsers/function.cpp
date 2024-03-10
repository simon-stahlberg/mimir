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

#include "function.hpp"

#include "function_skeleton.hpp"
#include "term.hpp"

#include <mimir/formalism/factories.hpp>

namespace mimir
{
Function parse(loki::pddl::Function function, PDDLFactories& factories)
{
    return factories.functions.get_or_create<FunctionImpl>(parse(function->get_function_skeleton(), factories), parse(function->get_terms(), factories));
}

FunctionList parse(loki::pddl::FunctionList function_list, PDDLFactories& factories)
{
    auto result_function_list = FunctionList();
    for (const auto& function : function_list)
    {
        result_function_list.push_back(parse(function, factories));
    }
    return result_function_list;
}
}
