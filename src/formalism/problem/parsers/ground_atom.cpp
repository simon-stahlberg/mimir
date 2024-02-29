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

#include <mimir/formalism/domain/parsers/object.hpp>
#include <mimir/formalism/domain/parsers/predicate.hpp>
#include <mimir/formalism/problem/parsers/ground_atom.hpp>

namespace mimir
{
GroundAtom parse(loki::pddl::GroundAtom ground_atom, PDDLFactories& factories)
{
    return factories.ground_atoms.get_or_create<GroundAtomImpl>(parse(ground_atom->get_predicate(), factories), parse(ground_atom->get_objects(), factories));
}

GroundAtomList parse(loki::pddl::GroundAtomList ground_atom_list, PDDLFactories& factories)
{
    auto result_ground_atom_list = GroundAtomList();
    for (const auto& ground_atom : ground_atom_list)
    {
        result_ground_atom_list.push_back(parse(ground_atom, factories));
    }
    return result_ground_atom_list;
}
}
