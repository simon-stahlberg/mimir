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

TupleGraphVertex::TupleGraphVertex(VertexIndex index, TupleIndex tuple_index, StateList states) :
    m_index(index),
    m_tuple_index(tuple_index),
    m_states(std::move(states))
{
}

VertexIndex TupleGraphVertex::get_index() const { return m_index; }

TupleIndex TupleGraphVertex::get_tuple_index() const { return m_tuple_index; }

const StateList& TupleGraphVertex::get_states() const { return m_states; }

/**
 * TupleGraph
 */

TupleGraph::TupleGraph(std::shared_ptr<StateSpace> state_space,
                       std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper,
                       std::shared_ptr<TupleIndexMapper> tuple_index_mapper,
                       bool prune_dominated_tuples) :
    m_state_space(std::move(state_space)),
    m_atom_index_mapper(std::move(atom_index_mapper)),
    m_tuple_index_mapper(std::move(tuple_index_mapper)),
    m_prune_dominated_tuples(prune_dominated_tuples),
    m_vertices(),
    m_forward_successors(),
    m_backward_successors(),
    m_vertex_indices_by_distance(),
    m_states_by_distance()
{
}

std::optional<VertexIndexList> TupleGraph::compute_admissible_chain(const GroundAtomList<Fluent>& fluent_atoms, const GroundAtomList<Derived>& derived_atoms)
{
    // Construct the explict tuple representation
    auto atom_indices = AtomIndexList {};
    for (const auto& atom : fluent_atoms)
    {
        const auto atom_index = atom->get_identifier();
        const auto remapped_atom_index = m_atom_index_mapper->get_fluent_remap().at(atom_index);
        assert(remapped_atom_index != -1);

        atom_indices.push_back(remapped_atom_index);
    }
    for (const auto& atom : derived_atoms)
    {
        const auto atom_index = atom->get_identifier();
        const auto remapped_atom_index = m_atom_index_mapper->get_derived_remap().at(atom_index);
        assert(remapped_atom_index != -1);

        atom_indices.push_back(remapped_atom_index);
    }
    std::sort(atom_indices.begin(), atom_indices.end());

    if (static_cast<int>(atom_indices.size()) > m_tuple_index_mapper->get_arity())
    {
        // Size of tuple exceeds width, hence, it cannot be in the tuple graph.
        return std::nullopt;
    }
    // Construct the implicit tuple representation by adding placeholders and using the perfect hash function.
    atom_indices.resize(m_tuple_index_mapper->get_arity(), m_tuple_index_mapper->get_num_atoms());
    assert(std::is_sorted(atom_indices.begin(), atom_indices.end()));
    const auto tuple_index = m_tuple_index_mapper->to_tuple_index(atom_indices);

    for (const auto& vertex : m_vertices)
    {
        if (vertex.get_tuple_index() == tuple_index)
        {
            // Backtrack admissible chain until the root and return an admissible chain that proves the width.
            auto cur_vertex_index = vertex.get_index();
            auto admissible_chain = VertexIndexList { cur_vertex_index };
            while (!m_backward_successors.at(cur_vertex_index).empty())
            {
                cur_vertex_index = m_backward_successors.at(cur_vertex_index).front();
                admissible_chain.push_back(cur_vertex_index);
            }
            std::reverse(admissible_chain.begin(), admissible_chain.end());
            return admissible_chain;
        }
    }
    // Tuple was not found in the tuple graph.
    return std::nullopt;
}

