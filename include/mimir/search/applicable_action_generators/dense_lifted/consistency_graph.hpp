#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_LIFTED_CONSISTENCY_GRAPH_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_LIFTED_CONSISTENCY_GRAPH_HPP_

#include "mimir/formalism/declarations.hpp"

#include <optional>
#include <sstream>
#include <vector>

namespace mimir
{
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

    VertexID get_id() const { return m_id; }
    ParameterID get_param_index() const { return m_param; }
    ObjectID get_object_index() const { return m_object; }

    friend std::ostream& operator<<(std::ostream& out, const Vertex& vertex)
    {
        out << "Vertex(id=" << vertex.get_id() << " param=" << vertex.get_param_index() << " object=" << vertex.get_object_index() << ")";
        return out;
    }
};

/// @brief An undirected edge {src,dst} in the consistency graph.
class Edge
{
private:
    Vertex m_src;
    Vertex m_dst;

public:
    Edge(Vertex src, Vertex dst) : m_src(std::move(src)), m_dst(std::move(dst)) {}

    const Vertex& get_src() const { return m_src; }
    const Vertex& get_dst() const { return m_dst; }

    friend std::ostream& operator<<(std::ostream& out, const Edge& edge)
    {
        out << "Edge(src=" << edge.get_src() << " dst=" << edge.get_dst() << ")";
        return out;
    }
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
                           const LiteralList& static_conditions,
                           const AssignmentSet& static_assignment_set);

    /// @brief Get the vertices.
    const Vertices& get_vertices() const { return m_vertices; }

    /// @brief Get the edges.
    const Edges& get_edges() const { return m_edges; }

    /// @brief Get the vertex indices partitioned by the parameter index.
    const std::vector<VertexIDs>& get_vertices_by_parameter_index() const { return m_vertices_by_parameter_index; }

    /// @brief Get the object indices partitioned by the parameter index.
    const std::vector<ObjectIDs>& get_objects_by_parameter_index() const { return m_objects_by_parameter_index; }

    friend std::ostream& operator<<(std::ostream& out, const StaticConsistencyGraph& graph);
};

/// @brief The Graphs is a collection of StaticConsistenctGraphs:
/// 1) one for the action precondition, and
/// 2) one for each condition of a universally quantified conditional effect
class Graphs
{
private:
    // Store graphs only if arity >= 2.
    std::optional<StaticConsistencyGraph> m_precondition;
    std::vector<StaticConsistencyGraph> m_universal_effects;

public:
    Graphs(Problem problem, Action action, const AssignmentSet& static_assignment_set);

    const std::optional<StaticConsistencyGraph>& get_precondition_graph() const;
    const std::vector<StaticConsistencyGraph>& get_universal_effect_graphs() const;
};

}

#endif
