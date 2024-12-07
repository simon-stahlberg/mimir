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

#ifndef MIMIR_SEARCH_GROUNDING_CONSISTENCY_GRAPH_HPP_
#define MIMIR_SEARCH_GROUNDING_CONSISTENCY_GRAPH_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/common/types.hpp"
#include "mimir/formalism/declarations.hpp"

#include <optional>
#include <sstream>
#include <vector>

namespace mimir
{
template<PredicateTag P>
class AssignmentSet;
}

namespace mimir::consistency_graph
{

using ParameterIndex = size_t;
using VertexIndex = size_t;
using ObjectIndex = size_t;
using VertexIndexList = std::vector<VertexIndex>;
using ObjectIndexList = std::vector<ObjectIndex>;

/// @brief A vertex [parameter_index/object_index] in the consistency graph.
class Vertex
{
private:
    VertexIndex m_index;
    ParameterIndex m_parameter_index;
    ObjectIndex m_object_index;

public:
    Vertex(VertexIndex index, ParameterIndex parameter_index, ObjectIndex object_index) :
        m_index(index),
        m_parameter_index(parameter_index),
        m_object_index(object_index)
    {
    }

    bool operator==(const Vertex& other) const { return m_index == other.m_index; }

    VertexIndex get_index() const { return m_index; }
    ParameterIndex get_parameter_index() const { return m_parameter_index; }
    ObjectIndex get_object_index() const { return m_object_index; }
};

/// @brief An undirected edge {src,dst} in the consistency graph.
class Edge
{
private:
    Vertex m_src;
    Vertex m_dst;

public:
    Edge(Vertex src, Vertex dst) : m_src(std::move(src)), m_dst(std::move(dst)) {}

    bool operator==(const Edge& other) const { return m_src == other.m_src && m_dst == other.m_dst; }

    const Vertex& get_src() const { return m_src; }
    const Vertex& get_dst() const { return m_dst; }
};

using Vertices = std::vector<Vertex>;
using Edges = std::vector<Edge>;

/// @brief The StaticConsistencyGraph encodes the assignments to static conditions,
/// and hence, it is an overapproximation of the actual consistency graph.
class StaticConsistencyGraph
{
private:
    Problem m_problem;

    /* The data member of the consistency graph. */
    Vertices m_vertices;
    Edges m_edges;

    std::vector<VertexIndexList> m_vertices_by_parameter_index;
    std::vector<ObjectIndexList> m_objects_by_parameter_index;

public:
    /// @brief Construct a static consistency graph
    /// @parameter_index problem The problem context.
    /// @parameter_index begin_parameter_index The first parameter index for which consistent assignments are represented.
    /// @parameter_index end_parameter_index The last parameter index plus one for which consistent assignments are represented.
    /// @parameter_index static_conditions The static literals for which a bindings must be found.
    /// @parameter_index static_assignment_set The assignment set of static initial literals.
    ///
    /// For universal effects, we can set first and last parameter indices respectively
    /// to find consistent assignments irrespective of the action parameter bindings.
    /// More specifically:
    ///  1. for action parameters, we set first to 0 and last to 0 + arity(action)
    ///  2. for universal effects, we set first to arity(action) and last to arity(action) + arity(effect)
    StaticConsistencyGraph(Problem problem,
                           size_t begin_parameter_index,
                           size_t end_parameter_index,
                           const LiteralList<Static>& static_conditions,
                           const AssignmentSet<Static>& static_assignment_set);

    /// @brief Get the vertices.
    const Vertices& get_vertices() const { return m_vertices; }

    /// @brief Get the edges.
    const Edges& get_edges() const { return m_edges; }

    /// @brief Get the vertex indices partitioned by the parameter index.
    const std::vector<VertexIndexList>& get_vertices_by_parameter_index() const { return m_vertices_by_parameter_index; }

    /// @brief Get the object_index indices partitioned by the parameter index.
    const std::vector<ObjectIndexList>& get_objects_by_parameter_index() const { return m_objects_by_parameter_index; }
};

/// @brief The Graphs is a collection of StaticConsistenctGraphs:
/// 1) one for the action precondition, and
/// 2) one for each condition of a universally quantified conditional effect
class Graphs
{
private:
    StaticConsistencyGraph m_precondition;
    // TODO: For universal effects, we do not need edges, lets keep it for simplicity now.
    std::vector<StaticConsistencyGraph> m_conditional_effects;

public:
    Graphs(Problem problem, Action action, const AssignmentSet<Static>& static_assignment_set);

    const StaticConsistencyGraph& get_precondition_graph() const;
    const std::vector<StaticConsistencyGraph>& get_conditional_effect_graphs() const;
};

}

namespace mimir
{
/**
 * Print the graph nicely as dot format
 */
template<>
std::ostream& operator<<(std::ostream& out, const std::tuple<const consistency_graph::StaticConsistencyGraph&, const PDDLRepositories&>& data);
}

#endif
