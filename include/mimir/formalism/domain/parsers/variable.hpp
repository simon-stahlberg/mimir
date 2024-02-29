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

#ifndef MIMIR_FORMALISM_DOMAIN_PARSER_VARIABLE_HPP_
#define MIMIR_FORMALISM_DOMAIN_PARSER_VARIABLE_HPP_

#include "../../common/types.hpp"
#include "../declarations.hpp"
#include "../variable.hpp"

namespace mimir
{
extern Variable parse(loki::pddl::Variable variable, PDDLFactories& factories);
}

#endif
