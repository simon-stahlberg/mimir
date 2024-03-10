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

#include "parameter.hpp"

#include "type.hpp"
#include "variable.hpp"

#include <mimir/formalism/factories.hpp>

namespace mimir
{
Parameter parse(loki::pddl::Parameter parameter, PDDLFactories& factories)
{
    return factories.parameters.get_or_create<ParameterImpl>(parse(parameter->get_variable(), factories), parse(parameter->get_bases(), factories));
}

ParameterList parse(loki::pddl::ParameterList parameter_list, PDDLFactories& factories)
{
    auto result_parameter_list = ParameterList();
    for (const auto& parameter : parameter_list)
    {
        result_parameter_list.push_back(parse(parameter, factories));
    }
    return result_parameter_list;
}
}
