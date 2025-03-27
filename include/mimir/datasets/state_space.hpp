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

#ifndef MIMIR_DATASETS_STATE_SPACE_HPP_
#define MIMIR_DATASETS_STATE_SPACE_HPP_

#include "mimir/common/types.hpp"
#include "mimir/datasets/declarations.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/graphs/algorithms/nauty.hpp"
#include "mimir/graphs/declarations.hpp"
#include "mimir/graphs/graph_edges.hpp"
#include "mimir/graphs/graph_vertices.hpp"
#include "mimir/graphs/static_graph.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/search_context.hpp"

namespace mimir::graphs
{

/**
 * ProblemGraph
 */

/// @typedef ProblemVertex
/// @brief `ProblemVertex` encapsulates information about a vertex in a `ProblemGraph`
using ProblemVertex =
    Vertex<search::State, formalism::Problem, std::shared_ptr<const nauty::Certificate>, DiscreteCost, ContinuousCost, bool, bool, bool, bool>;
using ProblemVertexList = std::vector<ProblemVertex>;

/// @brief Get the `State` of the given `ProblemVertex`.
/// @param vertex is a `ProblemVertex`.
/// @return the `State` of the given `ProblemVertex` in the `ProblemGraph`.
inline search::State get_state(const ProblemVertex& vertex) { return vertex.get_property<0>(); }

/// @brief Get the `formalism::Problem` of the given `ProblemVertex`.
/// @param vertex is a `ProblemVertex`.
/// @return the `formalism::Problem` of the given `ProblemVertex` in the `ProblemGraph`.
inline const formalism::Problem& get_problem(const ProblemVertex& vertex) { return vertex.get_property<1>(); }

/// @brief Get the `nauty::Certificate` of the given `ProblemVertex`.
/// @param vertex is a `ProblemVertex`.
/// @return the `nauty::Certificate` of the given `ProblemVertex` in the `ProblemGraph` if it was computed, and otherwise nullptr.
inline const std::shared_ptr<const nauty::Certificate>& get_certificate(const ProblemVertex& vertex) { return vertex.get_property<2>(); }

/// @brief Get the unit goal distance of the given `ProblemVertex`.
/// @param vertex is a `ProblemVertex`.
/// @return the unit goal distance of the given `ProblemVertex`.
inline DiscreteCost get_unit_goal_distance(const ProblemVertex& vertex) { return vertex.get_property<3>(); }

/// @brief Get the action cost goal distance of the given `ProblemVertex`.
/// @param vertex is a `ProblemVertex`
/// @return the action cost goal distance of the given `ProblemVertex`.
inline ContinuousCost get_action_goal_distance(const ProblemVertex& vertex) { return vertex.get_property<4>(); }

/// @brief Get information whether the representative associated with the given `ProblemVertex` is an initial vertex.
/// @param vertex is a `ProblemVertex`.
/// @return true if the representative associated with the given `ProblemVertex` is an initial vertex, and false otherwise.
inline bool is_initial(const ProblemVertex& vertex) { return vertex.get_property<5>(); }

/// @brief Get information whether the representative associated with the given `ProblemVertex` is a goal vertex.
/// @param vertex is a `ProblemVertex`.
/// @return true if the representative associated with the given `ProblemVertex` is a goal vertex, and false otherwise.
inline bool is_goal(const ProblemVertex& vertex) { return vertex.get_property<6>(); }

/// @brief Get information whether the representative associated with the given `ProblemVertex` is an unsolvable vertex.
/// @param vertex is a `ProblemVertex`.
/// @return true if the representative associated with the given `ProblemVertex` is an unsolvable vertex, and false otherwise.
inline bool is_unsolvable(const ProblemVertex& vertex) { return vertex.get_property<7>(); }

/// @brief Get information whether the representative associated with the given `ProblemVertex` is an alive vertex.
/// @param vertex is a `ProblemVertex`.
/// @return true if the representative associated with the given `ProblemVertex` is an alive vertex, and false otherwise.
inline bool is_alive(const ProblemVertex& vertex) { return vertex.get_property<8>(); }

/// @typedef ProblemEdge
/// @brief `ProblemEdge` encapsulates information about an edge in a `ProblemGraph`.
using ProblemEdge = Edge<formalism::GroundAction, formalism::Problem, ContinuousCost>;
using ProblemEdgeList = std::vector<ProblemEdge>;

/// @brief Get the `GroundAction` of the given `ProblemEdge`.
/// @param edge is a `ProblemEdge`.
/// @return the `GroundAction` of the given `ProblemEdge` in the `ProblemGraph`.
inline formalism::GroundAction get_action(const ProblemEdge& edge) { return edge.get_property<0>(); }

/// @brief Get the `formalism::Problem` of the given `ProblemEdge`.
/// @param vertex is a `ProblemEdge`.
/// @return the `formalism::Problem` of the given `ProblemEdge` in the `ProblemGraph`.
inline const formalism::Problem& get_problem(const ProblemEdge& edge) { return edge.get_property<1>(); }

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

}