std::optional<VertexIndexList> TupleGraph::compute_admissible_chain(const StateList& states)
{
    if (states.empty())
    {
        // No tuple can optimally goal imply an empty set of states
        return std::nullopt;
    }

    auto optimal_distance = std::numeric_limits<double>::max();
    const auto distances = m_state_space->compute_shortest_distances_from_states(StateIdList { m_state_space->get_initial_state() });
    for (const auto& state : states)
    {
        const auto state_index = m_state_space->get_state_index(state);
        const auto state_distance = distances.at(state_index);
        // Unreachable states have distance std::numeric_limits<double>::max()
        if (state_distance != std::numeric_limits<double>::max())
        {
            optimal_distance = std::min(optimal_distance, state_distance);
        }
    }

    auto optimal_states = StateSet {};
    for (const auto& state : states)
    {
        const auto state_index = m_state_space->get_state_index(state);
        if (distances.at(state_index) == optimal_distance)
        {
            optimal_states.insert(state);
        }
    }

    if (optimal_states.empty())
    {
        // No tuple can optimally goal imply an empty set of states
        return std::nullopt;
    }

    for (const auto& vertex : m_vertices)
    {
        // Part 3 of definition of width
        const auto tuple_optimally_goal_implies_states = std::all_of(vertex.get_states().begin(),
                                                                     vertex.get_states().end(),
                                                                     [&optimal_states](const auto& tuple_state) { return optimal_states.count(tuple_state); });

        if (tuple_optimally_goal_implies_states)
        {
            // Backtrack admissible chain until the root and return an admissible chain that proves the width.
            auto cur_vertex_index = vertex.get_index();
            auto admissible_chain = VertexIndexList { cur_vertex_index };
            while (!m_backward_successors.at(cur_vertex_index).empty())
            {
                cur_vertex_index = m_backward_successors.at(cur_vertex_index).front();
                admissible_chain.push_back(cur_vertex_index);
            }
            std::reverse(admissible_chain.begin(), admissible_chain.end());
            return admissible_chain;
        }
    }
    return std::nullopt;
}

const std::shared_ptr<StateSpace>& TupleGraph::get_state_space() const { return m_state_space; }

const std::shared_ptr<FluentAndDerivedMapper>& TupleGraph::get_atom_index_mapper() const { return m_atom_index_mapper; }

const std::shared_ptr<TupleIndexMapper>& TupleGraph::get_tuple_index_mapper() const { return m_tuple_index_mapper; }

State TupleGraph::get_root_state() const { return m_states_by_distance.front().front(); }

const TupleGraphVertexList& TupleGraph::get_vertices() const { return m_vertices; }

const std::vector<VertexIndexList>& TupleGraph::get_forward_successors() const { return m_forward_successors; }

const std::vector<VertexIndexList>& TupleGraph::get_backward_successors() const { return m_backward_successors; }

const std::vector<VertexIndexList>& TupleGraph::get_vertex_indices_by_distances() const { return m_vertex_indices_by_distance; }

const std::vector<StateList>& TupleGraph::get_states_by_distance() const { return m_states_by_distance; }

/**
 * TupleGraphFactory
 */

TupleGraphFactory::TupleGraphArityZeroComputation::TupleGraphArityZeroComputation(std::shared_ptr<StateSpace> state_space,
                                                                                  std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper,
                                                                                  std::shared_ptr<TupleIndexMapper> tuple_index_mapper,
                                                                                  bool prune_dominated_tuples) :
    m_tuple_graph(std::move(state_space), std::move(atom_index_mapper), std::move(tuple_index_mapper), prune_dominated_tuples)
{
}

void TupleGraphFactory::TupleGraphArityZeroComputation::compute_root_state_layer(const State root_state)
{
    // Clear tuple graph
    m_tuple_graph.m_vertices.clear();
    m_tuple_graph.m_forward_successors.clear();
    m_tuple_graph.m_backward_successors.clear();
    m_tuple_graph.m_vertex_indices_by_distance.clear();
    m_tuple_graph.m_states_by_distance.clear();

    const auto empty_tuple_index = m_tuple_graph.m_tuple_index_mapper->get_empty_tuple_index();
    const auto root_state_vertex_index = 0;
    m_tuple_graph.m_vertices.emplace_back(root_state_vertex_index, empty_tuple_index, StateList { root_state });
    m_tuple_graph.m_vertex_indices_by_distance.push_back({ empty_tuple_index });
    m_tuple_graph.m_states_by_distance.push_back({ root_state });
}

