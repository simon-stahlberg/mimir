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

#include "mimir/formalism/repositories.hpp"
#include "tuple_graph_factory.hpp"

namespace mimir
{
TupleGraph::TupleGraph(const ProblemContext& context,
                       const ProblemGraph& problem_graph,
                       const ClassGraph& class_graph,
                       const TupleIndexMapper& index_mapper,
                       InternalTupleGraph graph,
                       IndexGroupedVector<const Index> vertices_grouped_by_distance,
                       IndexGroupedVector<const Index> problem_vertices_grouped_by_distance) :
    m_context(context),
    m_problem_graph(problem_graph),
    m_class_graph(class_graph),
    m_index_mapper(index_mapper),
    m_graph(std::move(graph)),
    m_v_idxs_grouped_by_distance(std::move(vertices_grouped_by_distance)),
    m_problem_v_idxs_grouped_by_distance(std::move(problem_vertices_grouped_by_distance))
{
}

const ProblemContext& TupleGraph::get_context() const { return m_context; }

const ClassGraph& TupleGraph::get_class_graph() const { return m_class_graph; }

const ProblemGraph& TupleGraph::get_problem_graph() const { return m_problem_graph; }

const TupleIndexMapper& TupleGraph::get_index_mapper() const { return m_index_mapper; }

const InternalTupleGraph& TupleGraph::get_graph() const { return m_graph; }

const IndexGroupedVector<const Index>& TupleGraph::get_vertices_grouped_by_distance() const { return m_v_idxs_grouped_by_distance; }

const IndexGroupedVector<const Index>& TupleGraph::get_problem_vertices_grouped_by_distance() const { return m_problem_v_idxs_grouped_by_distance; }

TupleGraphCollection::TupleGraphCollection(const GeneralizedStateSpace& state_space, const Options& options) :
    m_per_problem_index_mapper(),
    m_per_class_vertex_tuple_graph()
{
    for (const auto& search_context : state_space.get_search_contexts())
    {
        m_per_problem_index_mapper.push_back(
            TupleIndexMapper(options.width,
                             boost::hana::at_key(search_context.get_problem_context().get_repositories()->get_pddl_type_to_factory(),
                                                 boost::hana::type<GroundAtomImpl<Fluent>> {})
                                     .size()
                                 + boost::hana::at_key(search_context.get_problem_context().get_repositories()->get_pddl_type_to_factory(),
                                                       boost::hana::type<GroundAtomImpl<Derived>> {})
                                       .size()));
    }
    for (const auto& class_vertex : state_space.get_class_state_space().get_graph().get_vertices())
    {
        m_per_class_vertex_tuple_graph.push_back(create_tuple_graph(state_space.get_problem_vertex(class_vertex),
                                                                    state_space.get_problem_graph(class_vertex),
                                                                    state_space.get_class_state_space().get_graph(),
                                                                    state_space.get_search_contexts().at(get_problem_index(class_vertex)).get_problem_context(),
                                                                    m_per_problem_index_mapper.at(get_problem_index(class_vertex)),
                                                                    options));
    }
}

const TupleGraphList& TupleGraphCollection::get_per_class_vertex_tuple_graph() const { return m_per_class_vertex_tuple_graph; }

std::ostream& operator<<(std::ostream& out, const TupleGraph& tuple_graph)
{
    const auto problem = tuple_graph.get_context().get_problem();
    const auto& pddl_repositories = *tuple_graph.get_context().get_repositories();
    auto atom_indices = AtomIndexList {};

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
            out << "tuple index=" << get_tuple_index(vertex) << "<BR/>";

            tuple_graph.get_index_mapper().to_atom_indices(get_tuple_index(vertex), atom_indices);
            const auto fluent_atoms = pddl_repositories.get_ground_atoms_from_indices<Fluent>(atom_indices);
            out << "fluent_atoms=" << fluent_atoms << "<BR/>";
            out << "states=[";
            for (size_t i = 0; i < get_problem_vertices(vertex).size(); ++i)
            {
                const auto& state = get_state(tuple_graph.get_problem_graph().get_vertex(get_problem_vertices(vertex).at(i)));
                if (i != 0)
                {
                    out << "<BR/>";
                }
                out << std::make_tuple(problem, state, std::cref(pddl_repositories));
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
            for (const auto& adj_v_idx : tuple_graph.get_graph().get_adjacent_vertex_indices<ForwardTraversal>(v_idx))
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
