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

#ifndef MIMIR_SEARCH_WORKSPACES_GROUNDED_APPLICABLE_ACTION_GENERATOR_HPP_
#define MIMIR_SEARCH_WORKSPACES_GROUNDED_APPLICABLE_ACTION_GENERATOR_HPP_

#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/dense_state.hpp"

namespace mimir
{

/// @brief `GroundedApplicableActionGeneratorWorkspace` encapsulates internally used memory.
///
/// Users need to ensure that it is repeatedly used in the same context.
/// The context is determined by the member function or free function that accepts the workspace.
class GroundedApplicableActionGeneratorWorkspace
{
private:
    friend class GroundedApplicableActionGenerator;

    DenseState& get_or_create_dense_state();

    std::optional<DenseState> dense_state = std::nullopt;
};

}

#endif