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

using TupleVertexIndexSet = std::unordered_set<TupleVertexIndex>;

/**
 * TupleGraphVertex
 */

TupleGraphVertex::TupleGraphVertex(TupleVertexIndex index, TupleIndex tuple_index, StateList states) :
    m_index(index),
    m_tuple_index(tuple_index),
    m_states(std::move(states))
{
}

TupleVertexIndex TupleGraphVertex::get_index() const { return m_index; }

TupleIndex TupleGraphVertex::get_tuple_index() const { return m_tuple_index; }

const StateList& TupleGraphVertex::get_states() const { return m_states; }

/**
 * TupleGraph
 */

TupleGraph::TupleGraph(std::shared_ptr<StateSpace> state_space,
                       std::shared_ptr<TupleIndexMapper> tuple_index_mapper,
                       bool prune_dominated_tuples,
                       StaticDigraph digraph,
                       IndexGroupedVector<const TupleGraphVertex> vertices_grouped_by_distance,
                       IndexGroupedVector<const State> states_grouped_by_distance) :
    m_state_space(std::move(state_space)),
    m_tuple_index_mapper(std::move(tuple_index_mapper)),
    m_prune_dominated_tuples(prune_dominated_tuples),
    m_digraph(std::move(digraph)),
    m_vertices_grouped_by_distance(std::move(vertices_grouped_by_distance)),
    m_states_grouped_by_distance(std::move(states_grouped_by_distance))
{
}

std::optional<TupleVertexIndexList> TupleGraph::compute_admissible_chain(const GroundAtomList<Fluent>& fluent_atoms)
{
    // Find all states that satisfy the given set of atoms, then call more general function.
    auto states = StateList {};
    auto fluent_atom_bitset = FlatBitset();
    for (const auto& atom : fluent_atoms)
    {
        fluent_atom_bitset.set(atom->get_index());
    }
    for (const auto group : m_states_grouped_by_distance)
    {
        for (const auto& state : group)
        {
            if (state->get_atoms<Fluent>().is_superseteq(fluent_atom_bitset))
            {
                states.push_back(state);
            }
        }
    }
    return compute_admissible_chain(states);
}

