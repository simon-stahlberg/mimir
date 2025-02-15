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

using ProblemVertex = Vertex<State>;

inline State get_state(const ProblemVertex& vertex) { return vertex.get_property<0>(); }

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

using ClassVertex = Vertex<ProblemVertex>;

inline const ProblemVertex& get_problem_vertex(const ClassVertex& vertex) { return vertex.get_property<0>(); }

using ClassEdge = Edge<ProblemEdge>;

inline const ProblemEdge& get_problem_vertex(const ClassEdge& edge) { return edge.get_property<0>(); }

using StaticClassGraph = StaticGraph<ClassVertex, ClassEdge>;
using ClassGraph = StaticBidirectionalGraph<StaticClassGraph>;

/**
 * Class
 */

class ProblemClassGraph
{
private:
    ProblemGraphList m_problem_graphs;

    ClassGraph m_class_graph;

public:
    ProblemClassGraph(const ProblemContextList& contexts, const ClassOptions& options = ClassOptions());
};
}

#endif
