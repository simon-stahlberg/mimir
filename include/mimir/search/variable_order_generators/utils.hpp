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

#ifndef MIMIR_SEARCH_VARIABLE_ORDER_GENERATORS_UTILS_HPP_
#define MIMIR_SEARCH_VARIABLE_ORDER_GENERATORS_UTILS_HPP_

#include "mimir/search/variable_order_generators/types.hpp"

namespace mimir::vog
{

/**
 * The default order prioritizes elements in the order that appear most frequently.
 * To obtain deterministic results, we break ties in favor of their string representations.
 * Unfortunately, we do not have a canonical string representation for function expressions.
 */

extern VariableOrder get_default_order(const GroundActionList& actions);

extern VariableOrder get_default_order(const GroundAxiomList& axioms);
}

#endif