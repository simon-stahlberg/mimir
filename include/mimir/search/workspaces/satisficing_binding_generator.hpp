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

#ifndef MIMIR_SEARCH_WORKSPACES_SATISFICING_BINDING_GENERATOR_HPP_
#define MIMIR_SEARCH_WORKSPACES_SATISFICING_BINDING_GENERATOR_HPP_

#include "mimir/algorithms/kpkc.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/dense_state.hpp"
#include "mimir/search/satisficing_binding_generator/consistency_graph.hpp"
#include "mimir/search/workspaces/assignment_set.hpp"

#include <boost/dynamic_bitset/dynamic_bitset.hpp>

namespace mimir
{

/// @brief `SatisficingBindingGeneratorWorkspace` encapsulates internally used memory.
///
/// Users need to ensure that it is repeatedly used in the same context.
/// The context is determined by the member function or free function that accepts the workspace.
class SatisficingBindingGeneratorWorkspace
{
private:
    friend class SatisficingBindingGenerator;

    std::vector<boost::dynamic_bitset<>>& get_or_create_full_consistency_graph(const consistency_graph::StaticConsistencyGraph& static_consistency_graph);
    KPKCWorkspace& get_or_create_kpkc_workspace(const consistency_graph::StaticConsistencyGraph& static_consistency_graph);
    DenseState& get_or_create_dense_state();
    AssignmentSetWorkspace& get_or_create_assignment_set_workspace();

    std::optional<std::vector<boost::dynamic_bitset<>>> full_consistency_graph = std::nullopt;
    std::optional<KPKCWorkspace> kpkc_workspace = std::nullopt;
    std::optional<DenseState> dense_state = std::nullopt;
    std::optional<AssignmentSetWorkspace> assignment_set_workspace = std::nullopt;
};

}

#endif
