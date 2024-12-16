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

#ifndef MIMIR_SEARCH_WORKSPACES_APPLICABLE_ACTION_GENERATOR_HPP_
#define MIMIR_SEARCH_WORKSPACES_APPLICABLE_ACTION_GENERATOR_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/workspaces/grounded_applicable_action_generator.hpp"
#include "mimir/search/workspaces/lifted_applicable_action_generator.hpp"

namespace mimir
{

/// @brief `ApplicableActionGeneratorWorkspace` encapsulates internally used memory.
///
/// Users need to ensure that it is repeatedly used in the same context.
/// The context is determined by the member function or free function that accepts the workspace.
class ApplicableActionGeneratorWorkspace
{
private:
    friend class GroundedApplicableActionGenerator;
    friend class LiftedApplicableActionGenerator;

    std::optional<LiftedApplicableActionGeneratorWorkspace> lifted_workspace = std::nullopt;
    std::optional<GroundedApplicableActionGeneratorWorkspace> grounded_workspace = std::nullopt;

    LiftedApplicableActionGeneratorWorkspace& get_or_create_lifted_workspace();
    GroundedApplicableActionGeneratorWorkspace& get_or_create_grounded_workspace();
};

}

#endif
