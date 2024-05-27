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

#ifndef MIMIR_SEARCH_TRANSLATIONS_HPP_
#define MIMIR_SEARCH_TRANSLATIONS_HPP_

#include "mimir/formalism/formalism.hpp"

namespace mimir
{

/// @brief Translates a bitset into a list of ground atoms
template<PredicateCategory P>
void to_ground_atoms(const GroundLiteralList<P>& literals, GroundAtomList<P>& out_ground_atoms)
{
    out_ground_atoms.clear();

    for (const auto& literal : literals)
    {
        out_ground_atoms.push_back(literal->get_atom());
    }
}

/// @brief Translates a bitset into a list of ground atoms
template<PredicateCategory P>
GroundAtomList<P> to_ground_atoms(const GroundLiteralList<P>& literals)
{
    GroundAtomList<P> ground_atoms;
    to_ground_atoms(literals, ground_atoms);
    return ground_atoms;
}

}

#endif  // MIMIR_SEARCH_TRANSLATIONS_HPP_