std::optional<TupleVertexIndexList> TupleGraph::compute_admissible_chain(const StateList& states)
{
    if (states.empty())
    {
        // No tuple can optimally goal imply an empty set of states.
        return std::nullopt;
    }

    auto optimal_distance = std::numeric_limits<ContinuousCost>::infinity();
    const auto distances = m_state_space->compute_shortest_distances_from_vertices<ForwardTraversal>(IndexList { m_state_space->get_initial_vertex_index() });
    for (const auto& state : states)
    {
        const auto state_index = m_state_space->get_vertex_index(state);
        const auto state_distance = distances.at(state_index);
        // Unreachable states have distance DISTANCE_INFINITY
        if (state_distance != std::numeric_limits<ContinuousCost>::infinity())
        {
            optimal_distance = std::min(optimal_distance, state_distance);
        }
    }

    auto optimal_states = StateSet {};
    for (const auto& state : states)
    {
        const auto state_index = m_state_space->get_vertex_index(state);
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

    for (const auto& vertex_group : m_vertices_grouped_by_distance)
    {
        for (const auto& vertex : vertex_group)
        {
            // Part 3 of definition of width
            const auto tuple_optimally_goal_implies_states =
                std::all_of(vertex.get_states().begin(),
                            vertex.get_states().end(),
                            [&optimal_states](const auto& tuple_state) { return optimal_states.count(tuple_state); });

            if (tuple_optimally_goal_implies_states)
            {
                // Backtrack admissible chain until the root and return an admissible chain that proves the width.
                auto cur_vertex_index = vertex.get_index();
                auto admissible_chain = TupleVertexIndexList { cur_vertex_index };
                while (m_digraph.get_adjacent_vertices<BackwardTraversal>(cur_vertex_index).begin()
                       != m_digraph.get_adjacent_vertices<BackwardTraversal>(cur_vertex_index).end())
                {
                    cur_vertex_index = (*m_digraph.get_adjacent_vertices<BackwardTraversal>(cur_vertex_index).begin()).get_index();
                    admissible_chain.push_back(cur_vertex_index);
                }
                std::reverse(admissible_chain.begin(), admissible_chain.end());
                return admissible_chain;
            }
        }
    }
    return std::nullopt;
}

const std::shared_ptr<StateSpace>& TupleGraph::get_state_space() const { return m_state_space; }

const std::shared_ptr<TupleIndexMapper>& TupleGraph::get_tuple_index_mapper() const { return m_tuple_index_mapper; }

State TupleGraph::get_root_state() const { return m_states_grouped_by_distance.front().front(); }

const StaticDigraph& TupleGraph::get_digraph() const { return m_digraph; }

const IndexGroupedVector<const TupleGraphVertex>& TupleGraph::get_vertices_grouped_by_distance() const { return m_vertices_grouped_by_distance; }

const IndexGroupedVector<const State>& TupleGraph::get_states_grouped_by_distance() const { return m_states_grouped_by_distance; }

/**
 * TupleGraphFactory
 */

class TupleGraphArityZeroComputation
{
private:
    // Input
    std::shared_ptr<StateSpace> m_state_space;
    std::shared_ptr<TupleIndexMapper> m_tuple_index_mapper;
    bool m_prune_dominated_tuples;

    // Result structures to create tuple graph
    StaticDigraph m_digraph;
    IndexGroupedVectorBuilder<const TupleGraphVertex> m_vertices_grouped_by_distance;
    IndexGroupedVectorBuilder<const State> m_states_grouped_by_distance;

public:
    TupleGraphArityZeroComputation(std::shared_ptr<StateSpace> state_space, std::shared_ptr<TupleIndexMapper> tuple_index_mapper, bool prune_dominated_tuples);

    /// @brief Compute the root state layer.
    void compute_root_state_layer(State root_state);

    /// @brief Compute the layer at distance 1, assumes that the root state layer exists.
    void compute_first_layer(State root_state);

    /// @brief Return a reference to the tuple graph.
    TupleGraph get_result();
};

// Bookkeeping for memory reuse when building tuple graph of width greater 0
class TupleGraphArityKComputation
{
private:
    // Input
    std::shared_ptr<StateSpace> m_state_space;
    std::shared_ptr<TupleIndexMapper> m_tuple_index_mapper;
    bool m_prune_dominated_tuples;

    // Result structures to create tuple graph
    StaticDigraph m_digraph;
    IndexGroupedVectorBuilder<const TupleGraphVertex> m_vertices_grouped_by_distance;
    IndexGroupedVectorBuilder<const State> m_states_grouped_by_distance;

    // Common book-keeping
    StateList prev_states;
    StateList curr_states;
    TupleGraphVertexList prev_vertices;
    TupleGraphVertexList curr_vertices;
    StateSet visited_states;
    DynamicNoveltyTable novelty_table;

    /**
     * Four step procedure to compute the next layer in the graph.
     */

    bool compute_next_state_layer();

    TupleIndexSet novel_tuple_indices_set;
    TupleIndexList novel_tuple_indices;
    std::unordered_map<TupleIndex, StateSet> novel_tuple_index_to_states;
    StateMap<TupleIndexList> state_to_novel_tuple_indices;

    void compute_next_novel_tuple_indices();

    std::unordered_map<TupleIndex, StateSet> cur_novel_tuple_index_to_extended_state;
    std::unordered_map<TupleIndex, TupleVertexIndexSet> cur_extended_novel_tuple_index_to_prev_vertices;
    TupleIndexSet cur_extended_novel_tuple_indices_set;
    TupleIndexList cur_extended_novel_tuple_indices;

    void extend_optimal_plans_from_prev_layer();

    std::unordered_map<TupleIndex, TupleIndexSet> tuple_index_to_dominating_tuple_indices;

    bool instantiate_next_layer();

public:
    TupleGraphArityKComputation(std::shared_ptr<StateSpace> state_space, std::shared_ptr<TupleIndexMapper> tuple_index_mapper, bool prune_dominated_tuples);

    /// @brief Compute the root state layer.
    void compute_root_state_layer(State root_state);

    /// @brief Compute the next layer, assumes that the root state layer exists
    /// and return true iff the layer is nonempty.
    bool compute_next_layer();

    /// @brief Return a reference to the tuple graph.
    TupleGraph get_result();
};

TupleGraphArityZeroComputation::TupleGraphArityZeroComputation(std::shared_ptr<StateSpace> state_space,
                                                               std::shared_ptr<TupleIndexMapper> tuple_index_mapper,
                                                               bool prune_dominated_tuples) :
    m_state_space(std::move(state_space)),
    m_tuple_index_mapper(std::move(tuple_index_mapper)),
    m_prune_dominated_tuples(prune_dominated_tuples)
{
}

void TupleGraphArityZeroComputation::compute_root_state_layer(State root_state)
{
    m_vertices_grouped_by_distance.start_group();
    m_states_grouped_by_distance.start_group();

    const auto empty_tuple_index = m_tuple_index_mapper->get_empty_tuple_index();
    const auto root_state_vertex_index = m_digraph.add_vertex();
    assert(root_state_vertex_index == 0);
    m_vertices_grouped_by_distance.add_group_element(TupleGraphVertex(root_state_vertex_index, empty_tuple_index, StateList { root_state }));
    m_states_grouped_by_distance.add_group_element(root_state);
}

void TupleGraphArityZeroComputation::compute_first_layer(State root_state)
{
    m_vertices_grouped_by_distance.start_group();
    m_states_grouped_by_distance.start_group();

    const auto empty_tuple_index = m_tuple_index_mapper->get_empty_tuple_index();
    const auto root_state_vertex_index = 0;
    const auto root_state_index = m_state_space->get_vertex_index(root_state);
    for (const auto& concrete_succ_state : m_state_space->get_graph().get_adjacent_vertices<ForwardTraversal>(root_state_index))
    {
        const auto succ_state = get_state(concrete_succ_state);
        if (succ_state == root_state)
        {
            // Root state was already visited
            continue;
        }
        const auto succ_state_vertex_index = m_digraph.add_vertex();
        m_vertices_grouped_by_distance.add_group_element(TupleGraphVertex(succ_state_vertex_index, empty_tuple_index, StateList { succ_state }));
        m_states_grouped_by_distance.add_group_element(succ_state);
        m_digraph.add_directed_edge(root_state_vertex_index, succ_state_vertex_index);
    }
}

TupleGraph TupleGraphArityZeroComputation::get_result()
{
    return TupleGraph(m_state_space,
                      m_tuple_index_mapper,
                      m_prune_dominated_tuples,
                      m_digraph,
                      m_vertices_grouped_by_distance.get_result(),
                      m_states_grouped_by_distance.get_result());
}

TupleGraphArityKComputation::TupleGraphArityKComputation(std::shared_ptr<StateSpace> state_space,
                                                         std::shared_ptr<TupleIndexMapper> tuple_index_mapper,
                                                         bool prune_dominated_tuples) :
    m_state_space(std::move(state_space)),
    m_tuple_index_mapper(std::move(tuple_index_mapper)),
    m_prune_dominated_tuples(prune_dominated_tuples),
    m_digraph(),
    m_vertices_grouped_by_distance(),
    m_states_grouped_by_distance(),
    prev_states(),
    curr_states(),
    prev_vertices(),
    curr_vertices(),
    visited_states(),
    novelty_table(m_tuple_index_mapper),
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

void TupleGraphArityKComputation::compute_root_state_layer(State root_state)
{
    m_vertices_grouped_by_distance.start_group();
    m_states_grouped_by_distance.start_group();

    novelty_table.compute_novel_tuple_indices(root_state, novel_tuple_indices);

    for (const auto& novel_tuple_index : novel_tuple_indices)
    {
        const auto vertex_index = m_digraph.add_vertex();
        m_vertices_grouped_by_distance.add_group_element(curr_vertices.emplace_back(vertex_index, novel_tuple_index, StateList { root_state }));

        if (m_prune_dominated_tuples)
        {
            // Break after the first tuple
            break;
        }
    }
    novelty_table.insert_tuple_indices(novel_tuple_indices);

    m_states_grouped_by_distance.add_group_element(root_state);
    curr_states.clear();
    curr_states.push_back(root_state);
    visited_states.insert(root_state);
}

bool TupleGraphArityKComputation::compute_next_state_layer()
{
    bool is_empty = true;
    for (const auto& state : prev_states)
    {
        const auto state_index = m_state_space->get_vertex_index(state);
        for (const auto& concrete_succ_state : m_state_space->get_graph().get_adjacent_vertices<ForwardTraversal>(state_index))
        {
            const auto succ_state = get_state(concrete_succ_state);
            if (!visited_states.count(succ_state))
            {
                curr_states.push_back(succ_state);
                visited_states.insert(succ_state);
                is_empty = false;
            }
        }
    }
    if (is_empty)
    {
        return true;
    }

    return false;
}

void TupleGraphArityKComputation::compute_next_novel_tuple_indices()
{
    // Clear data structures
    novel_tuple_indices_set.clear();
    novel_tuple_indices.clear();
    novel_tuple_index_to_states.clear();
    state_to_novel_tuple_indices.clear();

    for (const auto& state : curr_states)
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

void TupleGraphArityKComputation::extend_optimal_plans_from_prev_layer()
{
    // Clear data structures
    cur_extended_novel_tuple_indices_set.clear();
    cur_extended_novel_tuple_indices.clear();
    cur_extended_novel_tuple_index_to_prev_vertices.clear();

    // Part 2 of definition of width: Check whether all optimal plans for tuple t_{i-1}
    // can be extended into an optimal plan for tuple t_i by means of a single action.
    for (const auto& prev_vertex : prev_vertices)
    {
        cur_novel_tuple_index_to_extended_state.clear();

        // Bookkeeping..
        for (const auto state : prev_vertex.get_states())
        {
            const auto state_index = m_state_space->get_vertex_index(state);

            // "[...] by means of a single action".
            for (const auto& concrete_succ_state : m_state_space->get_graph().get_adjacent_vertices<ForwardTraversal>(state_index))
            {
                const auto succ_state = get_state(concrete_succ_state);

                if (state_to_novel_tuple_indices.count(succ_state))
                {
                    for (const auto target_tuple_index : state_to_novel_tuple_indices.at(succ_state))
                    {
                        cur_novel_tuple_index_to_extended_state[target_tuple_index].insert(state);
                    }
                }
            }
        }

        // "Check whether all optimal plans for tuple t_{i-1}
        // can be extended into an optimal plan for tuple t_i [...]""
        for (const auto& [cur_novel_tuple_index, extended_states] : cur_novel_tuple_index_to_extended_state)
        {
            bool all_optimal_plans_extended = (extended_states.size() == prev_vertex.get_states().size());

            if (all_optimal_plans_extended)
            {
                cur_extended_novel_tuple_indices_set.insert(cur_novel_tuple_index);
                cur_extended_novel_tuple_index_to_prev_vertices[cur_novel_tuple_index].insert(prev_vertex.get_index());
            }
        }
    }
    cur_extended_novel_tuple_indices.insert(cur_extended_novel_tuple_indices.end(),
                                            cur_extended_novel_tuple_indices_set.begin(),
                                            cur_extended_novel_tuple_indices_set.end());
}

bool TupleGraphArityKComputation::instantiate_next_layer()
{
    // Clear data structures
    tuple_index_to_dominating_tuple_indices.clear();

    if (m_prune_dominated_tuples)
    {
        for (size_t i = 0; i < cur_extended_novel_tuple_indices.size(); ++i)
        {
            const auto tuple_index_1 = cur_extended_novel_tuple_indices.at(i);

            const auto& states_1 = novel_tuple_index_to_states.at(tuple_index_1);

            for (size_t j = 0; j < cur_extended_novel_tuple_indices.size(); ++j)
            {
                const auto tuple_index_2 = cur_extended_novel_tuple_indices.at(j);

                const auto& states_2 = novel_tuple_index_to_states.at(tuple_index_2);

                if (i < j && states_1 == states_2)
                {
                    // Keep only smallest tuple_index with a specific set of underlying states.
                    cur_extended_novel_tuple_indices_set.erase(tuple_index_2);
                    continue;
                }

                if (states_1.size() == states_2.size())
                {
                    // Cannot be strict subset.
                    continue;
                }

                const auto is_subset = std::all_of(states_2.begin(), states_2.end(), [&states_1](const auto& element) { return states_1.count(element); });
                if (is_subset)
                {
                    // tuple_index_1 is dominated by tuple_index_2 because states_2 < states_1.
                    tuple_index_to_dominating_tuple_indices[tuple_index_2].insert(tuple_index_1);
                }
            }
        }

        // Keep only tuple indices whose underlying states is a smallest subset.
        for (const auto& [dominating_tuple_index, dominated_tuple_indices] : tuple_index_to_dominating_tuple_indices)
        {
            for (const auto& dominated_tuple_index : dominated_tuple_indices)
            {
                cur_extended_novel_tuple_indices_set.erase(dominated_tuple_index);
            }
        }
    }

    if (cur_extended_novel_tuple_indices_set.size() == 0)
    {
        return true;
    }

    for (const auto& tuple_index : cur_extended_novel_tuple_indices_set)
    {
        const auto cur_vertex_index = m_digraph.add_vertex();
        const auto& cur_states = novel_tuple_index_to_states.at(tuple_index);
        curr_vertices.emplace_back(cur_vertex_index, tuple_index, StateList(cur_states.begin(), cur_states.end()));

        for (const auto prev_vertex_index : cur_extended_novel_tuple_index_to_prev_vertices[tuple_index])
        {
            m_digraph.add_directed_edge(prev_vertex_index, cur_vertex_index);
        }
    }

    return false;
}

bool TupleGraphArityKComputation::compute_next_layer()
{
    // Swap prev and curr data structures.
    std::swap(curr_states, prev_states);
    std::swap(curr_vertices, prev_vertices);
    curr_states.clear();
    curr_vertices.clear();

    {
        const auto is_empty = compute_next_state_layer();
        if (is_empty)
        {
            return false;
        }
    }

    {
        compute_next_novel_tuple_indices();
        if (novel_tuple_indices.empty())
        {
            return false;
        }
    }

    {
        extend_optimal_plans_from_prev_layer();
        if (cur_extended_novel_tuple_indices_set.empty())
        {
            return false;
        }
    }

    {
        const auto is_empty = instantiate_next_layer();
        if (is_empty)
        {
            return false;
        }
    }

    // Create nonempty group.
    m_states_grouped_by_distance.start_group();
    for (const auto& state : curr_states)
    {
        m_states_grouped_by_distance.add_group_element(state);
    }
    m_vertices_grouped_by_distance.start_group();
    for (const auto& vertex : curr_vertices)
    {
        m_vertices_grouped_by_distance.add_group_element(vertex);
    }

    return true;
}

TupleGraph TupleGraphArityKComputation::get_result()
{
    return TupleGraph(m_state_space,
                      m_tuple_index_mapper,
                      m_prune_dominated_tuples,
                      m_digraph,
                      m_vertices_grouped_by_distance.get_result(),
                      m_states_grouped_by_distance.get_result());
}

TupleGraph TupleGraphFactory::create_for_arity_zero(State root_state)
{
    auto arity_zero_computation = TupleGraphArityZeroComputation(m_state_space, m_tuple_index_mapper, m_prune_dominated_tuples);

    arity_zero_computation.compute_root_state_layer(root_state);

    arity_zero_computation.compute_first_layer(root_state);

    return arity_zero_computation.get_result();
}

TupleGraph TupleGraphFactory::create_for_arity_k(State root_state)
{
    auto arity_k_computation = TupleGraphArityKComputation(m_state_space, m_tuple_index_mapper, m_prune_dominated_tuples);

    arity_k_computation.compute_root_state_layer(root_state);

    while (true)
    {
        auto is_empty = arity_k_computation.compute_next_layer();

        if (!is_empty)
        {
            break;
        }
    }

    return arity_k_computation.get_result();
}

TupleGraphFactory::TupleGraphFactory(std::shared_ptr<StateSpace> state_space, int arity, bool prune_dominated_tuples) :
    m_state_space(std::move(state_space)),
    m_tuple_index_mapper(std::make_shared<TupleIndexMapper>(
        arity,
        boost::hana::at_key(m_state_space->get_applicable_action_generator()->get_action_grounder().get_pddl_repositories()->get_pddl_type_to_factory(),
                            boost::hana::type<GroundAtomImpl<Fluent>> {})
                .size()
            + boost::hana::at_key(m_state_space->get_applicable_action_generator()->get_action_grounder().get_pddl_repositories()->get_pddl_type_to_factory(),
                                  boost::hana::type<GroundAtomImpl<Derived>> {})
                  .size())),
    m_prune_dominated_tuples(prune_dominated_tuples)
{
}

TupleGraph TupleGraphFactory::create(State root_state)
{
    return (m_tuple_index_mapper->get_arity() > 0) ? create_for_arity_k(root_state) : create_for_arity_zero(root_state);
}

const std::shared_ptr<StateSpace>& TupleGraphFactory::get_state_space() const { return m_state_space; }

const std::shared_ptr<TupleIndexMapper>& TupleGraphFactory::get_tuple_index_mapper() const { return m_tuple_index_mapper; }

std::ostream& operator<<(std::ostream& out, const TupleGraph& tuple_graph)
{
    const auto problem = tuple_graph.get_state_space()->get_problem();
    const auto& pddl_repositories = *tuple_graph.get_state_space()->get_pddl_repositories();
    auto atom_indices = AtomIndexList {};

    out << "digraph {\n"
        << "rankdir=\"LR\""
        << "\n";

    // 3. Tuple nodes.
    for (const auto& vertex : tuple_graph.get_vertices_grouped_by_distance().front())
    {
        out << "Dangling" << vertex.get_index() << " [ label = \"\", style = invis ]\n";
    }

    for (const auto group : tuple_graph.get_vertices_grouped_by_distance())
    {
        for (const auto& vertex : group)
        {
            out << "t" << vertex.get_index() << "[";
            out << "label=<";
            out << "index=" << vertex.get_index() << "<BR/>";
            out << "tuple index=" << vertex.get_tuple_index() << "<BR/>";

            tuple_graph.get_tuple_index_mapper()->to_atom_indices(vertex.get_tuple_index(), atom_indices);
            const auto fluent_atoms = pddl_repositories.get_ground_atoms_from_indices<Fluent>(atom_indices);
            out << "fluent_atoms=" << fluent_atoms << "<BR/>";
            out << "states=[";
            for (size_t i = 0; i < vertex.get_states().size(); ++i)
            {
                const auto& state = vertex.get_states()[i];
                if (i != 0)
                {
                    out << "<BR/>";
                }
                out << std::make_tuple(problem, state, std::cref(pddl_repositories));
            }
            out << "]>]\n";
        }
    }
    // 4. Group states with same distance together
    // 5. Tuple edges
    out << "{\n";
    for (const auto& vertex : tuple_graph.get_vertices_grouped_by_distance().front())
    {
        out << "Dangling" << vertex.get_index() << "->t" << vertex.get_index() << "\n";
    }
    out << "}\n";
    for (const auto group : tuple_graph.get_vertices_grouped_by_distance())
    {
        out << "{\n";
        for (const auto& vertex : group)
        {
            for (const auto& succ_vertex : tuple_graph.get_digraph().get_adjacent_vertices<ForwardTraversal>(vertex.get_index()))
            {
                out << "t" << vertex.get_index() << "->"
                    << "t" << succ_vertex.get_index() << "\n";
            }
        }
        out << "}\n";
    }
    out << "}\n";  // end digraph

    return out;
}
}
