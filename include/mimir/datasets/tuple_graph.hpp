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

#ifndef MIMIR_DATASETS_TUPLE_GRAPH_HPP_
#define MIMIR_DATASETS_TUPLE_GRAPH_HPP_

#include "mimir/common/grouped_vector.hpp"
#include "mimir/datasets/declarations.hpp"
#include "mimir/graphs/static_graph.hpp"
#include "mimir/search/algorithms/iw/novelty_table.hpp"
#include "mimir/search/algorithms/iw/tuple_index_generators.hpp"
#include "mimir/search/algorithms/iw/tuple_index_mapper.hpp"
#include "mimir/search/state.hpp"

#include <ostream>
#include <vector>

/**
 * Implementation of TupleGraphs (Lipovetzky and Geffner ECAI2012)
 * Source: https://www-i6.informatik.rwth-aachen.de/~hector.geffner/www.dtic.upf.edu/~hgeffner/width-ecai-2012.pdf
 */

namespace mimir::graphs
{
/// @brief `TupleGraphVertex` encapsulates information about a vertex in a tuple graph.
using TupleGraphVertex = Vertex<search::iw::AtomIndexList, IndexList>;
using TupleGraphVertexList = std::vector<TupleGraphVertex>;

inline const search::iw::AtomIndexList& get_atom_tuple(const TupleGraphVertex& vertex) { return vertex.get_property<0>(); }

inline const IndexList& get_problem_vertices(const TupleGraphVertex& vertex) { return vertex.get_property<1>(); }

using TupleGraphEdge = Edge<>;
using TupleGraphEdgeList = std::vector<TupleGraphEdge>;

using StaticTupleGraph = StaticGraph<TupleGraphVertex, TupleGraphEdge>;
using InternalTupleGraph = StaticBidirectionalGraph<StaticTupleGraph>;
}

namespace mimir::datasets
{

class TupleGraphImpl
{
private:
    StateSpace m_state_space;
    graphs::InternalTupleGraph m_graph;

    IndexGroupedVector<const Index> m_v_idxs_grouped_by_distance;
    IndexGroupedVector<const Index> m_problem_v_idxs_grouped_by_distance;

public:
    TupleGraphImpl(StateSpace state_space,
                   graphs::InternalTupleGraph graph,
                   IndexGroupedVector<const Index> vertices_grouped_by_distance,
                   IndexGroupedVector<const Index> problem_vertices_grouped_by_distance);

    struct Options
    {
        size_t width;
        bool enable_dominance_pruning;

        Options() : width(0), enable_dominance_pruning(true) {}
        Options(size_t width, bool enable_dominance_pruning) : width(width), enable_dominance_pruning(enable_dominance_pruning) {}
    };

    /// @brief Create the `TupleGraph` for each vertex in the given `StateSpace`.
    /// @param state_space is the `StateSpace`
    /// @return are the `TupleGraph` for each vertex in the given `StateSpace`.
    static TupleGraphList create(StateSpace state_space, const Options& options = Options());

    const StateSpace& get_state_space() const;
    const graphs::InternalTupleGraph& get_graph() const;
    const IndexGroupedVector<const Index>& get_vertices_grouped_by_distance() const;
    const IndexGroupedVector<const Index>& get_problem_vertices_grouped_by_distance() const;
};

using TupleGraphList = std::vector<TupleGraph>;

/**
 * Pretty printing as dot representation
 */

extern std::ostream& operator<<(std::ostream& out, const TupleGraphImpl& tuple_graph);
}

#endif