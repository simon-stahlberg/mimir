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

#include "mimir/formalism/utils.hpp"

#include "mimir/formalism/ground_literal.hpp"

namespace mimir
{

template<PredicateTag P>
void to_ground_atoms(const GroundLiteralList<P>& literals, GroundAtomList<P>& out_ground_atoms)
{
    out_ground_atoms.clear();

    for (const auto& literal : literals)
    {
        out_ground_atoms.push_back(literal->get_atom());
    }
}

template void to_ground_atoms(const GroundLiteralList<Static>& literals, GroundAtomList<Static>& out_ground_atoms);
template void to_ground_atoms(const GroundLiteralList<Fluent>& literals, GroundAtomList<Fluent>& out_ground_atoms);
template void to_ground_atoms(const GroundLiteralList<Derived>& literals, GroundAtomList<Derived>& out_ground_atoms);

template<PredicateTag P>
GroundAtomList<P> to_ground_atoms(const GroundLiteralList<P>& literals)
{
    GroundAtomList<P> ground_atoms;
    to_ground_atoms(literals, ground_atoms);
    return ground_atoms;
}

template GroundAtomList<Static> to_ground_atoms(const GroundLiteralList<Static>& literals);
template GroundAtomList<Fluent> to_ground_atoms(const GroundLiteralList<Fluent>& literals);
template GroundAtomList<Derived> to_ground_atoms(const GroundLiteralList<Derived>& literals);
}