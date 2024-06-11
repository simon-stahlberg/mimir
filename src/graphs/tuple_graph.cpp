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

#include "mimir/graphs/tuple_graph.hpp"

namespace mimir
{

/**
 * TupleGraphVertex
 */

TupleGraphVertex::TupleGraphVertex(int identifier, int tuple_index, StateList states) :
    m_identifier(identifier),
    m_tuple_index(tuple_index),
    m_states(std::move(states))
{
}

int TupleGraphVertex::get_identifier() const { return m_identifier; }

int TupleGraphVertex::get_tuple_index() const { return m_tuple_index; }

const StateList& TupleGraphVertex::get_states() const { return m_states; }

/**
 * TupleGraph
 */

TupleGraph::TupleGraph(std::shared_ptr<StateSpaceImpl> state_space,
                       std::shared_ptr<TupleIndexMapper> tuple_index_mapper,
                       State root_state,
                       TupleGraphVertexList vertices,
                       std::vector<std::vector<int>> forward_successors,
                       std::vector<std::vector<int>> backward_successors,
                       std::vector<std::vector<int>> vertex_indices_by_distance,
                       std::vector<StateList> states_by_distance) :
    m_state_space(std::move(state_space)),
    m_tuple_index_mapper(std::move(tuple_index_mapper)),
    m_root_state(root_state),
    m_vertices(std::move(vertices)),
    m_forward_successors(std::move(forward_successors)),
    m_backward_successors(std::move(backward_successors)),
    m_vertex_indices_by_distance(std::move(vertex_indices_by_distance)),
    m_states_by_distance(std::move(states_by_distance))
{
}

std::shared_ptr<StateSpaceImpl> TupleGraph::get_state_space() const { return m_state_space; }

std::shared_ptr<TupleIndexMapper> TupleGraph::get_tuple_index_mapper() const { return m_tuple_index_mapper; }

State TupleGraph::get_root_state() const { return m_root_state; }

const TupleGraphVertexList& TupleGraph::get_vertices() const { return m_vertices; }

const std::vector<std::vector<int>>& TupleGraph::get_forward_successors() const { return m_forward_successors; }

const std::vector<std::vector<int>>& TupleGraph::get_backward_successors() const { return m_backward_successors; }

const std::vector<std::vector<int>>& TupleGraph::get_vertex_indices_by_distances() const { return m_vertex_indices_by_distance; }

const std::vector<StateList>& TupleGraph::get_states_by_distance() const { return m_states_by_distance; }

/**
 * TupleGraphFactory
 */

TupleGraph TupleGraphFactory::create_for_arity_zero(const State root_state)
{
    auto vertices = TupleGraphVertexList {};
    auto forward_successors = std::vector<std::vector<int>> {};
    auto backward_successors = std::vector<std::vector<int>> {};
    auto vertex_indices_by_distances = std::vector<std::vector<int>> {};
    auto states_by_distance = std::vector<StateList> {};

    // Initialize distance 0 layer.
    const auto empty_tuple_index = m_tuple_index_mapper->get_empty_tuple_index();
    const auto root_state_vertex_id = vertices.size();
    vertices.emplace_back(root_state_vertex_id, empty_tuple_index, StateList { root_state });
    vertex_indices_by_distances.push_back({ empty_tuple_index });
    states_by_distance.push_back({ root_state });

    // Initialize distance 1 layer.
    const auto& transitions = m_state_space->get_forward_transitions()[root_state.get_id()];
    forward_successors.resize(vertices.size() + transitions.size());
    backward_successors.resize(vertices.size() + transitions.size());
    auto vertex_indices_layer = std::vector<int> {};
    auto states_layer = StateList {};
    for (const auto& transition : transitions)
    {
        const auto succ_state = transition.get_successor_state();
        const auto succ_state_vertex_id = vertices.size();
        vertices.emplace_back(succ_state_vertex_id, empty_tuple_index, StateList { succ_state });
        forward_successors[root_state_vertex_id].push_back(succ_state_vertex_id);
        backward_successors[succ_state_vertex_id].push_back(root_state_vertex_id);
        vertex_indices_layer.push_back(succ_state_vertex_id);
        states_layer.push_back(succ_state);
    }
    vertex_indices_by_distances.push_back(std::move(vertex_indices_layer));
    states_by_distance.push_back(std::move(states_layer));

    return TupleGraph(m_state_space,
                      m_tuple_index_mapper,
                      root_state,
                      std::move(vertices),
                      std::move(forward_successors),
                      std::move(backward_successors),
                      std::move(vertex_indices_by_distances),
                      std::move(states_by_distance));
}

TupleGraph TupleGraphFactory::create_for_arity_k(const State root_state)
{
    auto vertices = TupleGraphVertexList {};
    auto forward_successors = std::vector<std::vector<int>> {};
    auto backward_successors = std::vector<std::vector<int>> {};
    auto vertex_indices_by_distances = std::vector<std::vector<int>> {};
    auto states_by_distance = std::vector<StateList> {};

    // TODO: apply standard definition of width.

    return TupleGraph(m_state_space,
                      m_tuple_index_mapper,
                      root_state,
                      std::move(vertices),
                      std::move(forward_successors),
                      std::move(backward_successors),
                      std::move(vertex_indices_by_distances),
                      std::move(states_by_distance));
}

TupleGraphFactory::TupleGraphFactory(std::shared_ptr<StateSpaceImpl> state_space, int arity) :
    m_state_space(std::move(state_space)),
    m_tuple_index_mapper(std::make_shared<TupleIndexMapper>(arity,
                                                            m_state_space->get_aag()->get_pddl_factories().get_num_ground_atoms<Fluent>()
                                                                + m_state_space->get_aag()->get_pddl_factories().get_num_ground_atoms<Derived>()))
{
}

TupleGraph TupleGraphFactory::create(const State root_state)
{
    return (m_tuple_index_mapper->get_arity() > 0) ? create_for_arity_k(root_state) : create_for_arity_zero(root_state);
}

std::shared_ptr<StateSpaceImpl> TupleGraphFactory::get_state_space() const { return m_state_space; }

std::shared_ptr<TupleIndexMapper> TupleGraphFactory::get_tuple_index_mapper() const { return m_tuple_index_mapper; }

}