void TupleGraphFactory::TupleGraphArityZeroComputation::compute_first_layer()
{
    const auto empty_tuple_index = m_tuple_graph.m_tuple_index_mapper->get_empty_tuple_index();
    const auto root_state_vertex_index = 0;
    const auto root_state_index = m_tuple_graph.m_state_space->get_state_index(m_tuple_graph.get_root_state());
    const auto& transitions = m_tuple_graph.m_state_space->get_transitions();
    const auto& transition_indices = m_tuple_graph.m_state_space->get_forward_transition_adjacency_lists().at(root_state_index);
    m_tuple_graph.m_forward_successors.resize(m_tuple_graph.m_vertices.size() + transition_indices.size());
    m_tuple_graph.m_backward_successors.resize(m_tuple_graph.m_vertices.size() + transition_indices.size());
    auto vertex_indices_layer = VertexIndexList {};
    auto states_layer = StateList {};
    for (const auto& transition_index : transition_indices)
    {
        const auto& transition = transitions.at(transition_index);
        const auto succ_state_index = transition.get_forward_successor();
        const auto succ_state = m_tuple_graph.m_state_space->get_states().at(succ_state_index);
        if (succ_state == m_tuple_graph.get_root_state())
        {
            // Root state was already visited
            continue;
        }
        const auto succ_state_vertex_index = m_tuple_graph.m_vertices.size();
        m_tuple_graph.m_vertices.emplace_back(succ_state_vertex_index, empty_tuple_index, StateList { succ_state });
        m_tuple_graph.m_forward_successors.at(root_state_vertex_index).push_back(succ_state_vertex_index);
        m_tuple_graph.m_backward_successors.at(succ_state_vertex_index).push_back(root_state_vertex_index);
        vertex_indices_layer.push_back(succ_state_vertex_index);
        states_layer.push_back(succ_state);
    }
    m_tuple_graph.m_vertex_indices_by_distance.push_back(std::move(vertex_indices_layer));
    m_tuple_graph.m_states_by_distance.push_back(std::move(states_layer));
}

const TupleGraph& TupleGraphFactory::TupleGraphArityZeroComputation::get_tuple_graph() { return m_tuple_graph; }

TupleGraphFactory::TupleGraphArityKComputation::TupleGraphArityKComputation(std::shared_ptr<StateSpace> state_space,
                                                                            std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper,
                                                                            std::shared_ptr<TupleIndexMapper> tuple_index_mapper,
                                                                            bool prune_dominated_tuples) :
    m_tuple_graph(std::move(state_space), std::move(atom_index_mapper), std::move(tuple_index_mapper), prune_dominated_tuples),
    visited_states(),
    novelty_table(m_tuple_graph.m_atom_index_mapper, m_tuple_graph.m_tuple_index_mapper),
    cur_states(),
    cur_vertices(),
    novel_tuple_indices_set(),
    novel_tuple_indices(),
    novel_tuple_index_to_states(),
    state_to_novel_tuple_indices(),
    cur_novel_tuple_index_to_extended_state(),
    cur_extended_novel_tuple_index_to_prev_vertices(),
    cur_extended_novel_tuple_indices_set(),
    cur_extended_novel_tuple_indices(),
    tuple_index_to_dominating_tuple_indices()
{
}

void TupleGraphFactory::TupleGraphArityKComputation::compute_root_state_layer(const State root_state)
{
    // Clear tuple graph
    m_tuple_graph.m_vertices.clear();
    m_tuple_graph.m_forward_successors.clear();
    m_tuple_graph.m_backward_successors.clear();
    m_tuple_graph.m_vertex_indices_by_distance.clear();
    m_tuple_graph.m_states_by_distance.clear();

    // Clear data structures
    cur_vertices.clear();
    cur_states.clear();
    novelty_table.reset();
    visited_states.clear();

    cur_states.push_back(root_state);
    novelty_table.compute_novel_tuple_indices(root_state, novel_tuple_indices);
    if (m_tuple_graph.m_prune_dominated_tuples)
    {
        const int vertex_index = m_tuple_graph.m_vertices.size();
        m_tuple_graph.m_vertices.emplace_back(vertex_index, novel_tuple_indices.front(), StateList { root_state });
        cur_vertices.push_back(vertex_index);
    }
    else
    {
        for (const auto& novel_tuple_index : novel_tuple_indices)
        {
            const int vertex_index = m_tuple_graph.m_vertices.size();
            m_tuple_graph.m_vertices.emplace_back(vertex_index, novel_tuple_index, StateList { root_state });
            cur_vertices.push_back(vertex_index);
        }
    }
    m_tuple_graph.m_vertex_indices_by_distance.push_back(cur_vertices);
    m_tuple_graph.m_states_by_distance.push_back(cur_states);
    novelty_table.insert_tuple_indices(novel_tuple_indices);
}

