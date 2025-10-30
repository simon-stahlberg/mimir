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

#include "mimir/datasets/tuple_graph.hpp"

#include "mimir/datasets/state_space.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/search_context.hpp"
#include "mimir/search/state.hpp"
#include "tuple_graph_factory.hpp"

using namespace mimir::formalism;
using namespace mimir::search;

namespace mimir::datasets
{
TupleGraphImpl::TupleGraphImpl(StateSpace state_space,
                               graphs::InternalTupleGraph graph,
                               IndexGroupedVector<const Index> vertices_grouped_by_distance,
                               IndexGroupedVector<const Index> problem_vertices_grouped_by_distance) :
    m_state_space(state_space),
    m_graph(std::move(graph)),
    m_v_idxs_grouped_by_distance(std::move(vertices_grouped_by_distance)),
    m_problem_v_idxs_grouped_by_distance(std::move(problem_vertices_grouped_by_distance))
{
}

const StateSpace& TupleGraphImpl::get_state_space() const { return m_state_space; }

const graphs::InternalTupleGraph& TupleGraphImpl::get_graph() const { return m_graph; }

const IndexGroupedVector<const Index>& TupleGraphImpl::get_tuple_vertex_indices_grouped_by_distance() const { return m_v_idxs_grouped_by_distance; }

const IndexGroupedVector<const Index>& TupleGraphImpl::get_problem_vertex_indices_grouped_by_distance() const { return m_problem_v_idxs_grouped_by_distance; }

TupleGraphList TupleGraphImpl::create(StateSpace state_space, std::optional<CertificateMaps>& certificate_maps, const Options& options)
{
    auto tuple_graphs = TupleGraphList {};

    for (const auto& v : state_space->get_graph().get_vertices())
    {
        tuple_graphs.push_back(create_tuple_graph(v, state_space, certificate_maps, options));
    }

    return tuple_graphs;
}

}
