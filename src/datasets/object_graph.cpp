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

#include "mimir/datasets/object_graph.hpp"

#include "mimir/formalism/color_function.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/state.hpp"

using namespace mimir::formalism;
using namespace mimir::search;

namespace mimir::datasets
{

/**
 * ObjectGraph
 */

static void
add_objects_graph_structures(State state, const ProblemImpl& problem, const ColorFunctionImpl& color_function, graphs::StaticVertexColoredDigraph& out_digraph)
{
    for (const auto& object : problem.get_problem_and_domain_objects())
    {
        const auto vertex_color = color_function.get_color(object);
        const auto v_idx = out_digraph.add_vertex(vertex_color);
        assert(v_idx == object->get_index());
    }
}

template<IsStaticOrFluentOrDerivedTag P>
static void add_ground_atom_graph_structures(const ColorFunctionImpl& color_function, GroundAtom<P> atom, graphs::StaticVertexColoredDigraph& out_digraph)
{
    for (size_t pos = 0; pos < atom->get_arity(); ++pos)
    {
        const auto vertex_color = color_function.get_color(atom, pos);
        const auto vertex_index = out_digraph.add_vertex(vertex_color);
        out_digraph.add_undirected_edge(vertex_index, atom->get_objects().at(pos)->get_index());
        if (pos > 0)
        {
            out_digraph.add_undirected_edge(vertex_index - 1, vertex_index);
        }
    }
}

static void add_ground_atoms_graph_structures(State state,
                                              const ProblemImpl& problem,
                                              const ColorFunctionImpl& color_function,
                                              graphs::StaticVertexColoredDigraph& out_digraph)
{
    for (const auto& atom : problem.get_repositories().get_ground_atoms_from_indices<StaticTag>(problem.get_static_initial_positive_atoms_bitset()))
    {
        add_ground_atom_graph_structures(color_function, atom, out_digraph);
    }
    for (const auto& atom : problem.get_repositories().get_ground_atoms_from_indices<FluentTag>(state->get_atoms<FluentTag>()))
    {
        add_ground_atom_graph_structures(color_function, atom, out_digraph);
    }
    for (const auto& atom : problem.get_repositories().get_ground_atoms_from_indices<DerivedTag>(state->get_atoms<DerivedTag>()))
    {
        add_ground_atom_graph_structures(color_function, atom, out_digraph);
    }
}

template<IsStaticOrFluentOrDerivedTag P>
static void add_ground_literal_graph_structures(State state,
                                                const ProblemImpl& problem,
                                                const ColorFunctionImpl& color_function,
                                                GroundLiteral<P> literal,
                                                graphs::StaticVertexColoredDigraph& out_digraph)
{
    for (size_t pos = 0; pos < literal->get_atom()->get_arity(); ++pos)
    {
        const auto vertex_color = color_function.get_color(literal, pos);
        const auto vertex_index = out_digraph.add_vertex(vertex_color);
        out_digraph.add_undirected_edge(vertex_index, literal->get_atom()->get_objects().at(pos)->get_index());
        if (pos > 0)
        {
            out_digraph.add_undirected_edge(vertex_index - 1, vertex_index);
        }
    }
}

static void add_ground_goal_literals_graph_structures(State state,
                                                      const ProblemImpl& problem,
                                                      const ColorFunctionImpl& color_function,
                                                      graphs::StaticVertexColoredDigraph& out_digraph)
{
    for (const auto& literal : problem.get_goal_condition<StaticTag>())
    {
        add_ground_literal_graph_structures(state, problem, color_function, literal, out_digraph);
    }
    for (const auto& literal : problem.get_goal_condition<FluentTag>())
    {
        add_ground_literal_graph_structures(state, problem, color_function, literal, out_digraph);
    }
    for (const auto& literal : problem.get_goal_condition<DerivedTag>())
    {
        add_ground_literal_graph_structures(state, problem, color_function, literal, out_digraph);
    }
}

graphs::StaticVertexColoredDigraph
create_object_graph(search::State state, const formalism::ProblemImpl& problem, const formalism::ColorFunctionImpl& color_function)
{
    auto graph = graphs::StaticVertexColoredDigraph();

    add_objects_graph_structures(state, problem, color_function, graph);

    add_ground_atoms_graph_structures(state, problem, color_function, graph);

    add_ground_goal_literals_graph_structures(state, problem, color_function, graph);

    return graph;
}

}