void TupleGraphFactory::TupleGraphArityKComputation::compute_next_state_layer()
{
    // Clear data structures
    cur_states.clear();

    const auto& transitions = m_tuple_graph.get_state_space()->get_transitions();
    for (const auto& state : m_tuple_graph.m_states_by_distance.back())
    {
        const auto state_index = m_tuple_graph.m_state_space->get_state_index(state);
        for (const auto& transition_index : m_tuple_graph.m_state_space->get_forward_transition_adjacency_lists().at(state_index))
        {
            const auto& transition = transitions.at(transition_index);
            const auto succ_state_index = transition.get_forward_successor();
            const auto succ_state = m_tuple_graph.m_state_space->get_states().at(succ_state_index);

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
    // Clear data structures
    novel_tuple_indices_set.clear();
    novel_tuple_indices.clear();
    novel_tuple_index_to_states.clear();
    state_to_novel_tuple_indices.clear();

    for (const auto& state : cur_states)
    {
        novelty_table.compute_novel_tuple_indices(state, novel_tuple_indices);
        for (const auto& tuple_index : novel_tuple_indices)
        {
            novel_tuple_index_to_states[tuple_index].insert(state);
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
    // Clear data structures
    cur_extended_novel_tuple_indices_set.clear();
    cur_extended_novel_tuple_indices.clear();
    cur_extended_novel_tuple_index_to_prev_vertices.clear();

    const auto& forward_transitions = m_tuple_graph.m_state_space->get_forward_transition_adjacency_lists();

    const auto& transitions = m_tuple_graph.get_state_space()->get_transitions();
    for (auto& prev_vertex : cur_vertices)
    {
        cur_novel_tuple_index_to_extended_state.clear();

        // Compute extended plans
        for (const auto state : m_tuple_graph.m_vertices.at(prev_vertex).get_states())
        {
            const auto state_index = m_tuple_graph.m_state_space->get_state_index(state);
            for (const auto& transition_index : forward_transitions.at(state_index))
            {
                const auto& transition = transitions.at(transition_index);
                const auto succ_state_index = transition.get_forward_successor();
                const auto succ_state = m_tuple_graph.m_state_space->get_states().at(succ_state_index);

                if (state_to_novel_tuple_indices.count(succ_state))
                {
                    for (const auto target_tuple_index : state_to_novel_tuple_indices.at(succ_state))
                    {
                        cur_novel_tuple_index_to_extended_state[target_tuple_index].insert(state);
                    }
                }
            }
        }

        // Check whether all plans for tuple t_{i-1} were extended into optimal plan for tuple t_i.
        for (const auto& [cur_novel_tuple_index, extended_states] : cur_novel_tuple_index_to_extended_state)
        {
            bool all_optimal_plans_extended = (extended_states.size() == m_tuple_graph.m_vertices.at(prev_vertex).get_states().size());

            if (all_optimal_plans_extended)
            {
                cur_extended_novel_tuple_indices_set.insert(cur_novel_tuple_index);
                cur_extended_novel_tuple_index_to_prev_vertices[cur_novel_tuple_index].insert(prev_vertex);
            }
        }
    }
    cur_extended_novel_tuple_indices.insert(cur_extended_novel_tuple_indices.end(),
                                            cur_extended_novel_tuple_indices_set.begin(),
                                            cur_extended_novel_tuple_indices_set.end());
}

void TupleGraphFactory::TupleGraphArityKComputation::instantiate_next_layer()
{
    // Clear data structures
    tuple_index_to_dominating_tuple_indices.clear();
    cur_vertices.clear();

    if (m_tuple_graph.m_prune_dominated_tuples)
    {
        for (size_t i = 0; i < cur_extended_novel_tuple_indices.size(); ++i)
        {
            const auto tuple_index_1 = cur_extended_novel_tuple_indices.at(i);

            const auto& states_1 = novel_tuple_index_to_states.at(tuple_index_1);

            for (size_t j = i + 1; j < cur_extended_novel_tuple_indices.size(); ++j)
            {
                const auto tuple_index_2 = cur_extended_novel_tuple_indices.at(j);

                const auto& states_2 = novel_tuple_index_to_states.at(tuple_index_2);

                if (states_1 == states_2)
                {
                    // Keep only one tuple_index with a specific set of underlying states.
                    cur_extended_novel_tuple_indices_set.erase(tuple_index_2);
                    continue;
                }

                const auto is_subseteq = std::all_of(states_2.begin(), states_2.end(), [&states_1](const auto& element) { return states_1.count(element); });

                if (is_subseteq)
                {
                    // tuple_index_2 is dominated by tuple_index_1 because states_2 < states_1.
                    tuple_index_to_dominating_tuple_indices[tuple_index_1].insert(tuple_index_2);
                }
            }
        }

        // Keep only tuple indices whose underlying states is a smallest subset.
        for (const auto& [tuple_index, dominated_by_tuple_indices] : tuple_index_to_dominating_tuple_indices)
        {
            if (dominated_by_tuple_indices.empty())
            {
                cur_extended_novel_tuple_indices_set.erase(tuple_index);
            }
        }
    }

    for (const auto& tuple_index : cur_extended_novel_tuple_indices_set)
    {
        auto cur_vertex_index = m_tuple_graph.m_vertices.size();
        const auto& cur_states = novel_tuple_index_to_states.at(tuple_index);
        m_tuple_graph.m_vertices.emplace_back(cur_vertex_index, tuple_index, StateList(cur_states.begin(), cur_states.end()));
        cur_vertices.push_back(cur_vertex_index);

        for (const auto prev_vertex_index : cur_extended_novel_tuple_index_to_prev_vertices[tuple_index])
        {
            m_tuple_graph.m_forward_successors.resize(cur_vertex_index + 1);
            m_tuple_graph.m_backward_successors.resize(cur_vertex_index + 1);

            m_tuple_graph.m_forward_successors.at(prev_vertex_index).push_back(cur_vertex_index);
            m_tuple_graph.m_backward_successors.at(cur_vertex_index).push_back(prev_vertex_index);
        }
    }

    m_tuple_graph.m_states_by_distance.push_back(cur_states);
    m_tuple_graph.m_vertex_indices_by_distance.push_back(cur_vertices);
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
    if (cur_extended_novel_tuple_indices_set.empty())
    {
        return false;
    }

    instantiate_next_layer();

    return true;
}

const TupleGraph& TupleGraphFactory::TupleGraphArityKComputation::get_tuple_graph() { return m_tuple_graph; }

const TupleGraph& TupleGraphFactory::create_for_arity_zero(const State root_state)
{
    m_arity_zero_computation.compute_root_state_layer(root_state);

    m_arity_zero_computation.compute_first_layer();

    return m_arity_zero_computation.get_tuple_graph();
}

const TupleGraph& TupleGraphFactory::create_for_arity_k(const State root_state)
{
    m_arity_k_computation.compute_root_state_layer(root_state);

    while (true)
    {
        auto is_empty = m_arity_k_computation.compute_next_layer();

        if (!is_empty)
        {
            break;
        }
    }

    return m_arity_k_computation.get_tuple_graph();
}

TupleGraphFactory::TupleGraphFactory(std::shared_ptr<StateSpace> state_space, int arity, bool prune_dominated_tuples) :
    m_state_space(std::move(state_space)),
    m_atom_index_mapper(std::make_shared<FluentAndDerivedMapper>(m_state_space->get_aag()->get_pddl_factories()->get_ground_atom_factory<Fluent>(),
                                                                 m_state_space->get_aag()->get_pddl_factories()->get_ground_atom_factory<Derived>())),
    m_tuple_index_mapper(std::make_shared<TupleIndexMapper>(arity,
                                                            m_state_space->get_aag()->get_pddl_factories()->get_num_ground_atoms<Fluent>()
                                                                + m_state_space->get_aag()->get_pddl_factories()->get_num_ground_atoms<Derived>())),
    m_prune_dominated_tuples(prune_dominated_tuples),
    m_arity_zero_computation(m_state_space, m_atom_index_mapper, m_tuple_index_mapper, m_prune_dominated_tuples),
    m_arity_k_computation(m_state_space, m_atom_index_mapper, m_tuple_index_mapper, m_prune_dominated_tuples)
{
}

const TupleGraph& TupleGraphFactory::create(const State root_state)
{
    return (m_tuple_index_mapper->get_arity() > 0) ? create_for_arity_k(root_state) : create_for_arity_zero(root_state);
}

const std::shared_ptr<StateSpace>& TupleGraphFactory::get_state_space() const { return m_state_space; }

const std::shared_ptr<FluentAndDerivedMapper>& TupleGraphFactory::get_atom_index_mapper() const { return m_atom_index_mapper; }

const std::shared_ptr<TupleIndexMapper>& TupleGraphFactory::get_tuple_index_mapper() const { return m_tuple_index_mapper; }

std::ostream& operator<<(std::ostream& out, const TupleGraph& tuple_graph)
{
    const auto problem = tuple_graph.get_state_space()->get_pddl_parser()->get_problem();
    const auto& pddl_factories = *tuple_graph.get_state_space()->get_pddl_parser()->get_factories();
    auto combined_atom_indices = AtomIndexList {};
    auto fluent_atom_indices = AtomIndexList {};
    auto derived_atom_indices = AtomIndexList {};

    out << "digraph {\n"
        << "rankdir=\"LR\""
        << "\n";

    // 3. Tuple nodes.
    for (int node_index : tuple_graph.get_vertex_indices_by_distances().front())
    {
        out << "Dangling" << node_index << " [ label = \"\", style = invis ]\n";
    }
    for (const auto& vertex_indices : tuple_graph.get_vertex_indices_by_distances())
    {
        for (int vertex_index : vertex_indices)
        {
            const auto& vertex = tuple_graph.get_vertices().at(vertex_index);
            out << "t" << vertex.get_index() << "[";
            out << "label=<";
            out << "index=" << vertex.get_index() << "<BR/>";
            out << "tuple index=" << vertex.get_tuple_index() << "<BR/>";

            tuple_graph.get_tuple_index_mapper()->to_atom_indices(vertex.get_tuple_index(), combined_atom_indices);
            tuple_graph.get_atom_index_mapper()->inverse_remap_and_separate(combined_atom_indices, fluent_atom_indices, derived_atom_indices);
            const auto fluent_atoms = pddl_factories.get_ground_atoms_from_ids<Fluent>(fluent_atom_indices);
            const auto derived_atoms = pddl_factories.get_ground_atoms_from_ids<Derived>(derived_atom_indices);
            out << "fluent_atoms=" << fluent_atoms << ", derived_atoms=" << derived_atoms << "<BR/>";
            out << "states=[";
            for (size_t i = 0; i < vertex.get_states().size(); ++i)
            {
                const auto& state = vertex.get_states()[i];
                if (i != 0)
                {
                    out << "<BR/>";
                }
                out << std::make_tuple(problem, state, std::cref(pddl_factories));
            }
            out << "]>]\n";
        }
    }
    // 4. Group states with same distance together
    // 5. Tuple edges
    out << "{\n";
    for (const auto& vertex_index : tuple_graph.get_vertex_indices_by_distances().front())
    {
        const auto& vertex = tuple_graph.get_vertices().at(vertex_index);
        out << "Dangling" << vertex.get_index() << "->t" << vertex.get_index() << "\n";
    }
    out << "}\n";
    for (const auto& vertex_indices : tuple_graph.get_vertex_indices_by_distances())
    {
        out << "{\n";
        for (const auto& vertex_index : vertex_indices)
        {
            for (const auto& succ_vertex_index : tuple_graph.get_forward_successors().at(vertex_index))
            {
                out << "t" << vertex_index << "->"
                    << "t" << succ_vertex_index << "\n";
            }
        }
        out << "}\n";
    }
    out << "}\n";  // end digraph

    return out;
}

}
