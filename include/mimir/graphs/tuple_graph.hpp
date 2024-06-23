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
/**
 * Implementation of TupleGraphs (Lipovetzky and Geffner ECAI2012)
 * Source: https://www-i6.informatik.rwth-aachen.de/~hector.geffner/www.dtic.upf.edu/~hgeffner/width-ecai-2012.pdf
 */

class TupleGraphFactory;

using VertexIndex = int;
using VertexIndexList = std::vector<VertexIndex>;
using VertexIndexSet = std::unordered_set<VertexIndex>;

class TupleGraphVertex
{
private:
    VertexIndex m_index;
    TupleIndex m_tuple_index;
    StateList m_states;

public:
    TupleGraphVertex(VertexIndex identifier, TupleIndex tuple_index, StateList states);

    /**
     * Getters.
     */

    VertexIndex get_index() const;
    TupleIndex get_tuple_index() const;
    const StateList& get_states() const;
};

using TupleGraphVertexList = std::vector<TupleGraphVertex>;

class TupleGraph
{
private:
    std::shared_ptr<StateSpace> m_state_space;
    std::shared_ptr<FluentAndDerivedMapper> m_atom_index_mapper;
    std::shared_ptr<TupleIndexMapper> m_tuple_index_mapper;
    bool m_prune_dominated_tuples;

    TupleGraphVertexList m_vertices;

    // TODO: avoid reallocations and return span to relevant parts.
    std::vector<VertexIndexList> m_forward_successors;
    std::vector<VertexIndexList> m_backward_successors;

    std::vector<VertexIndexList> m_vertex_indices_by_distance;
    std::vector<StateList> m_states_by_distance;

    TupleGraph(std::shared_ptr<StateSpace> state_space,
               std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper,
               std::shared_ptr<TupleIndexMapper> tuple_index_mapper,
               bool prune_dominated_tuples);

    friend class TupleGraphFactory;

public:
    /// @brief Compute and return an admissible chain for a given tuple of ground atoms.
    /// Return std::nullopt if no such admissible chain exists.
    std::optional<VertexIndexList> compute_admissible_chain(const GroundAtomList<Fluent>& fluent_atoms, const GroundAtomList<Derived>& derived_atoms);

    /// @brief Compute and return an admissible chain for a given set of states.
    /// Return std::nullopt if no such admissible chain exists.
    std::optional<VertexIndexList> compute_admissible_chain(const StateList& states);

    /**
     * Getters.
     */

    const std::shared_ptr<StateSpace>& get_state_space() const;
    const std::shared_ptr<FluentAndDerivedMapper>& get_atom_index_mapper() const;
    const std::shared_ptr<TupleIndexMapper>& get_tuple_index_mapper() const;
    State get_root_state() const;
    const TupleGraphVertexList& get_vertices() const;
    const std::vector<VertexIndexList>& get_forward_successors() const;
    const std::vector<VertexIndexList>& get_backward_successors() const;
    const std::vector<VertexIndexList>& get_vertex_indices_by_distances() const;
    const std::vector<StateList>& get_states_by_distance() const;
};

class TupleGraphFactory
{
private:
    std::shared_ptr<StateSpace> m_state_space;
    std::shared_ptr<FluentAndDerivedMapper> m_atom_index_mapper;
    std::shared_ptr<TupleIndexMapper> m_tuple_index_mapper;
    bool m_prune_dominated_tuples;

    class TupleGraphArityZeroComputation
    {
    private:
        TupleGraph m_tuple_graph;

    public:
        TupleGraphArityZeroComputation(std::shared_ptr<StateSpace> state_space,
                                       std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper,
                                       std::shared_ptr<TupleIndexMapper> tuple_index_mapper,
                                       bool prune_dominated_tuples);

        /// @brief Compute the root state layer.
        void compute_root_state_layer(const State root_state);

        /// @brief Compute the layer at distance 1, assumes that the root state layer exists.
        void compute_first_layer();

        /// @brief Return a reference to the tuple graph.
        const TupleGraph& get_tuple_graph();
    };

    // Bookkeeping for memory reuse when building tuple graph of width greater 0
    class TupleGraphArityKComputation
    {
    private:
        TupleGraph m_tuple_graph;
        StateSet visited_states;
        DynamicNoveltyTable novelty_table;
        StateList cur_states;
        VertexIndexList cur_vertices;

        /**
         * Four step procedure to compute the next layer in the graph.
         */

        void compute_next_state_layer();

        TupleIndexSet novel_tuple_indices_set;
        TupleIndexList novel_tuple_indices;
        std::unordered_map<TupleIndex, StateSet> novel_tuple_index_to_states;
        std::unordered_map<State, TupleIndexList, StateHash> state_to_novel_tuple_indices;

        void compute_next_novel_tuple_indices();

        std::unordered_map<TupleIndex, StateSet> cur_novel_tuple_index_to_extended_state;
        std::unordered_map<TupleIndex, VertexIndexSet> cur_extended_novel_tuple_index_to_prev_vertices;
        TupleIndexSet cur_extended_novel_tuple_indices_set;
        TupleIndexList cur_extended_novel_tuple_indices;

        void extend_optimal_plans_from_prev_layer();

        std::unordered_map<TupleIndex, TupleIndexSet> tuple_index_to_dominating_tuple_indices;

        void instantiate_next_layer();

    public:
        TupleGraphArityKComputation(std::shared_ptr<StateSpace> state_space,
                                    std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper,
                                    std::shared_ptr<TupleIndexMapper> tuple_index_mapper,
                                    bool prune_dominated_tuples);

        /// @brief Compute the root state layer.
        void compute_root_state_layer(const State root_state);

        /// @brief Compute the next layer, assumes that the root state layer exists
        /// and return true iff the layer is nonempty.
        bool compute_next_layer();

        /// @brief Return a reference to the tuple graph.
        const TupleGraph& get_tuple_graph();
    };

    // Preallocated memory and construction logic.
    TupleGraphArityZeroComputation m_arity_zero_computation;
    TupleGraphArityKComputation m_arity_k_computation;

    /// @brief Create tuple graph for the special case of width 0, i.e.,
    /// any state with distance at most 1 from the root_state is a subgoal state.
    const TupleGraph& create_for_arity_zero(const State root_state);

    /// @brief Create a tuple graph for width k > 0.
    const TupleGraph& create_for_arity_k(const State root_state);

public:
    TupleGraphFactory(std::shared_ptr<StateSpace> state_space, int arity, bool prune_dominated_tuples = false);

    /// @brief Create and return a reference to the tuple graph.
    const TupleGraph& create(const State root_state);

    /**
     * Getters.
     */

    const std::shared_ptr<StateSpace>& get_state_space() const;
    const std::shared_ptr<FluentAndDerivedMapper>& get_atom_index_mapper() const;
    const std::shared_ptr<TupleIndexMapper>& get_tuple_index_mapper() const;
};

/**
 * Pretty printing as dot representation
 */

extern std::ostream& operator<<(std::ostream& out, const TupleGraph& tuple_graph);

}

#endif