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

static std::tuple<std::vector<int>, StateList> compute_next_layer(const StateSpaceImpl& state_space,
                                                                  const std::vector<int>& prev_vertices,
                                                                  const StateList& pref_states,
                                                                  DynamicNoveltyTable& novelty_table,
                                                                  TupleGraphVertexList& ref_vertices,
                                                                  std::vector<std::vector<int>>& ref_forward_successors,
                                                                  std::vector<std::vector<int>>& ref_backward_successors,
                                                                  StateSet& ref_visited_states,
                                                                  TupleIndexSet& out_novel_tuple_indices_set,
                                                                  TupleIndexList& out_novel_tuple_indices,
                                                                  std::vector<int>& out_cur_vertices,
                                                                  StateList& out_cur_states)
{
    // Compute next state layer
    for (const auto& state : pref_states)
    {
        for (const auto& transition : state_space.get_forward_transitions()[state.get_id()])
        {
            if (!ref_visited_states.count(transition.get_successor_state()))
            {
                out_cur_states.push_back(transition.get_successor_state());
            }
        }
    }
    if (out_cur_states.empty())
    {
        return std::make_tuple(std::vector<int> {}, StateList {});
    }

    // Compute novel tuples
    auto novel_tuple_indices_set = std::unordered_set<TupleIndex> {};
    for (const auto& state : out_cur_states)
    {
        novelty_table.compute_novel_tuple_indices(state, out_novel_tuple_indices);
        novel_tuple_indices_set.insert(out_novel_tuple_indices.begin(), out_novel_tuple_indices.end());
    }
    out_novel_tuple_indices.clear();
    out_novel_tuple_indices.insert(out_novel_tuple_indices.end(), novel_tuple_indices_set.begin(), novel_tuple_indices_set.end());

    // Extend optimal plans for tuples at distance-1 to tuples at distance
    return std::make_tuple(std::vector<int> {}, StateList {});
}

TupleGraph TupleGraphFactory::create_for_arity_k(const State root_state)
{
    auto vertices = TupleGraphVertexList {};
    auto forward_successors = std::vector<std::vector<int>> {};
    auto backward_successors = std::vector<std::vector<int>> {};
    auto vertex_indices_by_distances = std::vector<std::vector<int>> {};
    auto states_by_distance = std::vector<StateList> {};

    // Initialize temporary variables
    auto novel_tuple_indices_set = TupleIndexSet {};
    auto novel_tuple_indices = TupleIndexList {};
    auto visited_states = StateSet {};

    // 1. Initialize layer at distance 0.
    auto novelty_table = DynamicNoveltyTable(m_atom_index_mapper, m_tuple_index_mapper);
    novelty_table.compute_novel_tuple_indices(root_state, novel_tuple_indices);
    assert(!novel_tuple_indices.empty());

    auto vertices_i = std::vector<int> {};
    auto states_i = StateList { root_state };
    if (m_prune_dominated_tuples)
    {
        const int vertex_id = vertices.size();
        vertices.emplace_back(vertex_id, novel_tuple_indices.front(), StateList { root_state });
        vertices_i.push_back(vertex_id);
    }
    else
    {
        for (const auto& novel_tuple_index : novel_tuple_indices)
        {
            const int vertex_id = vertices.size();
            vertices.emplace_back(vertex_id, novel_tuple_index, StateList { root_state });
            vertices_i.push_back(vertex_id);
        }
    }
    vertex_indices_by_distances.push_back(vertices_i);
    states_by_distance.push_back(states_i);
    novelty_table.insert_tuple_indices(novel_tuple_indices);

    // 2. Initialize layers at distance > 0.
    for (int dist = 0;; ++dist)
    {
        vertices_i.clear();
        states_i.clear();

        compute_next_layer(*m_state_space,
                           vertex_indices_by_distances[dist],
                           states_by_distance[dist],
                           novelty_table,
                           vertices,
                           forward_successors,
                           backward_successors,
                           visited_states,
                           novel_tuple_indices_set,
                           novel_tuple_indices,
                           vertices_i,
                           states_i);

        if (vertices_i.empty())
        {
            break;
        }

        vertex_indices_by_distances.push_back(vertices_i);
        states_by_distance.push_back(states_i);
    }

    return TupleGraph(m_state_space,
                      m_tuple_index_mapper,
                      root_state,
                      std::move(vertices),
                      std::move(forward_successors),
                      std::move(backward_successors),
                      std::move(vertex_indices_by_distances),
                      std::move(states_by_distance));
}

TupleGraphFactory::TupleGraphFactory(std::shared_ptr<StateSpaceImpl> state_space, int arity, bool prune_dominated_tuples) :
    m_state_space(std::move(state_space)),
    m_prune_dominated_tuples(prune_dominated_tuples),
    m_atom_index_mapper(std::make_shared<FluentAndDerivedMapper>()),
    m_tuple_index_mapper(std::make_shared<TupleIndexMapper>(arity,
                                                            m_state_space->get_ssg()->get_reached_fluent_ground_atoms().count()
                                                                + m_state_space->get_ssg()->get_reached_derived_ground_atoms().count()))
{
}

TupleGraph TupleGraphFactory::create(const State root_state)
{
    return (m_tuple_index_mapper->get_arity() > 0) ? create_for_arity_k(root_state) : create_for_arity_zero(root_state);
}

std::shared_ptr<StateSpaceImpl> TupleGraphFactory::get_state_space() const { return m_state_space; }

std::shared_ptr<TupleIndexMapper> TupleGraphFactory::get_tuple_index_mapper() const { return m_tuple_index_mapper; }

}
