#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_LIFTED_CONSISTENCY_GRAPH_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_LIFTED_CONSISTENCY_GRAPH_HPP_

#include "mimir/formalism/declarations.hpp"

#include <optional>
#include <sstream>
#include <vector>

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

/// @brief The StaticConsistencyGraph encodes the assignments to static conditions,
/// and hence, it is an overapproximation of the actual consistency graph.
class StaticConsistencyGraph
{
private:
    Problem m_problem;
    size_t m_arity;

    /* The data member of the consistency graph. */
    Vertices m_vertices;
    Edges m_edges;
    std::vector<VertexIDs> m_vertices_by_parameter_index;

public:
    /// @brief Construct a static consistency graph
    /// @param problem The problem
    /// @param arity The number of variables that occur in static_preconditions
    /// @param static_conditions The static literals for which a bindings must be found.
    StaticConsistencyGraph(Problem problem, size_t arity, const LiteralList& static_conditions);

    /// @brief Get the vertices.
    const Vertices& get_vertices() const { return m_vertices; }

    /// @brief Get the edges.
    const Edges& get_edges() const { return m_edges; }

    /// @brief Get the vertices partitioned by the parameter index.
    const std::vector<VertexIDs>& get_vertices_by_parameter_index() const { return m_vertices_by_parameter_index; }

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
    std::vector<std::optional<StaticConsistencyGraph>> m_universal_effects;

public:
    Graphs(Problem problem, Action action);

    const std::optional<StaticConsistencyGraph>& get_precondition_graph() const;
    const std::vector<std::optional<StaticConsistencyGraph>>& get_universal_effect_graphs() const;
};

}

#endif
