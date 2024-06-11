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

#include "mimir/datasets/state_space.hpp"
#include "mimir/search/algorithms/iw/dynamic_novelty_table.hpp"
#include "mimir/search/algorithms/iw/index_mappers.hpp"
#include "mimir/search/algorithms/iw/tuple_index_generators.hpp"
#include "mimir/search/state.hpp"

#include <ostream>
#include <vector>

namespace mimir
{
class TupleGraphFactory;

class TupleGraphVertex
{
private:
    int m_identifier;
    int m_tuple_index;
    StateList m_states;

public:
    TupleGraphVertex(int identifier, int tuple_index, StateList states);

    /**
     * Getters.
     */

    int get_identifier() const;
    int get_tuple_index() const;
    const StateList& get_states() const;
};

using TupleGraphVertexList = std::vector<TupleGraphVertex>;

class TupleGraph
{
private:
    std::shared_ptr<StateSpaceImpl> m_state_space;
    std::shared_ptr<TupleIndexMapper> m_tuple_index_mapper;
    State m_root_state;

    TupleGraphVertexList m_vertices;

    std::vector<std::vector<int>> m_forward_successors;
    std::vector<std::vector<int>> m_backward_successors;

    std::vector<std::vector<int>> m_vertex_indices_by_distance;
    std::vector<StateList> m_states_by_distance;

    TupleGraph(std::shared_ptr<StateSpaceImpl> state_space,
               std::shared_ptr<TupleIndexMapper> tuple_index_mapper,
               State root_state,
               TupleGraphVertexList vertices,
               std::vector<std::vector<int>> forward_successors,
               std::vector<std::vector<int>> backward_successors,
               std::vector<std::vector<int>> vertex_indices_by_distance,
               std::vector<StateList> states_by_distance);
    friend class TupleGraphFactory;

public:
    /**
     * Getters.
     */

    std::shared_ptr<StateSpaceImpl> get_state_space() const;
    std::shared_ptr<TupleIndexMapper> get_tuple_index_mapper() const;
    State get_root_state() const;
    const TupleGraphVertexList& get_vertices() const;
    const std::vector<std::vector<int>>& get_forward_successors() const;
    const std::vector<std::vector<int>>& get_backward_successors() const;
    const std::vector<std::vector<int>>& get_vertex_indices_by_distances() const;
    const std::vector<StateList>& get_states_by_distance() const;
};

class TupleGraphFactory
{
private:
    std::shared_ptr<StateSpaceImpl> m_state_space;
    std::shared_ptr<TupleIndexMapper> m_tuple_index_mapper;

    /// @brief Create tuple graph for the special case of width 0, i.e.,
    /// any state with distance at most 1 from the root_state is a subgoal state.
    TupleGraph create_for_arity_zero(const State root_state);

    TupleGraph create_for_arity_k(const State root_state);

public:
    TupleGraphFactory(std::shared_ptr<StateSpaceImpl> state_space, int arity);

    TupleGraph create(const State root_state);

    /**
     * Getters.
     */

    std::shared_ptr<StateSpaceImpl> get_state_space() const;
    std::shared_ptr<TupleIndexMapper> get_tuple_index_mapper() const;
};

/**
 * Pretty printing as dot representation
 */

std::ostream& operator<<(std::ostream& out, const TupleGraph& tuple_graph);

}

#endif