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

#include "mimir/search/assignment_set_utils.hpp"

#include "mimir/formalism/problem.hpp"
#include "mimir/search/state_unpacked.hpp"

namespace mimir::search
{
void initialize(const UnpackedStateImpl& unpacked_state, formalism::DynamicAssignmentSets& out_dynamic_assignment_sets)
{
    const auto& pddl_repositories = unpacked_state.get_problem().get_repositories();
    const auto& dense_fluent_atoms = unpacked_state.get_atoms<formalism::FluentTag>();
    const auto& dense_derived_atoms = unpacked_state.get_atoms<formalism::DerivedTag>();
    const auto& dense_numeric_variables = unpacked_state.get_numeric_variables();

    auto& fluent_predicate_assignment_set = out_dynamic_assignment_sets.fluent_predicate_assignment_sets;
    auto& fluent_atoms = fluent_predicate_assignment_set.get_atoms_scratch();
    pddl_repositories.get_ground_atoms_from_indices(dense_fluent_atoms, fluent_atoms);
    fluent_predicate_assignment_set.reset();
    fluent_predicate_assignment_set.insert_ground_atoms(fluent_atoms);

    auto& derived_predicate_assignment_set = out_dynamic_assignment_sets.derived_predicate_assignment_sets;
    auto& derived_atoms = derived_predicate_assignment_set.get_atoms_scratch();
    pddl_repositories.get_ground_atoms_from_indices(dense_derived_atoms, derived_atoms);
    derived_predicate_assignment_set.reset();
    derived_predicate_assignment_set.insert_ground_atoms(derived_atoms);

    auto& fluent_function_assignment_set = out_dynamic_assignment_sets.fluent_function_skeleton_assignment_sets;
    auto& fluent_functions = fluent_function_assignment_set.get_functions_scratch();
    pddl_repositories.get_ground_functions(dense_numeric_variables.size(), fluent_functions);
    fluent_function_assignment_set.reset();
    fluent_function_assignment_set.insert_ground_function_values(fluent_functions, dense_numeric_variables);
}
}
