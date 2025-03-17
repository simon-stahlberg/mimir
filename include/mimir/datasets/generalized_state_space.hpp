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

namespace mimir::graphs
{

/**
 * ProblemGraph
 */

/// @typedef ProblemVertex
/// @brief `ProblemVertex` encapsulates information about a vertex in a `ProblemGraph`
using ProblemVertex = Vertex<Index, search::State>;
using ProblemVertexList = std::vector<ProblemVertex>;

/// @brief Get the index of the corresponding `ClassVertex` in the `GeneralizedStateSpace`.
/// @param vertex is a `ProblemVertex`.
/// @return the index of the corresponding `ClassVertex` in the `GeneralizedStateSpace`.
inline Index get_class_vertex_index(const ProblemVertex& vertex) { return vertex.get_property<0>(); }

/// @brief Get the `State` of the given `ProblemVertex`.
/// @param vertex is a `ProblemVertex`.
/// @return the `State` of the given `ProblemVertex` in the `ProblemGraph`.
inline search::State get_state(const ProblemVertex& vertex) { return vertex.get_property<1>(); }

/// @typedef ProblemEdge
/// @brief `ProblemEdge` encapsulates information about an edge in a `ProblemGraph`.
using ProblemEdge = Edge<Index, formalism::GroundAction, ContinuousCost>;
using ProblemEdgeList = std::vector<ProblemEdge>;

/// @brief Get the index of the corresponding `ClassEdge` in the `GeneralizedStateSpace`.
/// @param edge is a `ProblemEdge`.
/// @return the index of the corresponding `ClassEdge` in the `GeneralizedStateSpace`.
inline Index get_class_edge_index(const ProblemEdge& edge) { return edge.get_property<0>(); }

/// @brief Get the `GroundAction` of the given `ProblemEdge`.
/// @param edge is a `ProblemEdge`.
/// @return the `GroundAction` of the given `ProblemEdge` in the `ProblemGraph`.
inline formalism::GroundAction get_action(const ProblemEdge& edge) { return edge.get_property<1>(); }

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
/// @brief `ClassGraph` implements a directed graph representing the state space of a class of problems.
using ClassGraph = StaticBidirectionalGraph<StaticClassGraph>;

/// @brief Write a string representation of the given `ProblemVertex` to the `std::ostream` buffer.
/// @param out is a reference to the given `std::ostream` buffer.
/// @param vertex is the given `ProblemVertex`.
/// @return is a reference to the given `std::ostream` buffer.
extern std::ostream& operator<<(std::ostream& out, const ProblemVertex& vertex);

/// @brief Write a string representation of the given `ProblemEdge` to the `std::ostream` buffer.
/// @param out is a reference to the given `std::ostream` buffer.
/// @param vertex is the given `ProblemEdge`.
/// @return is a reference to the given `std::ostream` buffer.
extern std::ostream& operator<<(std::ostream& out, const ProblemEdge& edge);

/// @brief Write a string representation of the given `ClassVertex` to the `std::ostream` buffer.
/// @param out is a reference to the given `std::ostream` buffer.
/// @param vertex is the given `ClassVertex`.
/// @return is a reference to the given `std::ostream` buffer.
extern std::ostream& operator<<(std::ostream& out, const ClassVertex& vertex);

/// @brief Write a string representation of the given `ClassEdge` to the `std::ostream` buffer.
/// @param out is a reference to the given `std::ostream` buffer.
/// @param vertex is the given `ClassEdge`.
/// @return is a reference to the given `std::ostream` buffer.
extern std::ostream& operator<<(std::ostream& out, const ClassEdge& edge);
}

