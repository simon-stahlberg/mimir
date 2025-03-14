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

#ifndef MIMIR_DATASETS_GENERALIZED_STATE_SPACE_HPP_
#define MIMIR_DATASETS_GENERALIZED_STATE_SPACE_HPP_

#include "mimir/common/types.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/generalized_problem.hpp"
#include "mimir/graphs/declarations.hpp"
#include "mimir/graphs/graph_edges.hpp"
#include "mimir/graphs/graph_vertices.hpp"
#include "mimir/graphs/static_graph.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/generalized_search_context.hpp"

namespace mimir
{
namespace graphs
{

/**
 * ProblemGraph
 */

/// @typedef ProblemVertex
/// @brief `ProblemVertex` encapsulates information about a vertex in a `ProblemGraph`
using ProblemVertex = Vertex<Index, State>;
using ProblemVertexList = std::vector<ProblemVertex>;

/// @brief Get the index of the corresponding `ClassVertex` in the `GeneralizedStateSpace`.
/// @param vertex is a `ProblemVertex`.
/// @return the index of the corresponding `ClassVertex` in the `GeneralizedStateSpace`.
inline Index get_class_vertex_index(const ProblemVertex& vertex) { return vertex.get_property<0>(); }

/// @brief Get the `State` of the given `ProblemVertex`.
/// @param vertex is a `ProblemVertex`.
/// @return the `State` of the given `ProblemVertex` in the `ProblemGraph`.
inline State get_state(const ProblemVertex& vertex) { return vertex.get_property<1>(); }

/// @typedef ProblemEdge
/// @brief `ProblemEdge` encapsulates information about an edge in a `ProblemGraph`.
using ProblemEdge = Edge<Index, GroundAction, ContinuousCost>;
using ProblemEdgeList = std::vector<ProblemEdge>;

/// @brief Get the index of the corresponding `ClassEdge` in the `GeneralizedStateSpace`.
/// @param edge is a `ProblemEdge`.
/// @return the index of the corresponding `ClassEdge` in the `GeneralizedStateSpace`.
inline Index get_class_edge_index(const ProblemEdge& edge) { return edge.get_property<0>(); }

/// @brief Get the `GroundAction` of the given `ProblemEdge`.
/// @param edge is a `ProblemEdge`.
/// @return the `GroundAction` of the given `ProblemEdge` in the `ProblemGraph`.
inline GroundAction get_action(const ProblemEdge& edge) { return edge.get_property<1>(); }

/// @brief Get the action cost associated with the `GroundAction` of the given `ProblemEdge`
/// @param edge is a `ProblemEdge`.
/// @return the action cost of associated with the `GroundAction` of the given `ProblemEdge` in the `ProblemGraph`.
inline ContinuousCost get_action_cost(const ProblemEdge& edge) { return edge.get_property<2>(); }

using StaticProblemGraph = StaticGraph<ProblemVertex, ProblemEdge>;
using StaticProblemGraphList = std::vector<StaticProblemGraph>;
/// @typedef ProblemGraph
/// @brief `ProblemGraph` implements a directed graph representing the state space of a single problem.
using ProblemGraph = StaticBidirectionalGraph<StaticProblemGraph>;
using ProblemGraphList = std::vector<ProblemGraph>;

/**
 * ClassGraph
 */

/// @typedef ClassVertex
/// @brief `ClassVertex` encapsulates information about a vertex in a `ClassGraph`.
using ClassVertex = Vertex<Index, Index, Index, DiscreteCost, ContinuousCost, bool, bool, bool, bool>;
using ClassVertexList = std::vector<ClassVertex>;

/// @brief Get the index of the given `ClassVertex` in the `GeneralizedStateSpace`.
/// @param vertex is a `ClassVertex`.
/// @return the index of the given `ClassVertex` in the `GeneralizedStateSpace`.
inline Index get_class_vertex_index(const ClassVertex& vertex) { return vertex.get_property<0>(); }

/// @brief Get the index of the `ProblemGraph` associated with the given `ClassVertex` in the `GeneralizedStateSpace`.
/// @param vertex is a `ClassVertex`.
/// @return the index of the `ProblemGraph` associated with the given `ClassVertex` in the `GeneralizedStateSpace`.
inline Index get_problem_index(const ClassVertex& vertex) { return vertex.get_property<1>(); }

/// @brief Get the index of the representative `ProblemVertex` in the `ProblemGraph` associated with the given `ClassVertex` in the `GeneralizedStateSpace`.
/// @param vertex is a `ClassVertex`.
/// @return the index of the representative `ProblemVertex` in the `ProblemGraph` associated with the given `ClassVertex` in the `GeneralizedStateSpace`.
inline Index get_problem_vertex_index(const ClassVertex& vertex) { return vertex.get_property<2>(); }

/// @brief Get the unit goal distance of the given `ClassVertex`.
/// @param vertex is a `ClassVertex`.
/// @return the unit goal distance of the given `ClassVertex`.
inline DiscreteCost get_unit_goal_distance(const ClassVertex& vertex) { return vertex.get_property<3>(); }

/// @brief Get the action cost goal distance of the given `ClassVertex`.
/// @param vertex is a `ClassVertex`
/// @return the action cost goal distance of the given `ClassVertex`.
inline ContinuousCost get_action_goal_distance(const ClassVertex& vertex) { return vertex.get_property<4>(); }

/// @brief Get information whether the representative associated with the given `ClassVertex` is an initial vertex.
/// @param vertex is a `ClassVertex`.
/// @return true if the representative associated with the given `ClassVertex` is an initial vertex, and false otherwise.
inline bool is_initial(const ClassVertex& vertex) { return vertex.get_property<5>(); }

/// @brief Get information whether the representative associated with the given `ClassVertex` is a goal vertex.
/// @param vertex is a `ClassVertex`.
/// @return true if the representative associated with the given `ClassVertex` is a goal vertex, and false otherwise.
inline bool is_goal(const ClassVertex& vertex) { return vertex.get_property<6>(); }

/// @brief Get information whether the representative associated with the given `ClassVertex` is an unsolvable vertex.
/// @param vertex is a `ClassVertex`.
/// @return true if the representative associated with the given `ClassVertex` is an unsolvable vertex, and false otherwise.
inline bool is_unsolvable(const ClassVertex& vertex) { return vertex.get_property<7>(); }

/// @brief Get information whether the representative associated with the given `ClassVertex` is an alive vertex.
/// @param vertex is a `ClassVertex`.
/// @return true if the representative associated with the given `ClassVertex` is an alive vertex, and false otherwise.
inline bool is_alive(const ClassVertex& vertex) { return vertex.get_property<8>(); }

/// @typedef ClassEdge
/// @brief `ClassEdge` encapsulates information about an edge in a `ClassGraph`.
using ClassEdge = Edge<Index, Index, Index, ContinuousCost>;
using ClassEdgeList = std::vector<ClassEdge>;

/// @brief Get the index of the given `ClassEdge` in the `GeneralizedStateSpace`.
/// @param edge is a `ClassEdge`.
/// @return the index of the given `ClassEdge` in the `GeneralizedStateSpace`.
inline Index get_class_edge_index(const ClassEdge& edge) { return edge.get_property<0>(); }

/// @brief Get the index of the `ProblemGraph` associated with the given `ClassEdge` in the `GeneralizedStateSpace`.
/// @param edge is a `ClassEdge`.
/// @return the index of the `ProblemGraph` associated with the given `ClassEdge` in the `GeneralizedStateSpace`.
inline Index get_problem_index(const ClassEdge& edge) { return edge.get_property<1>(); }

/// @brief Get the index of the representative `ProblemEdge` in the `ProblemGraph` associated with the given `ClassEdge` in the `GeneralizedStateSpace`.
/// @param edge is a `ClassEdge`.
/// @return the index of the representative `ProblemEdge` in the `ProblemGraph` associated with the given `ClassEdge` in the `GeneralizedStateSpace`.
inline Index get_problem_edge_index(const ClassEdge& edge) { return edge.get_property<2>(); }

/// @brief Get the action cost associated with the `GroundAction` of the representative `ProblemEdge`.
/// @param edge is a `ClassEdge`.
/// @return the action cost associated with the `GroundAction` of the representative `ProblemEdge`.
inline Index get_action_cost(const ClassEdge& edge) { return edge.get_property<3>(); }

using StaticClassGraph = StaticGraph<ClassVertex, ClassEdge>;
/// @typedef ClassGraph
/// @brief `ClassGraph` implements a directed graph representing the state space of a collection of problems.
using ClassGraph = StaticBidirectionalGraph<StaticClassGraph>;

extern std::ostream& operator<<(std::ostream& out, const ProblemVertex& vertex);
extern std::ostream& operator<<(std::ostream& out, const ProblemEdge& edge);

extern std::ostream& operator<<(std::ostream& out, const ClassVertex& vertex);
extern std::ostream& operator<<(std::ostream& out, const ClassEdge& edge);
}

/**
 * GeneralizedStateSpace
 */

/// @brief `GeneralizedStateSpace` encapsulates a `ProblemGraphList` Q with an additional `ClassStateSpace` structure on top.
///
/// There is a one-to-many mapping from `ClassVertex` (resp. `ClassEdge`) to `ProblemVertex` (resp. `ProblemEdge`),
/// which is a strict one-to-many mapping, if symmetries across different problems are detected.
/// From each `ClassVertex` (resp. `ClassEdge`) one can access the single representative `ProblemVertex` (resp. `ProblemEdge`).
/// From each `ProblemVertex` (resp. `ProblemEdge`) one can access the corresponding `ClassVertex` (resp. `ClassEdge`).
class GeneralizedStateSpace
{
private:
    GeneralizedSearchContext m_context;  ///< The search contexts, possibly filtered and sorted.

