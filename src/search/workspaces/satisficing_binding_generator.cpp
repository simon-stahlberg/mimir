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

#include "mimir/search/workspaces/satisficing_binding_generator.hpp"

namespace mimir
{

std::vector<boost::dynamic_bitset<>>&
SatisficingBindingGeneratorWorkspace::get_or_create_full_consistency_graph(const consistency_graph::StaticConsistencyGraph& static_consistency_graph)
{
    if (!full_consistency_graph.has_value())
    {
        full_consistency_graph = std::vector<boost::dynamic_bitset<>>(static_consistency_graph.get_vertices().size(),
                                                                      boost::dynamic_bitset<>(static_consistency_graph.get_vertices().size()));
    }

    return full_consistency_graph.value();
}

KPKCWorkspace& SatisficingBindingGeneratorWorkspace::get_or_create_kpkc_workspace(const consistency_graph::StaticConsistencyGraph& static_consistency_graph)
{
    if (!kpkc_workspace.has_value())
    {
        kpkc_workspace = KPKCWorkspace(static_consistency_graph.get_vertices_by_parameter_index());
    }

    return kpkc_workspace.value();
}

DenseState& SatisficingBindingGeneratorWorkspace::get_or_create_dense_state()
{
    if (!dense_state.has_value())
    {
        dense_state = DenseState();
    }

    return dense_state.value();
}

AssignmentSetWorkspace& SatisficingBindingGeneratorWorkspace::get_or_create_assignment_set_workspace()
{
    if (!assignment_set_workspace.has_value())
    {
        assignment_set_workspace = AssignmentSetWorkspace();
    }

    return assignment_set_workspace.value();
}

}