namespace mimir::datasets
{
/**
 * GeneralizedStateSpace
 */

/// @brief `GeneralizedStateSpace` encapsulates a two-level graph structure to represent the state transition model of a class of problems.
///
/// The central features of this implementation are:
/// - the direct applicability of a large collection of Boost BGL graph algorithms on these graphs, making it convenient to analyze state transition models.
/// - the availability of symmetry reduction techniques (using graph isomorphism) and expressivity analysis techniques (using state graphs and the k-WL
/// algorithm or approximate techniques).
///
/// The two-level graph structure consists of two types of graphs: `graphs::ClassGraph` and `graphs::ProblemGraph`.
/// On the high level, we have a single `graphs::ClassGraph` that represents the state transition model of the class of problems and
/// on the low level, we have several `graphs::ProblemGraph` each representing the state transition model of a specfic problem in the class.
///
/// The `graphs::ClassGraph` acts as the main interface to retrieve information about the state transition model of a class of problems.
/// Each vertex in the `graphs::ClassGraph` has a single representative from a `graphs::ProblemGraph`.
/// When symmetry reduction is disabled, there is a strict one-to-one mapping between `graphs::ClassVertex` and `graphs::ProblemVertex`
/// and a strict one-to-one mapping between `graphs::ClassEdge` and `graphs::ProblemEdge`.
/// When symmetry reduction is enabled, the mapping may become a strict one-to-many mapping, in which case, the `graphs::ClassVertex`
/// or `graphs::ClassEdge` acts as an interface to pick access a unique representative.
///
/// The `graphs::ProblemGraph` acts as the main interface to retrieve information about the state transition model of a specific problem.
/// When symmetry reduction is disabled, the `graphs::ProblemGraph` is essentially the state space.
/// When symmetry reduction is enabled, the `graphs::ProblemGraph` becomes the equivalence-based abstraction.
///
/// The `GeneralizedStateSpace` provides several convenience functions to map between `graphs::ClassVertex` (resp. `graphs::ClassEdge`) and
/// `graphs::ProblemVertex` (resp. `graphs::ProblemEdge`). Furthermore, there is functionality to compute induced subgraphs, which is useful in learning tasks
/// where learning from the whole `graphs::ClassGraph` is inconvenient or simply not scalable. Computing a subgraph is fairly efficient and the preferred way
/// when working with fragments of the `GeneralizedStateSpace`.
class GeneralizedStateSpace
{
private:
    search::GeneralizedSearchContext m_context;  ///< The search contexts, possibly filtered and sorted.

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

    /// @brief Construct a `GeneralizedStateSpace` from the given `search::GeneralizedSearchContext` and the `Options`.
    /// Note that the constructor may internally prune irrelevant `search::SearchContext` if detected by symmetry reduction.
    /// Note also that the ordering of `search::SearchContext` might change if ordering by number of states is enabled.
    /// The final `search::GeneralizedSearchContext` can be accessed through the respective getter function.
    /// @param context encapsulates the problem specific `search::SearchContext` used to derive the `GeneralizedStateSpace`.
    /// @param options encapsulates the options used to derive the `GeneralizedStateSpace`.
    GeneralizedStateSpace(search::GeneralizedSearchContext context, const Options& options = Options());

    /**
     * Getters
     */

    /// @brief Get the underlying `search::GeneralizedSearchContext`.
    /// @return the underlying `search::GeneralizedSearchContext`.
    const search::GeneralizedSearchContext& get_generalized_search_context() const;

    /// @brief Get the underlying `graphs::ProblemGraphList`
    /// @return the underlying `graphs::ProblemGraphList`.
    const graphs::ProblemGraphList& get_problem_graphs() const;

    /// @brief Get the underlying `graphs::ClassGraph`
    /// @return the underlying `graphs::ClassGraph`.
    const graphs::ClassGraph& get_graph() const;

    /// @brief Get the `mimir::IndexSet` of the `graphs::ClassVertex` that are marked as initial vertices.
    /// @return an `mimir::IndexSet` that identifies the set of initial `graphs::ClassVertex`.
    const IndexSet& get_initial_vertices() const;
    /// @brief Get the `mimir::IndexSet` of the `graphs::ClassVertex` that are marked as goal vertices.
    /// @return an `mimir::IndexSet` that identifies the set of goal `graphs::ClassVertex`.
    const IndexSet& get_goal_vertices() const;
    /// @brief Get the `mimir::IndexSet` of the `graphs::ClassVertex` that are marked as unsolvable vertices.
    /// @return an `mimir::IndexSet` that identifies the set of unsolvable `graphs::ClassVertex`.
    const IndexSet& get_unsolvable_vertices() const;

