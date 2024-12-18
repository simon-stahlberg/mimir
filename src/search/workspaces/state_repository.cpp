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

#include "mimir/search/workspaces/state_repository.hpp"

namespace mimir
{
/**
 * StateRepositoryWorkspace
 */

StateImpl& StateRepositoryWorkspace::get_or_create_state_builder()
{
    if (!state_builder.has_value())
    {
        state_builder = StateImpl();
    }

    return state_builder.value();
}

DenseState& StateRepositoryWorkspace::get_or_create_dense_state()
{
    if (!dense_state.has_value())
    {
        dense_state = DenseState();
    }

    return dense_state.value();
}

FlatBitset& StateRepositoryWorkspace::get_or_create_applied_positive_effect_atoms()
{
    if (!applied_positive_effect_atoms.has_value())
    {
        applied_positive_effect_atoms = FlatBitset();
    }

    return applied_positive_effect_atoms.value();
}

FlatBitset& StateRepositoryWorkspace::get_or_create_applied_negative_effect_atoms()
{
    if (!applied_negative_effect_atoms.has_value())
    {
        applied_negative_effect_atoms = FlatBitset();
    }

    return applied_negative_effect_atoms.value();
}

FlatIndexList& StateRepositoryWorkspace::get_or_create_new_derived_atoms_list()
{
    if (!new_derived_atoms_list.has_value())
    {
        new_derived_atoms_list = FlatIndexList();
    }

    return new_derived_atoms_list.value();
}

AxiomEvaluatorWorkspace& StateRepositoryWorkspace::get_or_create_axiom_evaluator_workspace()
{
    if (!axiom_evaluator_workspace.has_value())
    {
        axiom_evaluator_workspace = AxiomEvaluatorWorkspace();
    }

    return axiom_evaluator_workspace.value();
}

}
