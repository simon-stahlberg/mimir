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

#include "atom.hpp"

#include "predicate.hpp"
#include "term.hpp"

#include <mimir/formalism/factories.hpp>

namespace mimir
{
Atom parse(loki::pddl::Atom atom, PDDLFactories& factories)
{
    return factories.atoms.get_or_create<AtomImpl>(parse(atom->get_predicate(), factories), parse(atom->get_terms(), factories));
}

AtomList parse(loki::pddl::AtomList atom_list, PDDLFactories& factories)
{
    auto result_atom_list = AtomList();
    for (const auto& atom : atom_list)
    {
        result_atom_list.push_back(parse(atom, factories));
    }
    return result_atom_list;
}
}