    /// @brief Get the representative `ProblemGraph` of the given `graphs::ClassVertex`.
    /// @param vertex is the `graphs::ClassVertex`.
    /// @return the representative `ProblemGraph` of the given `graphs::ClassVertex`.
    const graphs::ProblemGraph& get_problem_graph(const graphs::ClassVertex& vertex) const;
    /// @brief Get the representative `ProblemGraph` of the given `graphs::ClassEdge`.
    /// @param vertex is the `graphs::ClassEdge`.
    /// @return the representative `ProblemGraph` of the given `graphs::ClassEdge`.
    const graphs::ProblemGraph& get_problem_graph(const graphs::ClassEdge& edge) const;
    /// @brief Get the representative `graphs::ProblemVertex` of the given `graphs::ClassVertex`.
    /// @param vertex is the `graphs::ClassVertex`.
    /// @return the representative `graphs::ProblemVertex` of the given `graphs::ClassVertex`.
    const graphs::ProblemVertex& get_problem_vertex(const graphs::ClassVertex& vertex) const;
    /// @brief Get the representative `graphs::ProblemEdge` of the given `graphs::ClassEdge`.
    /// @param vertex is the `graphs::ClassEdge`.
    /// @return the representative `graphs::ProblemEdge` of the given `graphs::ClassEdge`.
    const graphs::ProblemEdge& get_problem_edge(const graphs::ClassEdge& edge) const;
    /// @brief Get the representative `Problem` of the given `graphs::ClassVertex`.
    /// @param vertex is the `graphs::ClassVertex`.
    /// @return the representative `formalism::Problem` of the given `graphs::ClassVertex`.
    const formalism::Problem& get_problem(const graphs::ClassVertex& vertex) const;
    /// @brief Get the representative `Problem` of the given `graphs::ClassEdge`.
    /// @param vertex is the `graphs::ClassEdge`.
    /// @return the representative `formalism::Problem` of the given `graphs::ClassEdge`.
    const formalism::Problem& get_problem(const graphs::ClassEdge& edge) const;

    /// @brief Lift `graphs::ProblemVertex` back to its corresponding `graphs::ClassVertex`.
    /// Note that other `graphs::ProblemVertex` may map to the same `graphs::ClassVertex`.
    /// @param vertex is the `graphs::ProblemVertex`
    /// @return the corresponding `graphs::ClassVertex`.
    const graphs::ClassVertex& get_class_vertex(const graphs::ProblemVertex& vertex) const;
    /// @brief Lift `graphs::ProblemEdge` back to its corresponding `graphs::ClassEdge`.
    /// Note that other `graphs::ProblemEdge` may map to the same `graphs::ClassEdge`.
    /// @param vertex is the `graphs::ProblemEdge`
    /// @return the corresponding `graphs::ClassEdge`.
    const graphs::ClassEdge& get_class_edge(const graphs::ProblemEdge& edge) const;

    /**
     * Construct subgraphs for learning from fragments of the `ClassGraph`.
     */

    /// @brief Create the induced `ClassGraph` by the given `class_vertex_indices`.
    /// This function copies the set of `graphs::ClassVertex`
    /// and all `graphs::ClassEdge` between those vertices into a new `ClassGraph`.
    graphs::ClassGraph create_induced_subgraph_from_class_vertex_indices(const IndexList& class_vertex_indices) const;
    /// @brief Create the induced `ClassGraph` by the given `problem_indices`.
    /// This function copies the corresponding sets of `graphs::ClassVertex`
    /// and `graphs::ClassEdge` into a new `ClassGraph`.
    graphs::ClassGraph create_induced_subgraph_from_problem_indices(const IndexList& problem_indices) const;
};
}

#endif
