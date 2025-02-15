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

#ifndef MIMIR_DATASETS_PROBLEM_CLASS_GRAPH_HPP_
#define MIMIR_DATASETS_PROBLEM_CLASS_GRAPH_HPP_

#include "mimir/common/types.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/graphs/graph_edges.hpp"
#include "mimir/graphs/graph_vertices.hpp"
#include "mimir/graphs/static_graph.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir::datasets
{

/**
 * Problem
 */

struct ProblemOptions
{
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

/// @brief `ProblemVertex` encapsulates information about a state in the context of a `ProblemClassGraph`.
/// The `Index` is the index of the corresponding `ClassVertex` in the `ProblemClassGraph`.
/// The `State` is the underlying planning state.
using ProblemVertex = Vertex<Index, State>;

inline Index get_global_state_index(const ProblemVertex& vertex) { return vertex.get_property<0>(); }

inline State get_state(const ProblemVertex& vertex) { return vertex.get_property<1>(); }

/// @brief `ProblemEdge` encapsulates information about a state transition in the context of a `ProblemClassGraph`.
/// The `Index` is the index of the corresponding `ClassEdge` in the `ProblemClassGraph`.
/// The `GroundAction` is the underlying ground action.
using ProblemEdge = Edge<GroundAction>;

inline GroundAction get_action(const ProblemEdge& edge) { return edge.get_property<0>(); }

using StaticProblemGraph = StaticGraph<ProblemVertex, ProblemEdge>;
using ProblemGraph = StaticBidirectionalGraph<StaticProblemGraph>;
using ProblemGraphList = std::vector<ProblemGraph>;

/**
 * Class is a graph on top of problems.
 *
 * For simplicity each class vertex (resp. edge) contains a copy of a problem vertex (resp. edge).
 */

struct ClassOptions
{
    bool symmetry_pruning = true;
    bool sort_ascending_by_num_states = true;

    ProblemOptions options = ProblemOptions();
};

/// @brief `ClassVertex` encapsulates information about a problem vertex where
/// the first `Index` is the index to the `ProblemGraph` in the problem graphs of the `ProblemClassGraph`
/// the second `Index` is the index of the `ProblemVertex` in the `ProblemGraph`
using ClassVertex = Vertex<Index, Index>;

inline Index get_problem_index(const ClassVertex& vertex) { return vertex.get_property<0>(); }

inline Index get_problem_vertex_index(const ClassVertex& vertex) { return vertex.get_property<1>(); }

/// @brief `ClassEdge` encapsulates information about a problem edge where
/// the first `Index` is the index to the `ProblemGraph` in the problem graphs of the `ProblemClassGraph`
/// the second `Index` is the index of the `ProblemEdge` in the `ProblemGraph`.
using ClassEdge = Edge<Index, Index>;

inline Index get_problem_index(const ClassEdge& edge) { return edge.get_property<0>(); }

inline Index get_problem_vertex_index(const ClassEdge& edge) { return edge.get_property<1>(); }

using StaticClassGraph = StaticGraph<ClassVertex, ClassEdge>;
using ClassGraph = StaticBidirectionalGraph<StaticClassGraph>;

/**
 * Class
 */

class ProblemClassGraph
{
private:
    ProblemGraphList m_problem_graphs;

public:
    ProblemClassGraph(const ProblemContextList& contexts, const ClassOptions& options = ClassOptions());
};

class GlobalTupleGraph
{
private:
public:
    /// @brief Create a `TupleGraph` for the `vertex` in the `graph`.
    /// @param graph
    /// @param vertex
    GlobalTupleGraph(const ProblemGraph& graph, Index vertex);
};
}

#endif
