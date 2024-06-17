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

#ifndef MIMIR_GRAPHS_OBJECT_GRAPH_HPP_
#define MIMIR_GRAPHS_OBJECT_GRAPH_HPP_

#include "mimir/formalism/formalism.hpp"
#include "mimir/graphs/coloring.hpp"
#include "mimir/graphs/digraph.hpp"
#include "mimir/search/state.hpp"

#include <ostream>

namespace mimir
{
class ObjectGraphFactory;

class ObjectGraph
{
private:
    std::shared_ptr<ProblemColorFunction> m_coloring_function;

    Digraph m_digraph;

    std::vector<std::vector<int>> m_vertex_partitioning;
    ColorList m_vertex_colors;
    ColorList m_edge_colors;

    friend class ObjectGraphFactory;

public:
    ObjectGraph(std::shared_ptr<ProblemColorFunction> coloring_function);

    const std::shared_ptr<ProblemColorFunction>& get_coloring_function() const;
    const Digraph& get_digraph() const;
    const std::vector<std::vector<int>>& get_vertex_partitioning() const;
    const ColorList& get_vertex_colors() const;
    const ColorList& get_edge_colors() const;
};

class ObjectGraphFactory
{
private:
    Problem m_problem;
    bool m_mark_true_goal_literals;

    std::shared_ptr<ProblemColorFunction> m_coloring_function;

    ObjectGraph m_object_graph;

    std::unordered_map<Object, int> m_object_to_vertex_id;
    std::set<Color> m_colors;
    std::unordered_map<int, int> m_color_to_surjective_l_color;

    int add_object_graph_structures(Object object, int num_vertices);

    template<PredicateCategory P>
    int add_ground_atom_graph_structures(GroundAtom<P> atom, int num_vertices);

    template<PredicateCategory P>
    int add_ground_literal_graph_structures(State state, GroundLiteral<P> atom, int num_vertices);

public:
    ObjectGraphFactory(Problem problem, bool mark_true_goal_literals = false);

    /// @brief Create and return a reference to the object graph.
    const ObjectGraph& create(State state, const PDDLFactories& pddl_factories);
};

/**
 * Pretty printing to dot representation
 */

extern std::ostream& operator<<(std::ostream& out, const ObjectGraph& object_graph);

/**
 * Implementations
 */

template<PredicateCategory P>
int ObjectGraphFactory::add_ground_atom_graph_structures(GroundAtom<P> atom, int num_vertices)
{
    auto prev_vertex_id = -1;
    for (size_t pos = 0; pos < atom->get_arity(); ++pos)
    {
        m_object_graph.m_vertex_colors.push_back(m_coloring_function->get_color(atom, pos));
        m_object_graph.m_digraph.add_edge(num_vertices, m_object_to_vertex_id.at(atom->get_objects().at(pos)));
        if (pos > 0)
        {
            m_object_graph.m_digraph.add_edge(prev_vertex_id, num_vertices);
        }

        prev_vertex_id = num_vertices;
        ++num_vertices;
    }
    return num_vertices;
}

template<PredicateCategory P>
int ObjectGraphFactory::add_ground_literal_graph_structures(State state, GroundLiteral<P> literal, int num_vertices)
{
    auto prev_vertex_id = -1;
    for (size_t pos = 0; pos < literal->get_atom()->get_arity(); ++pos)
    {
        m_object_graph.m_vertex_colors.push_back(m_coloring_function->get_color(state, literal, pos, m_mark_true_goal_literals));
        m_object_graph.m_digraph.add_edge(num_vertices, m_object_to_vertex_id.at(literal->get_atom()->get_objects().at(pos)));
        if (pos > 0)
        {
            m_object_graph.m_digraph.add_edge(prev_vertex_id, num_vertices);
        }

        prev_vertex_id = num_vertices;
        ++num_vertices;
    }
    return num_vertices;
}

}

#endif
