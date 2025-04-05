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

static ObjectMap<graphs::VertexIndex>
add_objects_graph_structures(State state, const ProblemImpl& problem, const ColorFunctionImpl& color_function, graphs::StaticVertexColoredDigraph& out_digraph)
{
    auto object_to_vertex_index = ObjectMap<graphs::VertexIndex> {};

    const auto add_atom_objects_func = [&](auto&& atom)
    {
        for (const auto& object : atom->get_objects())
        {
            if (!object_to_vertex_index.contains(object))
            {
                const auto vertex_color = color_function.get_color(object);
                object_to_vertex_index.emplace(object, out_digraph.add_vertex(vertex_color));
            }
        }
    };

    for (const auto& atom : problem.get_static_initial_atoms())
    {
        add_atom_objects_func(atom);
    }
    for (const auto& atom_index : state->get_atoms<FluentTag>())
    {
        add_atom_objects_func(problem.get_repositories().get_ground_atom<FluentTag>(atom_index));
    }
    for (const auto& atom_index : state->get_atoms<DerivedTag>())
    {
        add_atom_objects_func(problem.get_repositories().get_ground_atom<FluentTag>(atom_index));
    }

    return object_to_vertex_index;
}

static ToObjectMap<graphs::VertexIndex> compute_v_idx_to_object(const ObjectMap<graphs::VertexIndex>& object_to_v_idx)
{
    auto v_idx_to_object = ToObjectMap<graphs::VertexIndex> {};
    for (const auto [object, v_idx] : object_to_v_idx)
    {
        v_idx_to_object.emplace(v_idx, object);
    }
    return v_idx_to_object;
}

template<IsStaticOrFluentOrDerivedTag P>
static void add_ground_atom_graph_structures(const ColorFunctionImpl& color_function,
                                             const std::unordered_map<Object, graphs::VertexIndex>& object_to_vertex_index,
                                             GroundAtom<P> atom,
                                             graphs::StaticVertexColoredDigraph& out_digraph)
{
    for (size_t pos = 0; pos < atom->get_arity(); ++pos)
    {
        const auto vertex_color = color_function.get_color(atom, pos);
        const auto vertex_index = out_digraph.add_vertex(vertex_color);
        out_digraph.add_undirected_edge(vertex_index, object_to_vertex_index.at(atom->get_objects().at(pos)));
        if (pos > 0)
        {
            out_digraph.add_undirected_edge(vertex_index - 1, vertex_index);
        }
    }
}

static void add_ground_atoms_graph_structures(State state,
                                              const ProblemImpl& problem,
                                              const ColorFunctionImpl& color_function,
                                              const std::unordered_map<Object, graphs::VertexIndex>& object_to_vertex_index,
                                              graphs::StaticVertexColoredDigraph& out_digraph)
{
    for (const auto& atom : problem.get_repositories().get_ground_atoms_from_indices<StaticTag>(problem.get_static_initial_positive_atoms_bitset()))
    {
        add_ground_atom_graph_structures(color_function, object_to_vertex_index, atom, out_digraph);
    }
    for (const auto& atom : problem.get_repositories().get_ground_atoms_from_indices<FluentTag>(state->get_atoms<FluentTag>()))
    {
        add_ground_atom_graph_structures(color_function, object_to_vertex_index, atom, out_digraph);
    }
    for (const auto& atom : problem.get_repositories().get_ground_atoms_from_indices<DerivedTag>(state->get_atoms<DerivedTag>()))
    {
        add_ground_atom_graph_structures(color_function, object_to_vertex_index, atom, out_digraph);
    }
}

template<IsStaticOrFluentOrDerivedTag P>
static void add_ground_literal_graph_structures(State state,
                                                const ProblemImpl& problem,
                                                const ColorFunctionImpl& color_function,
                                                const std::unordered_map<Object, graphs::VertexIndex>& object_to_vertex_index,
                                                GroundLiteral<P> literal,
                                                graphs::StaticVertexColoredDigraph& out_digraph)
{
    for (size_t pos = 0; pos < literal->get_atom()->get_arity(); ++pos)
    {
        const auto vertex_color = color_function.get_color(literal, pos);
        const auto vertex_index = out_digraph.add_vertex(vertex_color);
        out_digraph.add_undirected_edge(vertex_index, object_to_vertex_index.at(literal->get_atom()->get_objects().at(pos)));
        if (pos > 0)
        {
            out_digraph.add_undirected_edge(vertex_index - 1, vertex_index);
        }
    }
}

static void add_ground_goal_literals_graph_structures(State state,
                                                      const ProblemImpl& problem,
                                                      const ColorFunctionImpl& color_function,
                                                      const std::unordered_map<Object, graphs::VertexIndex>& object_to_vertex_index,
                                                      graphs::StaticVertexColoredDigraph& out_digraph)
{
    for (const auto& literal : problem.get_goal_condition<StaticTag>())
    {
        add_ground_literal_graph_structures(state, problem, color_function, object_to_vertex_index, literal, out_digraph);
    }
    for (const auto& literal : problem.get_goal_condition<FluentTag>())
    {
        add_ground_literal_graph_structures(state, problem, color_function, object_to_vertex_index, literal, out_digraph);
    }
    for (const auto& literal : problem.get_goal_condition<DerivedTag>())
    {
        add_ground_literal_graph_structures(state, problem, color_function, object_to_vertex_index, literal, out_digraph);
    }
}

ObjectGraph::ObjectGraph(search::State state, const formalism::ProblemImpl& problem, const formalism::ColorFunctionImpl& color_function) :
    m_graph(),
    m_object_to_v_idx(),
    m_v_idx_to_object()
{
    m_object_to_v_idx = add_objects_graph_structures(state, problem, color_function, m_graph);
    m_v_idx_to_object = compute_v_idx_to_object(m_object_to_v_idx);

    add_ground_atoms_graph_structures(state, problem, color_function, m_object_to_v_idx, m_graph);

    add_ground_goal_literals_graph_structures(state, problem, color_function, m_object_to_v_idx, m_graph);
}

ObjectList ObjectGraph::apply_permutation(const ObjectList& objects, const std::vector<int>& v_idx_permutation) const
{
    auto result = ObjectList {};
    result.reserve(objects.size());
    for (const auto& object : objects)
    {
        result.push_back(m_v_idx_to_object.at(v_idx_permutation.at(m_object_to_v_idx.at(object))));
    }
    return result;
}

const graphs::StaticVertexColoredDigraph& ObjectGraph::get_graph() const { return m_graph; }
}
