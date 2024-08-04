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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_FORMALISM_HPP_
#define MIMIR_FORMALISM_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir
{

/// @brief Translates a ground literal list into a list of ground atoms.
template<PredicateCategory P>
extern void to_ground_atoms(const GroundLiteralList<P>& literals, GroundAtomList<P>& out_ground_atoms);

/// @brief Translates a ground literal list into a list of ground atoms.
template<PredicateCategory P>
extern GroundAtomList<P> to_ground_atoms(const GroundLiteralList<P>& literals);

}

#endif