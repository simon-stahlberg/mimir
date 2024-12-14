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

#include "mimir/search/axiom_evaluators/workspaces.hpp"

#include "mimir/formalism/repositories.hpp"
#include "mimir/search/state.hpp"

using namespace std::string_literals;

namespace mimir
{

/**
 * LiftedAxiomEvaluatorWorkspace
 */

GroundAtomList<Fluent>& LiftedAxiomEvaluatorWorkspace::get_or_create_fluent_atoms(State state, PDDLRepositories& pddl_repositories)
{
    pddl_repositories.get_ground_atoms_from_indices<Fluent>(state->get_atoms<Fluent>(), fluent_atoms);

    return fluent_atoms;
}

GroundAtomList<Derived>& LiftedAxiomEvaluatorWorkspace::get_or_create_derived_atoms(State state, PDDLRepositories& pddl_repositories)
{
    pddl_repositories.get_ground_atoms_from_indices<Derived>(state->get_atoms<Derived>(), derived_atoms);

    return derived_atoms;
}

AssignmentSet<Fluent>& LiftedAxiomEvaluatorWorkspace::get_or_create_fluent_assignment_set(Problem problem)
{
    if (!fluent_assignment_set.has_value())
    {
        fluent_assignment_set = AssignmentSet<Fluent>(problem->get_objects().size(), problem->get_domain()->get_predicates<Fluent>());
    }

    return fluent_assignment_set.value();
}

AssignmentSet<Derived>& LiftedAxiomEvaluatorWorkspace::get_or_create_derived_assignment_set(Problem problem)
{
    if (!derived_assignment_set.has_value())
    {
        derived_assignment_set = AssignmentSet<Derived>(problem->get_objects().size(), problem->get_domain()->get_predicates<Derived>());
    }

    return derived_assignment_set.value();
}

/**
 * AxiomEvaluatorWorkspace
 */

LiftedAxiomEvaluatorWorkspace& AxiomEvaluatorWorkspace::get_or_create_lifted_workspace(Problem problem)
{
    if (!lifted_workspace.has_value())
    {
        lifted_workspace = LiftedAxiomEvaluatorWorkspace();
    }

    return lifted_workspace.value();
}

}