    graphs::ProblemGraphList m_problem_graphs;  ///< The child-level graphs.

    graphs::ClassGraph m_graph;  ///< Core data.

    IndexSet m_initial_vertices;
    IndexSet m_goal_vertices;
    IndexSet m_unsolvable_vertices;

public:
    /**
     * Options
     */

    struct Options
    {
        bool sort_ascending_by_num_states;

        struct ProblemSpecific
        {
            bool symmetry_pruning;
            bool mark_true_goal_literals;
            bool remove_if_unsolvable;
            uint32_t max_num_states;
            uint32_t timeout_ms;

            ProblemSpecific() :
                symmetry_pruning(false),
                mark_true_goal_literals(false),
                remove_if_unsolvable(true),
                max_num_states(std::numeric_limits<uint32_t>::max()),
                timeout_ms(std::numeric_limits<uint32_t>::max())
            {
            }
        };

        ProblemSpecific problem_options;

        Options() : sort_ascending_by_num_states(true), problem_options() {}
    };

    /**
     * Constructors
     */

    GeneralizedStateSpace(GeneralizedSearchContext context, const Options& options = Options());

    /**
     * Getters
     */

    const GeneralizedSearchContext& get_generalized_search_context() const;

    const graphs::ProblemGraphList& get_problem_graphs() const;

