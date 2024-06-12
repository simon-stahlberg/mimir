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

TupleGraphFactory::TupleGraphArityZeroComputation::TupleGraphArityZeroComputation(std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper,
                                                                                  std::shared_ptr<TupleIndexMapper> tuple_index_mapper,
                                                                                  std::shared_ptr<StateSpaceImpl> state_space,
                                                                                  const State root_state,
                                                                                  bool prune_dominated_tuples) :
    m_atom_index_mapper(std::move(atom_index_mapper)),
    m_tuple_index_mapper(std::move(tuple_index_mapper)),
    m_state_space(std::move(state_space)),
    m_root_state(root_state),
    m_prune_dominated_tuples(prune_dominated_tuples)
{
}

void TupleGraphFactory::TupleGraphArityZeroComputation::compute_root_state_layer()
{
    const auto empty_tuple_index = m_tuple_index_mapper->get_empty_tuple_index();
    const auto root_state_vertex_id = 0;
    m_vertices.emplace_back(root_state_vertex_id, empty_tuple_index, StateList { m_root_state });
    m_vertex_indices_by_distances.push_back({ empty_tuple_index });
    m_states_by_distance.push_back({ m_root_state });
}

void TupleGraphFactory::TupleGraphArityZeroComputation::compute_first_layer()
{
    const auto empty_tuple_index = m_tuple_index_mapper->get_empty_tuple_index();
    const auto root_state_vertex_id = 0;

    const auto& transitions = m_state_space->get_forward_transitions()[m_root_state.get_id()];
    m_forward_successors.resize(m_vertices.size() + transitions.size());
    m_backward_successors.resize(m_vertices.size() + transitions.size());
    auto vertex_indices_layer = std::vector<int> {};
    auto states_layer = StateList {};
    for (const auto& transition : transitions)
    {
        const auto succ_state = transition.get_successor_state();
        const auto succ_state_vertex_id = m_vertices.size();
        m_vertices.emplace_back(succ_state_vertex_id, empty_tuple_index, StateList { succ_state });
        m_forward_successors[root_state_vertex_id].push_back(succ_state_vertex_id);
        m_backward_successors[succ_state_vertex_id].push_back(root_state_vertex_id);
        vertex_indices_layer.push_back(succ_state_vertex_id);
        states_layer.push_back(succ_state);
    }
    m_vertex_indices_by_distances.push_back(std::move(vertex_indices_layer));
    m_states_by_distance.push_back(std::move(states_layer));
}

TupleGraph TupleGraphFactory::TupleGraphArityZeroComputation::extract_tuple_graph()
{
    return TupleGraph(std::move(m_state_space),
                      std::move(m_tuple_index_mapper),
                      m_root_state,
                      std::move(m_vertices),
                      std::move(m_forward_successors),
                      std::move(m_backward_successors),
                      std::move(m_vertex_indices_by_distances),
                      std::move(m_states_by_distance));
}

TupleGraphFactory::TupleGraphArityKComputation::TupleGraphArityKComputation(std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper_,
                                                                            std::shared_ptr<TupleIndexMapper> tuple_index_mapper_,
                                                                            std::shared_ptr<StateSpaceImpl> state_space_,
                                                                            const State root_state_,
                                                                            bool prune_dominated_tuples_) :
    atom_index_mapper(std::move(atom_index_mapper_)),
    tuple_index_mapper(std::move(tuple_index_mapper_)),
    state_space(std::move(state_space_)),
    root_state(root_state_),
    prune_dominated_tuples(prune_dominated_tuples_),
    novelty_table(atom_index_mapper, tuple_index_mapper)
{
}

void TupleGraphFactory::TupleGraphArityKComputation::compute_root_state_layer()
{
    cur_vertices.clear();
    cur_states.clear();

    cur_states.push_back(root_state);
    if (prune_dominated_tuples)
    {
        const int vertex_id = vertices.size();
        vertices.emplace_back(vertex_id, novel_tuple_indices.front(), StateList { root_state });
        cur_vertices.push_back(vertex_id);
    }
    else
    {
        for (const auto& novel_tuple_index : novel_tuple_indices)
        {
            const int vertex_id = vertices.size();
            vertices.emplace_back(vertex_id, novel_tuple_index, StateList { root_state });
            cur_vertices.push_back(vertex_id);
        }
    }
    vertex_indices_by_distances.push_back(cur_vertices);
    states_by_distance.push_back(cur_states);
    novelty_table.insert_tuple_indices(novel_tuple_indices);
}

void TupleGraphFactory::TupleGraphArityKComputation::compute_next_state_layer()
{
    cur_states.clear();

    for (const auto& state : states_by_distance.back())
    {
        for (const auto& transition : state_space->get_forward_transitions()[state.get_id()])
        {
            const auto succ_state = transition.get_successor_state();

            if (!visited_states.count(succ_state))
            {
                cur_states.push_back(succ_state);
            }
            visited_states.insert(succ_state);
        }
    }
}

