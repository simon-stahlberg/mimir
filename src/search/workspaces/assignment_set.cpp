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

#include "mimir/search/workspaces/assignment_set.hpp"

#include "mimir/formalism/repositories.hpp"
#include "mimir/search/state.hpp"

namespace mimir
{

GroundAtomList<Fluent>& AssignmentSetWorkspace::get_or_create_fluent_atoms(const FlatBitset& fluent_atoms_indices, PDDLRepositories& pddl_repositories)
{
    pddl_repositories.get_ground_atoms_from_indices<Fluent>(fluent_atoms_indices, fluent_atoms);

    return fluent_atoms;
}

GroundAtomList<Derived>& AssignmentSetWorkspace::get_or_create_derived_atoms(const FlatBitset& derived_atom_indices, PDDLRepositories& pddl_repositories)
{
    pddl_repositories.get_ground_atoms_from_indices<Derived>(derived_atom_indices, derived_atoms);

    return derived_atoms;
}

AssignmentSet<Fluent>& AssignmentSetWorkspace::get_or_create_fluent_assignment_set(Problem problem)
{
    if (!fluent_assignment_set.has_value())
    {
        fluent_assignment_set = AssignmentSet<Fluent>(problem->get_objects().size(), problem->get_domain()->get_predicates<Fluent>());
    }

    return fluent_assignment_set.value();
}

AssignmentSet<Derived>& AssignmentSetWorkspace::get_or_create_derived_assignment_set(Problem problem)
{
    if (!derived_assignment_set.has_value())
    {
        derived_assignment_set = AssignmentSet<Derived>(problem->get_objects().size(), problem->get_problem_and_domain_derived_predicates());
    }

    return derived_assignment_set.value();
}

}