namespace mimir::datasets
{
/**
 * StateSpaceImpl
 */

/// @brief `StateSpaceImpl` encapsulates a the state transition model of a problems.
///
/// The central features of this implementation are:
/// - the direct applicability of a large collection of Boost BGL graph algorithms on these graphs, making it convenient to analyze state transition models.
/// - the options to apply symmetry reduction (using graph isomorphism).
class StateSpaceImpl
{
private:
    search::SearchContext m_context;

    graphs::ProblemGraph m_graph;

    Index m_initial_vertex;
    IndexSet m_goal_vertices;
    IndexSet m_unsolvable_vertices;

public:
    StateSpaceImpl(search::SearchContext context, graphs::ProblemGraph graph, Index initial_vertex, IndexSet goal_vertices, IndexSet unsolvable_vertices);

    /**
     * Options
     */

    struct Options
    {
        bool sort_ascending_by_num_states;
        bool symmetry_pruning;
        bool mark_true_goal_literals;
        bool remove_if_unsolvable;
        uint32_t max_num_states;
        uint32_t timeout_ms;

        Options() :
            sort_ascending_by_num_states(true),
            symmetry_pruning(false),
            mark_true_goal_literals(false),
            remove_if_unsolvable(true),
            max_num_states(std::numeric_limits<uint32_t>::max()),
            timeout_ms(std::numeric_limits<uint32_t>::max())
        {
        }
    };

    /**
     * Constructors
     */

    static std::optional<StateSpace> create(search::SearchContext context, const Options& options = Options());

    static StateSpaceList create(search::GeneralizedSearchContext contexts, const Options& options = Options());

    /**
     * Getters
     */

    /// @brief Get the underlying `search::SearchContext`.
    /// @return the underlying `search::SearchContext`.
    const search::SearchContext& get_search_context() const;

    /// @brief Get the underlying `graphs::ProblemGraph`
    /// @return the underlying `graphs::ProblemGraph`.
    const graphs::ProblemGraph& get_graph() const;

    /// @brief Get the `mimir::Index` of the `graphs::ProblemVertex` that is marked as initial vertices.
    /// @return an `mimir::Index` that identifies the initial `graphs::ProblemVertex`.
    Index get_initial_vertex() const;
    /// @brief Get the `mimir::IndexSet` of the `graphs::ProblemVertex` that are marked as goal vertices.
    /// @return an `mimir::IndexSet` that identifies the set of goal `graphs::ProblemVertex`.
    const IndexSet& get_goal_vertices() const;
    /// @brief Get the `mimir::IndexSet` of the `graphs::ProblemVertex` that are marked as unsolvable vertices.
    /// @return an `mimir::IndexSet` that identifies the set of unsolvable `graphs::ProblemVertex`.
    const IndexSet& get_unsolvable_vertices() const;
};

}

#endif
