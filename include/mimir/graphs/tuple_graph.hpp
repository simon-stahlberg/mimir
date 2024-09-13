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

#ifndef MIMIR_GRAPHS_TUPLE_GRAPH_HPP_
#define MIMIR_GRAPHS_TUPLE_GRAPH_HPP_

#include "mimir/common/grouped_vector.hpp"
#include "mimir/datasets/state_space.hpp"
#include "mimir/graphs/digraph.hpp"
#include "mimir/search/algorithms/iw/dynamic_novelty_table.hpp"
#include "mimir/search/algorithms/iw/tuple_index_generators.hpp"
#include "mimir/search/algorithms/iw/tuple_index_mapper.hpp"
#include "mimir/search/state.hpp"

#include <ostream>
#include <vector>

namespace mimir
{
/**
 * Implementation of TupleGraphs (Lipovetzky and Geffner ECAI2012)
 * Source: https://www-i6.informatik.rwth-aachen.de/~hector.geffner/www.dtic.upf.edu/~hgeffner/width-ecai-2012.pdf
 */

class TupleGraphArityZeroComputation;
class TupleGraphArityKComputation;

using TupleVertexIndex = int;
using TupleVertexIndexList = std::vector<TupleVertexIndex>;

class TupleGraphVertex
{
private:
    TupleVertexIndex m_index;
    TupleIndex m_tuple_index;
    StateList m_states;

public:
    TupleGraphVertex(TupleVertexIndex identifier, TupleIndex tuple_index, StateList states);

    /**
     * Getters.
     */

    TupleVertexIndex get_index() const;
    TupleIndex get_tuple_index() const;
    const StateList& get_states() const;
};

using TupleGraphVertexList = std::vector<TupleGraphVertex>;

class TupleGraph
{
private:
    std::shared_ptr<StateSpace> m_state_space;
    std::shared_ptr<TupleIndexMapper> m_tuple_index_mapper;
    bool m_prune_dominated_tuples;

    StaticDigraph m_digraph;
    IndexGroupedVector<const TupleGraphVertex> m_vertices_grouped_by_distance;
    IndexGroupedVector<const State> m_states_grouped_by_distance;

    TupleGraph(std::shared_ptr<StateSpace> state_space,
               std::shared_ptr<TupleIndexMapper> tuple_index_mapper,
               bool prune_dominated_tuples,
               StaticDigraph digraph,
               IndexGroupedVector<const TupleGraphVertex> vertices_grouped_by_distance,
               IndexGroupedVector<const State> states_grouped_by_distance);

    friend class TupleGraphArityZeroComputation;
    friend class TupleGraphArityKComputation;

public:
    /// @brief Compute and return an admissible chain for a given tuple of ground atoms.
    /// Return std::nullopt if no such admissible chain exists.
    std::optional<TupleVertexIndexList> compute_admissible_chain(const GroundAtomList<Fluent>& fluent_atoms);

    /// @brief Compute and return an admissible chain for a given set of states.
    /// Return std::nullopt if no such admissible chain exists.
    std::optional<TupleVertexIndexList> compute_admissible_chain(const StateList& states);

    /**
     * Getters.
     */

    const std::shared_ptr<StateSpace>& get_state_space() const;
    const std::shared_ptr<TupleIndexMapper>& get_tuple_index_mapper() const;
    State get_root_state() const;
    const StaticDigraph& get_digraph() const;
    const IndexGroupedVector<const TupleGraphVertex>& get_vertices_grouped_by_distance() const;
    const IndexGroupedVector<const State>& get_states_grouped_by_distance() const;
};

using TupleGraphList = std::vector<TupleGraph>;

class TupleGraphFactory
{
private:
    std::shared_ptr<StateSpace> m_state_space;
    std::shared_ptr<TupleIndexMapper> m_tuple_index_mapper;
    bool m_prune_dominated_tuples;

    /// @brief Create tuple graph for the special case of width 0, i.e.,
    /// any state with distance at most 1 from the root_state is a subgoal state.
    TupleGraph create_for_arity_zero(State root_state);

    /// @brief Create a tuple graph for width k > 0.
    TupleGraph create_for_arity_k(State root_state);

public:
    TupleGraphFactory(std::shared_ptr<StateSpace> state_space, int arity, bool prune_dominated_tuples = false);

    /// @brief Create and return the tuple graph.
    TupleGraph create(State root_state);

    /**
     * Getters.
     */

    const std::shared_ptr<StateSpace>& get_state_space() const;
    const std::shared_ptr<TupleIndexMapper>& get_tuple_index_mapper() const;
};

/**
 * Pretty printing as dot representation
 */

extern std::ostream& operator<<(std::ostream& out, const TupleGraph& tuple_graph);

}

#endif