#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_LIFTED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_LIFTED_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/factories.hpp"
#include "mimir/search/actions/dense.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/applicable_action_generators/internal_representation.hpp"
#include "mimir/search/states.hpp"

#include <flatmemory/details/view_const.hpp>
#include <unordered_map>
#include <vector>

namespace mimir
{

namespace consistency_graph
{

using ParameterID = size_t;
using ObjectID = size_t;

/// @brief A vertex [param/object] in the consistency graph.
class Vertex
{
private:
    ParameterID m_param;
    ObjectID m_object;

public:
    Vertex(ParameterID param, ObjectID object) : m_param(param), m_object(object) {}

    ParameterID get_param_index() const { return m_param; }
    ObjectID get_object_index() const { return m_object; }
};

using NumVertices = size_t;
using NumParameters = size_t;
using NumObjects = size_t;
using VertexID = size_t;
using VertexIDs = std::vector<VertexID>;

/// @brief An undirected edge {src,dst} in the consistency graph.
class Edge
{
private:
    Vertex m_src;
    Vertex m_dst;

public:
    Edge(Vertex src, Vertex dst) : m_src(src), m_dst(dst) {}

    Vertex get_src() const { return m_src; }
    Vertex get_dst() const { return m_dst; }
};

using NumEdges = size_t;
using EdgeID = size_t;

using Vertices = std::vector<Vertex>;
using Edges = std::vector<Edge>;

/// @brief AssignmentSet is a helper class representing a set of functions
/// f : Predicates x Params(A) x Object x Params(A) x Object -> {true, false} where
///   1. f(p,i,o,j,o') = true iff there exists an atom p(...,o_i,...,o'_j)
///   2. f(p,i,o,-1,-1) = true iff there exists an atom p(...,o_i,...)
/// with respective meanings
///   1. the assignment [i/o], [j/o'] is consistent
///   2. the assignment [i/o] is consistent
///
/// We say that an assignment set is static if all atoms it considers are static.
class AssignmentSet
{
private:
    Problem m_problem;

    // The underlying function
    std::vector<std::vector<bool>> m_f;

public:
    /// @brief Construct from a given set of ground atoms.
    AssignmentSet(Problem problem, const GroundAtomList& atoms);

    /// @brief Return true iff all literals are consistent with
    /// 1. the assignment set, and 2. the edge of the consistency graph.
    ///
    /// The meaning of the result being true is that the edge remains consistent.
    bool literal_all_consistent(const std::vector<Literal>& literals, const Edge& consistent_edge) const;
};

/// @brief The StaticConsistencyGraph encodes the part of the consisteny graph
///        that is static for each state for a single action.
class StaticConsistencyGraph
{
private:
    Problem m_problem;

    /* The data member of the consistency graph. */
    Vertices m_vertices;
    Edges m_edges;
    std::vector<VertexIDs> m_vertices_by_parameter_index;

public:
    StaticConsistencyGraph(Problem problem, Action action, const PDDLFactories& factories);

    /// @brief Get the vertices.
    const Vertices& get_vertices() const { return m_vertices; }

    /// @brief Get the edges.
    const Edges& get_edges() const { return m_edges; }

    /// @brief Get the vertices partitioned by the parameter index.
    const std::vector<VertexIDs> get_vertices_by_parameter_index() const { return m_vertices_by_parameter_index; }

    /// @brief Return a dot string representation of the graph.
    std::string to_dot() const;
};

}

using GroundFunctionToValue = std::unordered_map<GroundFunction, double>;

/**
 * Fully specialized implementation class.
 */
template<>
class AAG<LiftedAAGDispatcher<DenseStateTag>> : public IStaticAAG<AAG<LiftedAAGDispatcher<DenseStateTag>>>
{
private:
    Problem m_problem;
    PDDLFactories& m_pddl_factories;

    flat::DenseActionSet m_actions;
    std::vector<DenseAction> m_actions_by_index;
    Builder<ActionDispatcher<DenseStateTag>> m_action_builder;

    GroundFunctionToValue m_ground_function_value_costs;

    // D: will be substituted by StaticConsistencyGraph::vertices_by_parameter_index
    std::unordered_map<Action, std::vector<std::vector<size_t>>> m_partitions;
    // D: will be substituted by StaticConsistencyGraph::vertices
    std::unordered_map<Action, std::vector<Assignment>> m_to_vertex_assignment;
    // D: will be substituted by StaticConsistencyGraph::edges
    std::unordered_map<Action, std::vector<AssignmentPair>> m_statically_consistent_assignments;

    std::unordered_map<Action, consistency_graph::StaticConsistencyGraph> m_static_consistency_graphs;

    GroundLiteral ground_literal(const Literal& literal, const ObjectList& binding) const;

    /// @brief Returns true if all nullary literals in the precondition hold, false otherwise.
    bool nullary_preconditions_hold(const Action& action, DenseState state) const;

    void nullary_case(const Action& action, DenseState state, std::vector<DenseAction>& out_applicable_actions);

    void unary_case(const Action& action, DenseState state, std::vector<DenseAction>& out_applicable_actions);

    void general_case(const consistency_graph::AssignmentSet& assignment_sets,
                      const Action& action,
                      DenseState state,
                      std::vector<DenseAction>& out_applicable_actions);

    /* Implement IStaticAAG interface */
    friend class IStaticAAG<AAG<LiftedAAGDispatcher<DenseStateTag>>>;

    void generate_applicable_actions_impl(const DenseState state, std::vector<DenseAction>& out_applicable_actions);

public:
    AAG(Problem problem, PDDLFactories& pddl_factories);

    /// @brief Ground an action and return a view onto it.
    DenseAction ground_action(const Action& action, ObjectList&& binding);

    /// @brief Return all actions.
    [[nodiscard]] const flat::DenseActionSet& get_actions() const;

    /// @brief Return the action with the given id.
    [[nodiscard]] DenseAction get_action(size_t action_id) const;
};

}  // namespace mimir

#endif