    const graphs::ClassGraph& get_graph() const;

    const IndexSet& get_initial_vertices() const;
    const IndexSet& get_goal_vertices() const;
    const IndexSet& get_unsolvable_vertices() const;

    /// @brief Ground `Class` related structures to `Problem` related structures
    /// to access detailed problem specific information about the state.
    const graphs::ProblemGraph& get_problem_graph(const graphs::ClassVertex& vertex) const;
    const graphs::ProblemGraph& get_problem_graph(const graphs::ClassEdge& edge) const;
    const graphs::ProblemVertex& get_problem_vertex(const graphs::ClassVertex& vertex) const;
    const graphs::ProblemEdge& get_problem_edge(const graphs::ClassEdge& edge) const;
    const Problem& get_problem(const graphs::ClassVertex& vertex) const;
    const Problem& get_problem(const graphs::ClassEdge& edge) const;

    /// @brief Lift `Problem` related structures to `Class` related structures.
    const graphs::ClassVertex& get_class_vertex(const graphs::ProblemVertex& vertex) const;
    const graphs::ClassEdge& get_class_edge(const graphs::ProblemEdge& edge) const;

    /**
     * Construct subgraphs for learning from fragments of the `ClassGraph`.
     */

    /// @brief Create the induced `ClassGraph` by the given `class_vertex_indices`.
    /// This function copies the set of `ClassVertex`
    /// and all `ClassEdge` between those vertices into a new `ClassGraph`.
    graphs::ClassGraph create_induced_subgraph_from_class_vertex_indices(const IndexList& class_vertex_indices) const;
    /// @brief Create the induced `ClassGraph` by the given `problem_indices`.
    /// This function copies the corresponding sets of `ClassVertex`
    /// and `ClassEdge` into a new `ClassGraph`.
    graphs::ClassGraph create_induced_subgraph_from_problem_indices(const IndexList& problem_indices) const;
};
}

#endif
