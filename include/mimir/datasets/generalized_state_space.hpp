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

/**
 * ProblemGraph
 */

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

/**
 * GeneralizedStateSpace
 */

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
    GeneralizedSearchContext m_context;  ///< The search contexts, possibly filtered and sorted.

    ProblemGraphList m_problem_graphs;  ///< The child-level graphs.

    ClassGraph m_graph;  ///< Core data.

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

    const ProblemGraphList& get_problem_graphs() const;

    const ClassGraph& get_graph() const;

    const IndexSet& get_initial_vertices() const;
    const IndexSet& get_goal_vertices() const;
    const IndexSet& get_unsolvable_vertices() const;

    /// @brief Ground `Class` related structures to `Problem` related structures
    /// to access detailed problem specific information about the state.
    const ProblemGraph& get_problem_graph(const ClassVertex& vertex) const;
    const ProblemGraph& get_problem_graph(const ClassEdge& edge) const;
    const ProblemVertex& get_problem_vertex(const ClassVertex& vertex) const;
    const ProblemEdge& get_problem_edge(const ClassEdge& edge) const;

    /// @brief Lift `Problem` related structures to `Class` related structures.
    const ClassVertex& get_class_vertex(const ProblemVertex& vertex) const;
    const ClassEdge& get_class_edge(const ProblemEdge& edge) const;

    /**
     * Construct subgraphs for learning from fragments of the `ClassGraph`.
     */

    /// @brief Create the induced `ClassGraph` by the given `class_vertex_indices`.
    /// This function copies the set of `ClassVertex`
    /// and all `ClassEdge` between those vertices into a new `ClassGraph`.
    ClassGraph create_induced_subgraph_from_class_vertex_indices(const IndexList& class_vertex_indices) const;
    /// @brief Create the induced `ClassGraph` by the given `problem_indices`.
    /// This function copies the corresponding sets of `ClassVertex`
    /// and `ClassEdge` into a new `ClassGraph`.
    ClassGraph create_induced_subgraph_from_problem_indices(const IndexList& problem_indices) const;
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
