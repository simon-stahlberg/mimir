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
#include "mimir/datasets/tuple_graph/internal_tuple_graph.hpp"
#include "mimir/datasets/tuple_graph/options.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/graphs/static_graph.hpp"
#include "mimir/search/algorithms/iw/novelty_table.hpp"
#include "mimir/search/algorithms/iw/tuple_index_generators.hpp"
#include "mimir/search/algorithms/iw/tuple_index_mapper.hpp"

#include <ostream>
#include <vector>

/**
 * Implementation of TupleGraphs (Lipovetzky and Geffner ECAI2012)
 * Source: https://www-i6.informatik.rwth-aachen.de/~hector.geffner/www.dtic.upf.edu/~hgeffner/width-ecai-2012.pdf
 */

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
    using Options = tuple_graph::Options;

    TupleGraphImpl(StateSpace state_space,
                   graphs::InternalTupleGraph graph,
                   IndexGroupedVector<const Index> vertices_grouped_by_distance,
                   IndexGroupedVector<const Index> problem_vertices_grouped_by_distance);

    /// @brief Create the `TupleGraph` for each vertex in the given `StateSpace`.
    /// @param state_space is the `StateSpace`
    /// @return are the `TupleGraph` for each vertex in the given `StateSpace`.
    static TupleGraphList create(StateSpace state_space, std::optional<CertificateMaps>& certificate_maps, const Options& options = Options());

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