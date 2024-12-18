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
    if (!m_state_builder.has_value())
    {
        m_state_builder = StateImpl();
    }

    return m_state_builder.value();
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

FlatBitset& StateRepositoryWorkspace::get_or_create_new_derived_atoms()
{
    if (!new_derived_atoms.has_value())
    {
        new_derived_atoms = FlatBitset();
    }

    return new_derived_atoms.value();
}

AxiomEvaluatorWorkspace& StateRepositoryWorkspace::get_or_create_axiom_evaluator_workspace()
{
    if (!m_axiom_evaluator_workspace.has_value())
    {
        m_axiom_evaluator_workspace = AxiomEvaluatorWorkspace();
    }

    return m_axiom_evaluator_workspace.value();
}

}
