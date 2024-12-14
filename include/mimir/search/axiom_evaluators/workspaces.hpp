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

#ifndef MIMIR_SEARCH_AXIOM_EVALUATORS_WORKSPACES_HPP_
#define MIMIR_SEARCH_AXIOM_EVALUATORS_WORKSPACES_HPP_

#include "mimir/formalism/assignment_set.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir
{

/// @brief `LiftedAxiomEvaluatorWorkspace` encapsulates internally used memory.
class LiftedAxiomEvaluatorWorkspace
{
private:
    friend class LiftedAxiomEvaluator;

    GroundAtomList<Fluent>& get_or_create_fluent_atoms(State state, PDDLRepositories& pddl_repositories);
    GroundAtomList<Derived>& get_or_create_derived_atoms(State state, PDDLRepositories& pddl_repositories);
    AssignmentSet<Fluent>& get_or_create_fluent_assignment_set(Problem problem);
    AssignmentSet<Derived>& get_or_create_derived_assignment_set(Problem problem);

    GroundAtomList<Fluent> fluent_atoms = GroundAtomList<Fluent>();
    GroundAtomList<Derived> derived_atoms = GroundAtomList<Derived>();
    std::optional<AssignmentSet<Fluent>> fluent_assignment_set;
    std::optional<AssignmentSet<Derived>> derived_assignment_set;
};

/// @brief `AxiomEvaluatorWorkspace` encapsulates internally used memory.
class AxiomEvaluatorWorkspace
{
private:
    friend class GroundedAxiomEvaluator;
    friend class LiftedAxiomEvaluator;

    std::optional<LiftedAxiomEvaluatorWorkspace> lifted_workspace = std::nullopt;

    LiftedAxiomEvaluatorWorkspace& get_or_create_lifted_workspace(Problem problem);
};

}

#endif
