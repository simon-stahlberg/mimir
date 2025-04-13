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

const IndexGroupedVector<const Index>& TupleGraphImpl::get_vertices_grouped_by_distance() const { return m_v_idxs_grouped_by_distance; }

const IndexGroupedVector<const Index>& TupleGraphImpl::get_problem_vertices_grouped_by_distance() const { return m_problem_v_idxs_grouped_by_distance; }

TupleGraphList TupleGraphImpl::create(StateSpace state_space, std::optional<CertificateMaps>& certificate_maps, const Options& options)
{
    auto tuple_graphs = TupleGraphList {};

    for (const auto& v : state_space->get_graph().get_vertices())
    {
        tuple_graphs.push_back(create_tuple_graph(v, state_space, certificate_maps, options));
    }

    return tuple_graphs;
}

std::ostream& operator<<(std::ostream& out, const TupleGraphImpl& tuple_graph)
{
    const auto& problem_graph = tuple_graph.get_state_space()->get_graph();
    const auto& problem = *tuple_graph.get_state_space()->get_search_context()->get_problem();
    const auto& pddl_repositories = problem.get_repositories();
    auto atom_indices = iw::AtomIndexList {};

    out << "digraph {\n"
        << "rankdir=\"LR\"" << "\n";

    // 3. Tuple nodes.
    for (const auto& v_idx : tuple_graph.get_vertices_grouped_by_distance().front())
    {
        out << "Dangling" << v_idx << " [ label = \"\", style = invis ]\n";
    }

    for (const auto group : tuple_graph.get_vertices_grouped_by_distance())
    {
        for (const auto& v_idx : group)
        {
            const auto& vertex = tuple_graph.get_graph().get_vertex(v_idx);

            out << "t" << v_idx << "[";
            out << "label=<";
            out << "index=" << v_idx << "<BR/>";
            const auto fluent_atoms = pddl_repositories.get_ground_atoms_from_indices<FluentTag>(get_atom_tuple(vertex));
            out << "fluent_atoms=";
            mimir::operator<<(out, fluent_atoms);
            out << "<BR/>";
            out << "states=[";
            for (size_t i = 0; i < get_problem_vertices(vertex).size(); ++i)
            {
                const auto& state = graphs::get_state(problem_graph.get_vertex(get_problem_vertices(vertex).at(i)));
                if (i != 0)
                {
                    out << "<BR/>";
                }
                mimir::operator<<(out, std::make_tuple(state, std::cref(problem)));
            }
            out << "]>]\n";
        }
    }
    // 4. Group states with same distance together
    // 5. Tuple edges
    out << "{\n";
    for (const auto& v_idx : tuple_graph.get_vertices_grouped_by_distance().front())
    {
        out << "Dangling" << v_idx << "->t" << v_idx << "\n";
    }
    out << "}\n";
    for (const auto group : tuple_graph.get_vertices_grouped_by_distance())
    {
        out << "{\n";
        for (const auto& v_idx : group)
        {
            for (const auto& adj_v_idx : tuple_graph.get_graph().get_adjacent_vertex_indices<graphs::ForwardTag>(v_idx))
            {
                out << "t" << v_idx << "->" << "t" << adj_v_idx << "\n";
            }
        }
        out << "}\n";
    }
    out << "}\n";  // end digraph

    return out;
}
}
