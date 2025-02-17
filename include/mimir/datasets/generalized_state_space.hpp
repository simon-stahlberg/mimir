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

#ifndef MIMIR_DATASETS_PROBLEM_CLASS_STATE_SPACE_HPP_
#define MIMIR_DATASETS_PROBLEM_CLASS_STATE_SPACE_HPP_

#include "mimir/common/types.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/graphs/declarations.hpp"
#include "mimir/graphs/graph_edges.hpp"
#include "mimir/graphs/graph_vertices.hpp"
#include "mimir/graphs/static_graph.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir
{

/**
 * ProblemGraph
 */

struct ProblemOptions
{
    bool symmetry_pruning = false;
    bool mark_true_goal_literals = false;
    bool remove_if_unsolvable = true;
    uint32_t max_num_states = std::numeric_limits<uint32_t>::max();
    uint32_t timeout_ms = std::numeric_limits<uint32_t>::max();
};

struct ProblemContext
{
    Problem problem = nullptr;
    std::shared_ptr<StateRepository> state_repository = nullptr;
    std::shared_ptr<IApplicableActionGenerator> applicable_action_generator = nullptr;
};

using ProblemContextList = std::vector<ProblemContext>;

/// @brief `ProblemVertex` encapsulates information about a state in the context of a `GeneralizedStateSpace`.
/// The `Index` is the index of the corresponding `ClassVertex` in the `GeneralizedStateSpace`.
/// The `State` is the underlying planning state.
using ProblemVertex = Vertex<Index, State>;
using ProblemVertexList = std::vector<ProblemVertex>;

inline Index get_class_vertex_index(const ProblemVertex& vertex) { return vertex.get_property<0>(); }

inline State get_state(const ProblemVertex& vertex) { return vertex.get_property<1>(); }

/// @brief `ProblemEdge` encapsulates information about a state transition in the context of a `GeneralizedStateSpace`.
/// The `Index` is the index of the corresponding `ClassEdge` in the `GeneralizedStateSpace`.
/// The `GroundAction` is the underlying ground action.
using ProblemEdge = Edge<Index, GroundAction, ContinuousCost>;
using ProblemEdgeList = std::vector<ProblemEdge>;

inline Index get_class_edge_index(const ProblemEdge& edge) { return edge.get_property<0>(); }

inline GroundAction get_action(const ProblemEdge& edge) { return edge.get_property<1>(); }

inline ContinuousCost get_action_cost(const ProblemEdge& edge) { return edge.get_property<2>(); }

using StaticProblemGraph = StaticGraph<ProblemVertex, ProblemEdge>;
using StaticProblemGraphList = std::vector<StaticProblemGraph>;
using ProblemGraph = StaticBidirectionalGraph<StaticProblemGraph>;
using ProblemGraphList = std::vector<ProblemGraph>;

/**
 * ClassGraph
 */

struct ClassOptions
{
    bool sort_ascending_by_num_states = true;

    ProblemOptions problem_options = ProblemOptions();
};

/// @brief `ClassVertex` encapsulates information about a vertex where
/// the first `Index` is the index of the `ClassVertex` in the `ClassGraph`.
/// the second `Index` is the index to the `ProblemGraph` in the `ClassGraph`
/// the third `Index` is the index of the `ProblemVertex` in the `ProblemGraph`
using ClassVertex = Vertex<Index, Index, Index, DiscreteCost, ContinuousCost, bool, bool, bool, bool>;
using ClassVertexList = std::vector<ClassVertex>;

inline Index get_class_vertex_index(const ClassVertex& vertex) { return vertex.get_property<0>(); }

inline Index get_problem_index(const ClassVertex& vertex) { return vertex.get_property<1>(); }

inline Index get_problem_vertex_index(const ClassVertex& vertex) { return vertex.get_property<2>(); }

inline DiscreteCost get_unit_goal_distance(const ClassVertex& vertex) { return vertex.get_property<3>(); }

inline ContinuousCost get_action_goal_distance(const ClassVertex& vertex) { return vertex.get_property<4>(); }

inline bool is_initial(const ClassVertex& vertex) { return vertex.get_property<5>(); }

inline bool is_goal(const ClassVertex& vertex) { return vertex.get_property<6>(); }

inline bool is_unsolvable(const ClassVertex& vertex) { return vertex.get_property<7>(); }

inline bool is_alive(const ClassVertex& vertex) { return vertex.get_property<8>(); }

/// @brief `ClassEdge` encapsulates information about an edge where
/// the first `Index` is the index of the `ClassEdge` in the `ClassGraph`.
/// the second `Index` is the index to the `ProblemGraph` in the `ClassGraph`.
/// the third `Index` is the index of the `ProblemEdge` in the `ProblemGraph`.
using ClassEdge = Edge<Index, Index, Index, ContinuousCost>;
using ClassEdgeList = std::vector<ClassEdge>;

inline Index get_class_edge_index(const ClassEdge& edge) { return edge.get_property<0>(); }

inline Index get_problem_index(const ClassEdge& edge) { return edge.get_property<1>(); }

inline Index get_problem_edge_index(const ClassEdge& edge) { return edge.get_property<2>(); }

inline Index get_action_cost(const ClassEdge& edge) { return edge.get_property<3>(); }

using StaticClassGraph = StaticGraph<ClassVertex, ClassEdge>;
using ClassGraph = StaticBidirectionalGraph<StaticClassGraph>;

class ClassStateSpace
{
private:
    ClassGraph m_graph;  ///< Core data.

    /* Initialize additional convenience data in the constructor! */

    IndexSet m_initial_vertices;     ///< Convenience data.
    IndexSet m_goal_vertices;        ///< Convenience data.
    IndexSet m_unsolvable_vertices;  ///< Convenience data.
    IndexSet m_alive_vertices;       ///< Convenience data.

public:
    ClassStateSpace() = default;
    explicit ClassStateSpace(ClassGraph graph);
    ClassStateSpace(const ClassStateSpace& other) = delete;
    ClassStateSpace& operator=(const ClassStateSpace& other) = delete;
    ClassStateSpace(ClassStateSpace&& other) = default;
    ClassStateSpace& operator=(ClassStateSpace&& other) = default;

    const ClassGraph& get_graph() const;  ///< Core data getter.

    const IndexSet& get_initial_vertices() const;     ///< Convenience data getter.
    const IndexSet& get_goal_vertices() const;        ///< Convenience data getter.
    const IndexSet& get_unsolvable_vertices() const;  ///< Convenience data getter.
    const IndexSet& get_alive_vertices() const;       ///< Convenience data getter.
};

/// @brief `GeneralizedStateSpace` encapsulates a `ProblemGraphList` Q with an additional `ClassStateSpace` structure on top.
///
/// There is a one-to-many mapping from `ClassVertex` (resp. `ClassEdge`) to `ProblemVertex` (resp. `ProblemEdge`),
/// which is a strict one-to-many mapping, if symmetries across different problems are detected.
/// From each `ClassVertex` (resp. `ClassEdge`) one can access the single representative `ProblemVertex` (resp. `ProblemEdge`).
/// From each `ProblemVertex` (resp. `ProblemEdge`) one can access the corresponding `ClassVertex` (resp. `ClassEdge`).
///
/// Example use case 1: Learning general policies
/// The `ClassStateSpace` contains the relevant vertices and edges and provide access to their underlying representative state or ground action.
/// When learning from a subset of problems or subset of states, one can use `create_induced_subspace`, to obtain the subspace.
class GeneralizedStateSpace
{
private:
    ProblemContextList m_problem_contexts;  ///< The problem contexts.
    ProblemGraphList m_problem_graphs;      ///< The child-level graphs.
    ClassStateSpace m_class_state_space;    ///< The top-level state space.

public:
    /* Construct from contexts. */
    GeneralizedStateSpace(ProblemContextList contexts, const ClassOptions& options = ClassOptions());
    GeneralizedStateSpace(ProblemContext context, const ClassOptions& options = ClassOptions());

    /* Construct from files. */
    GeneralizedStateSpace(const fs::path& domain_filepath, const fs::path& problem_filepath, const ClassOptions& options = ClassOptions());
    GeneralizedStateSpace(const fs::path& domain_filepath, const std::vector<fs::path>& problem_filepaths, const ClassOptions& options = ClassOptions());

    /**
     * Getters
     */

    const ProblemContextList& get_problem_contexts() const;
    const ProblemGraphList& get_problem_state_spaces() const;
    const ClassStateSpace& get_class_state_space() const;

    /// @brief Ground `Class` related structures to `Problem` related structures
    /// to access detailed problem specific information about the state.
    const ProblemGraph& get_problem_state_space(const ClassVertex& vertex) const;
    const ProblemGraph& get_problem_state_space(const ClassEdge& edge) const;
    const ProblemVertex& get_problem_vertex(const ClassVertex& vertex) const;
    const ProblemEdge& get_problem_edge(const ClassEdge& edge) const;

    /// @brief Lift `Problem` related structures to `Class` related structures.
    const ClassVertex& get_class_vertex(const ProblemVertex& vertex) const;
    const ClassEdge& get_class_edge(const ProblemEdge& edge) const;

    /**
     * Construct subgraphs for learning from fragments of the `ClassStateSpace`.
     */

    /// @brief Create the induced `ClassStateSpace` by the given `ClassVertexList`.
    /// This function copies the set of `ClassVertex`
    /// and all `ClassEdge` between those vertices into a new `ClassGraph`.
    ClassStateSpace create_induced_subspace(const ClassVertexList& vertices) const;
    /// @brief Create the induced `ClassStateSpace` by the given `ProblemVertexList`.
    /// This function copies the corresponding set of `ClassVertex`
    /// and all `ClassEdge` between those vertices into a new `ClassGraph`.
    ClassStateSpace create_induced_subspace(const ProblemVertexList& vertices) const;
    /// @brief Create the induced `ClassStateSpace` by the given `problem_indices`.
    /// This function copies the corresponding sets of `ClassVertex`
    /// and `ClassEdge` into a new `ClassGraph`.
    ClassStateSpace create_induced_subspace(const IndexList& problem_indices) const;
};

/**
 * Default printing of vertices and edges in a static graph.
 */

extern std::ostream& operator<<(std::ostream& out, const ProblemVertex& vertex);
extern std::ostream& operator<<(std::ostream& out, const ProblemEdge& edge);

extern std::ostream& operator<<(std::ostream& out, const ClassVertex& vertex);
extern std::ostream& operator<<(std::ostream& out, const ClassEdge& edge);
}

#endif
