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

#ifndef MIMIR_SEARCH_WORKSPACES_LIFTED_AXIOM_EVALUATOR_HPP_
#define MIMIR_SEARCH_WORKSPACES_LIFTED_AXIOM_EVALUATOR_HPP_

#include "mimir/formalism/assignment_set.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/workspaces/assignment_set.hpp"
#include "mimir/search/workspaces/satisficing_binding_generator.hpp"

namespace mimir
{

/// @brief `LiftedAxiomEvaluatorWorkspace` encapsulates internally used memory.
///
/// Users need to ensure that it is repeatedly used in the same context.
/// The context is determined by the member function or free function that accepts the workspace.
class LiftedAxiomEvaluatorWorkspace
{
private:
    friend class LiftedAxiomEvaluator;

    AssignmentSetWorkspace& get_or_create_assignment_set_workspace();
    SatisficingBindingGeneratorWorkspace& get_or_create_satisficing_binding_generator(Axiom axiom);

    std::optional<AssignmentSetWorkspace> assignment_set_workspace = std::nullopt;
    std::unordered_map<Axiom, SatisficingBindingGeneratorWorkspace> satisficing_binding_generator_workspaces =
        std::unordered_map<Axiom, SatisficingBindingGeneratorWorkspace>();
};

}

#endif
