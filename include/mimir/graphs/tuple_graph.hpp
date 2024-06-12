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
    bool m_prune_dominated_tuples;

    std::shared_ptr<FluentAndDerivedMapper> m_atom_index_mapper;
    std::shared_ptr<TupleIndexMapper> m_tuple_index_mapper;

    class TupleGraphArityZeroComputation
    {
    private:
        std::shared_ptr<FluentAndDerivedMapper> m_atom_index_mapper;
        std::shared_ptr<TupleIndexMapper> m_tuple_index_mapper;
        std::shared_ptr<StateSpaceImpl> m_state_space;
        State m_root_state;
        bool m_prune_dominated_tuples;
        TupleGraphVertexList m_vertices;
        std::vector<std::vector<int>> m_forward_successors;
        std::vector<std::vector<int>> m_backward_successors;
        std::vector<std::vector<int>> m_vertex_indices_by_distances;
        std::vector<StateList> m_states_by_distance;

    public:
        TupleGraphArityZeroComputation(std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper,
                                       std::shared_ptr<TupleIndexMapper> tuple_index_mapper,
                                       std::shared_ptr<StateSpaceImpl> state_space,
                                       const State root_state,
                                       bool prune_dominated_tuples);

        void compute_root_state_layer();

        void compute_first_layer();

        TupleGraph extract_tuple_graph();
    };

    // Bookkeeping for memory reuse when building tuple graph of width greater 0
    class TupleGraphArityKComputation
    {
    private:
        std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper;
        std::shared_ptr<TupleIndexMapper> tuple_index_mapper;
        std::shared_ptr<StateSpaceImpl> state_space;
        State root_state;
        bool prune_dominated_tuples;
        TupleGraphVertexList vertices;
        std::vector<std::vector<int>> forward_successors;
        std::vector<std::vector<int>> backward_successors;
        std::vector<std::vector<int>> vertex_indices_by_distances;
        std::vector<StateList> states_by_distance;

        // To compute novel tuples for a given state.
        DynamicNoveltyTable novelty_table;

        // To create next state layer
        StateSet visited_states;
        StateList cur_states;

        // Novel tuple indices in next layer
        TupleIndexSet novel_tuple_indices_set;
        // To compute novel tuple indices for a state
        TupleIndexList novel_tuple_indices;

        // To compute extended vertices
        std::unordered_map<TupleIndex, StateSet> cur_tuple_index_to_underlying_extendable_state;
        std::unordered_map<TupleIndex, std::unordered_set<int>> extendable_tuple_index_to_prev_vertices;
        std::unordered_map<State, TupleIndexList, StateHash> state_to_novel_tuple_indices;
        std::unordered_map<TupleIndex, StateSet> novel_tuple_index_to_state_indices;

        // Tuples that make it into vertices
        TupleIndexSet cur_extendable_tuple_indices;

        // Extended vertices
        std::vector<int> cur_vertices;

        void compute_next_state_layer();

        void compute_next_novel_tuple_indices();

        void extend_optimal_plans_from_prev_layer();

        void instantiate_next_layer();

    public:
        TupleGraphArityKComputation(std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper,
                                    std::shared_ptr<TupleIndexMapper> tuple_index_mapper,
                                    std::shared_ptr<StateSpaceImpl> state_space,
                                    const State root_state,
                                    bool prune_dominated_tuples);

        void compute_root_state_layer();

        bool compute_next_layer();

        TupleGraph extract_tuple_graph();
    };

    /// @brief Create tuple graph for the special case of width 0, i.e.,
    /// any state with distance at most 1 from the root_state is a subgoal state.
    TupleGraph create_for_arity_zero(const State root_state);

    TupleGraph create_for_arity_k(const State root_state);

public:
    TupleGraphFactory(std::shared_ptr<StateSpaceImpl> state_space, int arity, bool prune_dominated_tuples = false);

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