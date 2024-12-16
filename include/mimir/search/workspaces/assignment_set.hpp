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

#ifndef MIMIR_SEARCH_WORKSPACES_ASSIGNMENT_SET_HPP_
#define MIMIR_SEARCH_WORKSPACES_ASSIGNMENT_SET_HPP_

#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/assignment_set.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir
{

/// @brief `AssignmentSetWorkspace` encapsulates internally used memory.
///
/// Users need to ensure that it is repeatedly used in the same context.
/// The context is determined by the member function or free function that accepts the workspace.
class AssignmentSetWorkspace
{
private:
    friend class LiftedAxiomEvaluator;
    friend class LiftedApplicableActionGenerator;
    friend class SatisficingBindingGenerator;

    GroundAtomList<Fluent>& get_or_create_fluent_atoms(const FlatBitset& fluent_atom_indices, PDDLRepositories& pddl_repositories);
    GroundAtomList<Derived>& get_or_create_derived_atoms(const FlatBitset& derived_atom_indices, PDDLRepositories& pddl_repositories);
    AssignmentSet<Fluent>& get_or_create_fluent_assignment_set(Problem problem);
    AssignmentSet<Derived>& get_or_create_derived_assignment_set(Problem problem);

    GroundAtomList<Fluent> fluent_atoms = GroundAtomList<Fluent>();
    GroundAtomList<Derived> derived_atoms = GroundAtomList<Derived>();
    std::optional<AssignmentSet<Fluent>> fluent_assignment_set = std::nullopt;
    std::optional<AssignmentSet<Derived>> derived_assignment_set = std::nullopt;
};

}

#endif
