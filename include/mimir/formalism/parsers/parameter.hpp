/*
 * Copyright (C) 2023 Dominik Drexler
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

#ifndef MIMIR_FORMALISM_PARSER_PARAMETER_HPP_
#define MIMIR_FORMALISM_PARSER_PARAMETER_HPP_

#include "../declarations.hpp"
#include "../parameter.hpp"

namespace mimir
{
extern Parameter parse(loki::pddl::Parameter parameter, PDDLFactories& factories);
extern ParameterList parse(loki::pddl::ParameterList parameter_list, PDDLFactories& factories);
}

#endif