void TupleGraphFactory::TupleGraphArityKComputation::compute_next_novel_tuple_indices()
{
    novel_tuple_indices_set.clear();
    novel_tuple_indices.clear();
    novel_tuple_index_to_state_indices.clear();
    state_to_novel_tuple_indices.clear();

    for (const auto& state : cur_states)
    {
        novelty_table.compute_novel_tuple_indices(state, novel_tuple_indices);
        for (const auto& tuple_index : novel_tuple_indices)
        {
            novel_tuple_index_to_state_indices[tuple_index].insert(state);
        }
        state_to_novel_tuple_indices.emplace(state, novel_tuple_indices);
        novel_tuple_indices_set.insert(novel_tuple_indices.begin(), novel_tuple_indices.end());
    }
    novel_tuple_indices.clear();
    novel_tuple_indices.insert(novel_tuple_indices.end(), novel_tuple_indices_set.begin(), novel_tuple_indices_set.end());
    novelty_table.insert_tuple_indices(novel_tuple_indices);
}

void TupleGraphFactory::TupleGraphArityKComputation::extend_optimal_plans_from_prev_layer()
{
    cur_extendable_tuple_indices.clear();
    extendable_tuple_index_to_prev_vertices.clear();

    for (auto& prev_vertex : cur_vertices)
    {
        cur_tuple_index_to_underlying_extendable_state.clear();

        const auto& forward_transitions = state_space->get_forward_transitions();

        for (const auto state : vertices[prev_vertex].get_states())
        {
            const auto& transitions = forward_transitions[state.get_id()];

            for (const auto& transition : transitions)
            {
                const auto it = state_to_novel_tuple_indices.find(transition.get_successor_state());

                if (it != state_to_novel_tuple_indices.end())
                {
                    for (const auto target_tuple_index : it->second)
                    {
                        cur_tuple_index_to_underlying_extendable_state[target_tuple_index].insert(state);
                    }
                }
            }
        }

        auto extended_tuple_indices = TupleIndexList {};
        for (const auto& [cur_tuple_index, cur_states] : cur_tuple_index_to_underlying_extendable_state)
        {
            // Check whether all optimal plans for cur_node_index can be extended into optimal plans for succ_tuple_index
            if (novel_tuple_indices_set.count(cur_tuple_index) && cur_states.size() == vertices[prev_vertex].get_states().size())
            {
                cur_extendable_tuple_indices.insert(cur_tuple_index);

                extendable_tuple_index_to_prev_vertices[cur_tuple_index].insert(prev_vertex);
            }
        }
    }
}

void TupleGraphFactory::TupleGraphArityKComputation::instantiate_next_layer()
{
    cur_vertices.clear();

    for (const auto& tuple_index : cur_extendable_tuple_indices)
    {
        auto cur_vertex_index = vertices.size();
        const auto& cur_states = novel_tuple_index_to_state_indices.at(tuple_index);
        vertices.emplace_back(cur_vertex_index, tuple_index, StateList(cur_states.begin(), cur_states.end()));
        cur_vertices.push_back(cur_vertex_index);

        for (const auto prev_vertex_index : extendable_tuple_index_to_prev_vertices[tuple_index])
        {
            forward_successors.resize(cur_vertex_index);
            backward_successors.resize(cur_vertex_index);

            forward_successors[prev_vertex_index].push_back(cur_vertex_index);
            backward_successors[cur_vertex_index].push_back(prev_vertex_index);
        }
    }

    states_by_distance.push_back(cur_states);
    vertex_indices_by_distances.push_back(cur_vertices);
}

bool TupleGraphFactory::TupleGraphArityKComputation::compute_next_layer()
{
    compute_next_state_layer();
    if (cur_states.empty())
    {
        return false;
    }

    compute_next_novel_tuple_indices();
    if (novel_tuple_indices.empty())
    {
        return false;
    }

    extend_optimal_plans_from_prev_layer();
    if (cur_extendable_tuple_indices.empty())
    {
        return false;
    }

    instantiate_next_layer();

    return true;
}

TupleGraph TupleGraphFactory::TupleGraphArityKComputation::extract_tuple_graph()
{
    return TupleGraph(std::move(state_space),
                      std::move(tuple_index_mapper),
                      root_state,
                      std::move(vertices),
                      std::move(forward_successors),
                      std::move(backward_successors),
                      std::move(vertex_indices_by_distances),
                      std::move(states_by_distance));
}

TupleGraph TupleGraphFactory::create_for_arity_zero(const State root_state) const
{
    auto computation = TupleGraphArityZeroComputation(m_atom_index_mapper, m_tuple_index_mapper, m_state_space, root_state, m_prune_dominated_tuples);

    computation.compute_root_state_layer();

    computation.compute_first_layer();

    return computation.extract_tuple_graph();
}

TupleGraph TupleGraphFactory::create_for_arity_k(const State root_state) const
{
    auto computation = TupleGraphArityKComputation(m_atom_index_mapper, m_tuple_index_mapper, m_state_space, root_state, m_prune_dominated_tuples);

    computation.compute_root_state_layer();

    while (true)
    {
        auto is_empty = computation.compute_next_layer();

        if (!is_empty)
        {
            break;
        }
    }

    return computation.extract_tuple_graph();
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

TupleGraph TupleGraphFactory::create(const State root_state) const
{
    return (m_tuple_index_mapper->get_arity() > 0) ? create_for_arity_k(root_state) : create_for_arity_zero(root_state);
}

std::shared_ptr<StateSpaceImpl> TupleGraphFactory::get_state_space() const { return m_state_space; }

std::shared_ptr<TupleIndexMapper> TupleGraphFactory::get_tuple_index_mapper() const { return m_tuple_index_mapper; }

std::ostream& operator<<(std::ostream& out, const TupleGraph& tuple_graph)
{
    // TODO:
    out << "digraph {\n";

    out << "}";  // end digraph
    return out;
}

}
