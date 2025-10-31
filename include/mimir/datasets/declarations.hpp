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

#ifndef MIMIR_DATASETS_DECLARATIONS_HPP_
#define MIMIR_DATASETS_DECLARATIONS_HPP_

// Do not include headers with transitive dependencies.
#include "mimir/graphs/algorithms/nauty.hpp"
#include "mimir/graphs/graph_edges.hpp"
#include "mimir/graphs/graph_vertices.hpp"
#include "mimir/graphs/property.hpp"
#include "mimir/graphs/static_graph.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/state.hpp"

#include <cstdint>
#include <vector>

namespace mimir::graphs
{
/// @typedef ProblemVertex
/// @brief `ProblemVertex` encapsulates information about a vertex in a `ProblemGraph`
using ProblemVertex = Vertex<std::tuple<search::PackedState, search::StateRepository, DiscreteCost, ContinuousCost, bool, bool, bool, bool>>;
using ProblemVertexList = std::vector<ProblemVertex>;

/// @typedef ProblemEdge
/// @brief `ProblemEdge` encapsulates information about an edge in a `ProblemGraph`.
using ProblemEdge = Edge<std::tuple<formalism::GroundAction, formalism::Problem, ContinuousCost>>;
using ProblemEdgeList = std::vector<ProblemEdge>;

using StaticProblemGraph = StaticGraph<ProblemVertex, ProblemEdge>;
using StaticProblemGraphList = std::vector<StaticProblemGraph>;
/// @typedef ProblemGraph
/// @brief `ProblemGraph` implements a directed graph representing the state space of a single problem.
using ProblemGraph = StaticBidirectionalGraph<StaticProblemGraph>;
using ProblemGraphList = std::vector<ProblemGraph>;

/// @typedef ClassVertex
/// @brief `ClassVertex` encapsulates information about a vertex in a `ClassGraph`.
using ClassVertex = Vertex<std::tuple<Index, Index>>;
using ClassVertexList = std::vector<ClassVertex>;

/// @typedef ClassEdge
/// @brief `ClassEdge` encapsulates information about an edge in a `ClassGraph`.
using ClassEdge = Edge<std::tuple<Index, Index>>;
using ClassEdgeList = std::vector<ClassEdge>;

using StaticClassGraph = StaticGraph<ClassVertex, ClassEdge>;
/// @typedef ClassGraph
/// @brief `ClassGraph` implements a directed graph representing the state space of a class of problems.
using ClassGraph = StaticBidirectionalGraph<StaticClassGraph>;

/// @brief `TupleGraphVertex` encapsulates information about a vertex in a tuple graph.
using TupleGraphVertex = Vertex<std::tuple<search::iw::AtomIndexList, IndexList>>;
using TupleGraphVertexList = std::vector<TupleGraphVertex>;

using StaticTupleGraph = StaticGraph<TupleGraphVertex, Edge<>>;
using InternalTupleGraph = StaticBidirectionalGraph<StaticTupleGraph>;
}

namespace mimir::datasets
{
class KnowledgeBaseImpl;
using KnowledgeBase = std::shared_ptr<KnowledgeBaseImpl>;

class StateSpaceImpl;
using StateSpace = std::shared_ptr<StateSpaceImpl>;
using StateSpaceList = std::vector<StateSpace>;

class StateSpaceSamplerImpl;
using StateSpaceSampler = std::shared_ptr<StateSpaceSamplerImpl>;

class GeneralizedStateSpaceImpl;
using GeneralizedStateSpace = std::shared_ptr<GeneralizedStateSpaceImpl>;

class TupleGraphImpl;
using TupleGraph = std::shared_ptr<TupleGraphImpl>;
using TupleGraphList = std::vector<TupleGraph>;
}

#endif
