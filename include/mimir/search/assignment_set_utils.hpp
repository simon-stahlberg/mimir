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

#ifndef MIMIR_SEARCH_ASSIGNMENT_SET_UTILS_HPP_
#define MIMIR_SEARCH_ASSIGNMENT_SET_UTILS_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir::search
{
/// @brief Initializes the assignment sets in the dynamic consistency graph details with the given state.
/// @param unpacked_state
/// @param out_details
extern void initialize(const UnpackedStateImpl& unpacked_state, formalism::DynamicAssignmentSets& out_dynamic_assignment_sets);
}

#endif
