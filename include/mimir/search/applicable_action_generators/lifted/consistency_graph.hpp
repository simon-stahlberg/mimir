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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_CONSISTENCY_GRAPH_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_CONSISTENCY_GRAPH_HPP_

#include "mimir/formalism/formalism.hpp"

#include <optional>
#include <sstream>
#include <vector>

namespace mimir
{
template<PredicateCategory P>
class AssignmentSet;
}

namespace mimir::consistency_graph
{

using VertexID = size_t;
using ParameterID = size_t;
using ObjectID = size_t;

/// @brief A vertex [param/object] in the consistency graph.
class Vertex
{
private:
    VertexID m_id;
    ParameterID m_param;
    ObjectID m_object;

public:
    Vertex(VertexID id, ParameterID param, ObjectID object) : m_id(id), m_param(param), m_object(object) {}

    bool operator==(const Vertex& other) const { return m_id == other.m_id; }

    VertexID get_id() const { return m_id; }
    ParameterID get_parameter_index() const { return m_param; }
    ObjectID get_object_id() const { return m_object; }
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
using VertexIDs = std::vector<VertexID>;
using ObjectIDs = std::vector<ObjectID>;

/// @brief The StaticConsistencyGraph encodes the assignments to static conditions,
/// and hence, it is an overapproximation of the actual consistency graph.
class StaticConsistencyGraph
{
private:
    Problem m_problem;

    /* The data member of the consistency graph. */
    Vertices m_vertices;
    Edges m_edges;

    std::vector<VertexIDs> m_vertices_by_parameter_index;
    std::vector<ObjectIDs> m_objects_by_parameter_index;

public:
    /// @brief Construct a static consistency graph
    /// @param problem The problem context.
    /// @param begin_parameter_index The first parameter index for which consistent assignments are represented.
    /// @param end_parameter_index The last parameter index plus one for which consistent assignments are represented.
    /// @param static_conditions The static literals for which a bindings must be found.
    /// @param static_assignment_set The assignment set of static initial literals.
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
    const std::vector<VertexIDs>& get_vertices_by_parameter_index() const { return m_vertices_by_parameter_index; }

    /// @brief Get the object indices partitioned by the parameter index.
    const std::vector<ObjectIDs>& get_objects_by_parameter_index() const { return m_objects_by_parameter_index; }
};

/// @brief The Graphs is a collection of StaticConsistenctGraphs:
/// 1) one for the action precondition, and
/// 2) one for each condition of a universally quantified conditional effect
class Graphs
{
private:
    StaticConsistencyGraph m_precondition;
    // TODO: For universal effects, we do not need edges, lets keep it for simplicity now.
    std::vector<StaticConsistencyGraph> m_universal_effects;

public:
    Graphs(Problem problem, Action action, const AssignmentSet<Static>& static_assignment_set);

    const StaticConsistencyGraph& get_precondition_graph() const;
    const std::vector<StaticConsistencyGraph>& get_universal_effect_graphs() const;
};

/**
 * Print the graph nicely as dot format
 */

extern std::ostream& operator<<(std::ostream& out, std::tuple<const StaticConsistencyGraph&, const PDDLFactories&> data);

}

#endif
