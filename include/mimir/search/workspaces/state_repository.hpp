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

#ifndef MIMIR_SEARCH_WORKSPACES_STATE_REPOSITORY_HPP_
#define MIMIR_SEARCH_WORKSPACES_STATE_REPOSITORY_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/workspaces/axiom_evaluator.hpp"

namespace mimir
{
/// @brief `StateRepositoryWorkspace` encapsulates internally used memory.
///
/// Users need to ensure that it is repeatedly used in the same context.
/// The context is determined by the member function or free function that accepts the workspace.
class StateRepositoryWorkspace
{
private:
    friend class StateRepository;
    friend class DeleteRelaxedProblemExplorator;

    StateImpl& get_or_create_state_builder();
    AxiomEvaluatorWorkspace& get_or_create_axiom_evaluator_workspace();

    std::optional<StateImpl> m_state_builder = std::nullopt;
    std::optional<AxiomEvaluatorWorkspace> m_axiom_evaluator_workspace = std::nullopt;
};

}

#endif
