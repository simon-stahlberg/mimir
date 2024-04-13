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

#ifndef MIMIR_FORMALISM_PARSER_NUMERIC_FLUENT_HPP_
#define MIMIR_FORMALISM_PARSER_NUMERIC_FLUENT_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/numeric_fluent.hpp"

namespace mimir
{
extern NumericFluent parse(loki::NumericFluent numeric_fluent, PDDLFactories& factories);
extern NumericFluentList parse(loki::NumericFluentList numeric_fluent_list, PDDLFactories& factories);
}

#endif
