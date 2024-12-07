/*
 * Copyright (C) 2023 Simon Stahlberg
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

#include "mimir/datasets/global_faithful_abstraction.hpp"

#include "mimir/algorithms/BS_thread_pool.hpp"
#include "mimir/common/equal_to.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/common/timers.hpp"
#include "mimir/search/axiom_evaluators/grounded.hpp"
#include "mimir/search/delete_relaxed_problem_explorator.hpp"

#include <algorithm>
#include <cstdlib>
#include <deque>

size_t std::hash<mimir::GlobalFaithfulAbstractState>::operator()(const mimir::GlobalFaithfulAbstractState& element) const
{
    // Note: we do not use element.get_vertex_index() because it is fa specific
    return mimir::hash_combine(element.get_global_index(), element.get_faithful_abstraction_index(), element.get_faithful_abstraction_vertex_index());
}

namespace mimir
{
/**
 * GlobalFaithfulAbstractState
 */

GlobalFaithfulAbstractState::GlobalFaithfulAbstractState(Index vertex_index,
                                                         Index global_index,
                                                         Index faithful_abstraction_index,
                                                         Index faithful_abstraction_vertex_index) :
    m_vertex_index(vertex_index),
    m_global_index(global_index),
    m_faithful_abstraction_index(faithful_abstraction_index),
    m_faithful_abstraction_vertex_index(faithful_abstraction_vertex_index)
{
}

bool GlobalFaithfulAbstractState::operator==(const GlobalFaithfulAbstractState& other) const
{
    if (this != &other)
    {
        // Note: we do not use m_index because it is fa specific
        return (m_global_index == other.m_global_index) && (m_faithful_abstraction_index == other.m_faithful_abstraction_index)
               && (m_faithful_abstraction_vertex_index == other.m_faithful_abstraction_vertex_index);
    }
    return true;
}

Index GlobalFaithfulAbstractState::get_vertex_index() const { return m_vertex_index; }

Index GlobalFaithfulAbstractState::get_global_index() const { return m_global_index; }

Index GlobalFaithfulAbstractState::get_faithful_abstraction_index() const { return m_faithful_abstraction_index; }

Index GlobalFaithfulAbstractState::get_faithful_abstraction_vertex_index() const { return m_faithful_abstraction_vertex_index; }

/**
 * GlobalFaithfulAbstraction
 */

GlobalFaithfulAbstraction::GlobalFaithfulAbstraction(bool mark_true_goal_literals,
                                                     bool use_unit_cost_one,
                                                     Index index,
                                                     std::shared_ptr<const FaithfulAbstractionList> abstractions,
                                                     GlobalFaithfulAbstractStateList states,
                                                     size_t num_isomorphic_states,
                                                     size_t num_non_isomorphic_states) :
    m_mark_true_goal_literals(mark_true_goal_literals),
    m_use_unit_cost_one(use_unit_cost_one),
    m_index(index),
    m_abstractions(std::move(abstractions)),
    m_states(std::move(states)),
    m_num_isomorphic_states(num_isomorphic_states),
    m_num_non_isomorphic_states(num_non_isomorphic_states),
    m_global_vertex_index_to_vertex_index()
{
    /* Ensure correctness. */

    // Check correct vertex ordering
    for (Index vertex = 0; vertex < get_num_vertices(); ++vertex)
    {
        assert(get_vertices().at(vertex).get_vertex_index() == vertex && "State index does not match its position in the list");
    }

    /* Additional */
    for (const auto& state : m_states)
    {
        m_global_vertex_index_to_vertex_index.emplace(state.get_global_index(), state.get_vertex_index());
    }
}

std::vector<GlobalFaithfulAbstraction>
GlobalFaithfulAbstraction::create(const fs::path& domain_filepath, const std::vector<fs::path>& problem_filepaths, const FaithfulAbstractionsOptions& options)
{
    auto memories =
        std::vector<std::tuple<Problem, std::shared_ptr<PDDLRepositories>, std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>>> {};
    for (const auto& problem_filepath : problem_filepaths)
    {
        auto parser = PDDLParser(domain_filepath, problem_filepath);
        auto delete_relaxed_problem_explorator = DeleteRelaxedProblemExplorator(parser.get_problem(), parser.get_pddl_repositories());
        auto applicable_action_generator = delete_relaxed_problem_explorator.create_grounded_applicable_action_generator();
        auto axiom_evaluator = delete_relaxed_problem_explorator.create_grounded_axiom_evaluator();
        auto state_repository = std::make_shared<StateRepository>(std::dynamic_pointer_cast<IAxiomEvaluator>(axiom_evaluator));
        memories.emplace_back(parser.get_problem(), parser.get_pddl_repositories(), applicable_action_generator, state_repository);
    }

    return GlobalFaithfulAbstraction::create(memories, options);
}

std::vector<GlobalFaithfulAbstraction> GlobalFaithfulAbstraction::create(
    const std::vector<std::tuple<Problem, std::shared_ptr<PDDLRepositories>, std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>>>&
        memories,
    const FaithfulAbstractionsOptions& options)
{
    auto abstractions = std::vector<GlobalFaithfulAbstraction> {};
    auto faithful_abstractions = FaithfulAbstraction::create(memories, options);

    auto certificate_to_global_state = std::unordered_map<std::shared_ptr<const nauty_wrapper::Certificate>,
                                                          GlobalFaithfulAbstractState,
                                                          nauty_wrapper::UniqueCertificateSharedPtrHash,
                                                          nauty_wrapper::UniqueCertificateSharedPtrEqualTo> {};

    // An abstraction is considered relevant, if it contains at least one non-isomorphic state.
    auto relevant_faithful_abstractions = std::make_shared<FaithfulAbstractionList>();
    auto abstraction_index = Index { 0 };

    for (auto& faithful_abstraction : faithful_abstractions)
    {
        auto has_zero_non_isomorphic_states = certificate_to_global_state.count(
            mimir::get_certificate(faithful_abstraction.get_graph().get_vertices().at(faithful_abstraction.get_initial_vertex_index())));

        if (has_zero_non_isomorphic_states)
        {
            // Skip: no non-isomorphic state
            continue;
        }

        auto num_isomorphic_states = 0;
        auto num_non_isomorphic_states = 0;
        auto states = GlobalFaithfulAbstractStateList {};
        for (const auto& state : faithful_abstraction.get_graph().get_vertices())
        {
            // Ensure ordering consistent with state in faithful abstraction.
            assert(state.get_index() == states.size());

            auto it = certificate_to_global_state.find(mimir::get_certificate(state));

            if (it != certificate_to_global_state.end())
            {
                // Copy existing global state data.
                states.emplace_back(state.get_index(),
                                    it->second.get_global_index(),
                                    it->second.get_faithful_abstraction_index(),
                                    it->second.get_faithful_abstraction_vertex_index());
                ++num_isomorphic_states;

                // Ensure that goals remain goals and deadends remain deadends.
                const auto& other_faithful_abstraction = relevant_faithful_abstractions->at(it->second.get_faithful_abstraction_index());
                [[maybe_unused]] const auto& other_state =
                    other_faithful_abstraction.get_graph().get_vertices().at(it->second.get_faithful_abstraction_vertex_index());
                assert(faithful_abstraction.is_goal_vertex(state.get_index()) == other_faithful_abstraction.is_goal_vertex(other_state.get_index()));
                assert(faithful_abstraction.is_deadend_vertex(state.get_index()) == other_faithful_abstraction.is_deadend_vertex(other_state.get_index()));
            }
            else
            {
                // Create a new global state and add it to global mapping.
                certificate_to_global_state.emplace(
                    mimir::get_certificate(state),
                    states.emplace_back(state.get_index(), certificate_to_global_state.size(), abstraction_index, state.get_index()));
                ++num_non_isomorphic_states;
            }
        }

        relevant_faithful_abstractions->push_back(std::move(faithful_abstraction));

        abstractions.push_back(GlobalFaithfulAbstraction(options.fa_options.mark_true_goal_literals,
                                                         options.fa_options.use_unit_cost_one,
                                                         abstraction_index,
                                                         std::const_pointer_cast<const FaithfulAbstractionList>(relevant_faithful_abstractions),
                                                         std::move(states),
                                                         num_isomorphic_states,
                                                         num_non_isomorphic_states));
        ++abstraction_index;
    }

    return abstractions;
}

/**
 * Abstraction functionality
 */

Index GlobalFaithfulAbstraction::get_vertex_index(State state) const { return m_abstractions->at(m_index).get_vertex_index(state); }

Index GlobalFaithfulAbstraction::get_vertex_index(Index global_index) const
{
    if (m_global_vertex_index_to_vertex_index.contains(global_index))
    {
        return m_global_vertex_index_to_vertex_index.at(global_index);
    }
    throw std::runtime_error("Failed to access vertex of global state index. Are you sure that the global vertex index is part of the abstraction?");
}

/**
 * Extended functionality
 */

template<IsTraversalDirection Direction>
ContinuousCostList GlobalFaithfulAbstraction::compute_shortest_distances_from_vertices(const IndexList& vertices) const
{
    return m_abstractions->at(m_index).compute_shortest_distances_from_vertices<Direction>(vertices);
}

template ContinuousCostList GlobalFaithfulAbstraction::compute_shortest_distances_from_vertices<ForwardTraversal>(const IndexList& vertices) const;
template ContinuousCostList GlobalFaithfulAbstraction::compute_shortest_distances_from_vertices<BackwardTraversal>(const IndexList& vertices) const;

template<IsTraversalDirection Direction>
ContinuousCostMatrix GlobalFaithfulAbstraction::compute_pairwise_shortest_vertex_distances() const
{
    return m_abstractions->at(m_index).compute_pairwise_shortest_vertex_distances<Direction>();
}

template ContinuousCostMatrix GlobalFaithfulAbstraction::compute_pairwise_shortest_vertex_distances<ForwardTraversal>() const;
template ContinuousCostMatrix GlobalFaithfulAbstraction::compute_pairwise_shortest_vertex_distances<BackwardTraversal>() const;

/**
 * Getters
 */

/* Meta data */
Problem GlobalFaithfulAbstraction::get_problem() const { return m_abstractions->at(m_index).get_problem(); }

bool GlobalFaithfulAbstraction::get_mark_true_goal_literals() const { return m_mark_true_goal_literals; }

bool GlobalFaithfulAbstraction::get_use_unit_cost_one() const { return m_use_unit_cost_one; }

Index GlobalFaithfulAbstraction::get_index() const { return m_index; }

/* Memory */
const std::shared_ptr<PDDLRepositories>& GlobalFaithfulAbstraction::get_pddl_repositories() const
{
    return m_abstractions->at(m_index).get_pddl_repositories();
}

const std::shared_ptr<IApplicableActionGenerator>& GlobalFaithfulAbstraction::get_applicable_action_generator() const
{
    return m_abstractions->at(m_index).get_applicable_action_generator();
}

const std::shared_ptr<StateRepository>& GlobalFaithfulAbstraction::get_state_repository() const { return m_abstractions->at(m_index).get_state_repository(); }

const FaithfulAbstractionList& GlobalFaithfulAbstraction::get_abstractions() const { return *m_abstractions; }

/* Graph */
const typename GlobalFaithfulAbstraction::GraphType& GlobalFaithfulAbstraction::get_graph() const { return m_abstractions->at(m_index).get_graph(); }

/* States */
const GlobalFaithfulAbstractStateList& GlobalFaithfulAbstraction::get_vertices() const { return m_states; }

template<IsTraversalDirection Direction>
std::ranges::subrange<typename GlobalFaithfulAbstraction::AdjacentVertexConstIteratorType<Direction>>
GlobalFaithfulAbstraction::get_adjacent_vertices(Index vertex) const
{
    return get_graph().get_adjacent_vertices<Direction>(vertex);
}

template std::ranges::subrange<typename GlobalFaithfulAbstraction::AdjacentVertexConstIteratorType<ForwardTraversal>>
GlobalFaithfulAbstraction::get_adjacent_vertices<ForwardTraversal>(Index vertex) const;
template std::ranges::subrange<typename GlobalFaithfulAbstraction::AdjacentVertexConstIteratorType<BackwardTraversal>>
GlobalFaithfulAbstraction::get_adjacent_vertices<BackwardTraversal>(Index vertex) const;

template<IsTraversalDirection Direction>
std::ranges::subrange<typename GlobalFaithfulAbstraction::AdjacentVertexIndexConstIteratorType<Direction>>
GlobalFaithfulAbstraction::get_adjacent_vertex_indices(Index vertex) const
{
    return get_graph().get_adjacent_vertex_indices<Direction>(vertex);
}

template std::ranges::subrange<typename GlobalFaithfulAbstraction::AdjacentVertexIndexConstIteratorType<ForwardTraversal>>
GlobalFaithfulAbstraction::get_adjacent_vertex_indices<ForwardTraversal>(Index vertex) const;
template std::ranges::subrange<typename GlobalFaithfulAbstraction::AdjacentVertexIndexConstIteratorType<BackwardTraversal>>
GlobalFaithfulAbstraction::get_adjacent_vertex_indices<BackwardTraversal>(Index vertex) const;

const StateMap<Index>& GlobalFaithfulAbstraction::get_state_to_vertex_index() const { return m_abstractions->at(m_index).get_state_to_vertex_index(); }

const std::unordered_map<Index, Index>& GlobalFaithfulAbstraction::get_global_vertex_index_to_vertex_index() const
{
    return m_global_vertex_index_to_vertex_index;
}

Index GlobalFaithfulAbstraction::get_initial_vertex_index() const { return m_abstractions->at(m_index).get_initial_vertex_index(); }

const IndexSet& GlobalFaithfulAbstraction::get_goal_vertex_indices() const { return m_abstractions->at(m_index).get_goal_vertex_indices(); }

const IndexSet& GlobalFaithfulAbstraction::get_deadend_vertex_indices() const { return m_abstractions->at(m_index).get_deadend_vertex_indices(); }

size_t GlobalFaithfulAbstraction::get_num_vertices() const { return get_vertices().size(); }

size_t GlobalFaithfulAbstraction::get_num_goal_vertices() const { return get_goal_vertex_indices().size(); }

size_t GlobalFaithfulAbstraction::get_num_deadend_vertices() const { return get_deadend_vertex_indices().size(); }

bool GlobalFaithfulAbstraction::is_goal_vertex(Index vertex) const { return get_goal_vertex_indices().count(vertex); }

bool GlobalFaithfulAbstraction::is_deadend_vertex(Index vertex) const { return get_deadend_vertex_indices().count(vertex); }

bool GlobalFaithfulAbstraction::is_alive_vertex(Index vertex) const
{
    return !(get_goal_vertex_indices().count(vertex) || get_deadend_vertex_indices().count(vertex));
}

size_t GlobalFaithfulAbstraction::get_num_isomorphic_states() const { return m_num_isomorphic_states; }

size_t GlobalFaithfulAbstraction::get_num_non_isomorphic_states() const { return m_num_non_isomorphic_states; }

/* Transitions */
const GroundActionsEdgeList& GlobalFaithfulAbstraction::get_edges() const { return get_graph().get_edges(); }

const GroundActionsEdge& GlobalFaithfulAbstraction::get_edge(Index edge) const { return get_edges().at(edge); }

template<IsTraversalDirection Direction>
std::ranges::subrange<typename GlobalFaithfulAbstraction::AdjacentEdgeConstIteratorType<Direction>>
GlobalFaithfulAbstraction::get_adjacent_edges(Index vertex) const
{
    return get_graph().get_adjacent_edges<Direction>(vertex);
}

template std::ranges::subrange<typename GlobalFaithfulAbstraction::AdjacentEdgeConstIteratorType<ForwardTraversal>>
GlobalFaithfulAbstraction::get_adjacent_edges<ForwardTraversal>(Index vertex) const;
template std::ranges::subrange<typename GlobalFaithfulAbstraction::AdjacentEdgeConstIteratorType<BackwardTraversal>>
GlobalFaithfulAbstraction::get_adjacent_edges<BackwardTraversal>(Index vertex) const;

template<IsTraversalDirection Direction>
std::ranges::subrange<typename GlobalFaithfulAbstraction::AdjacentEdgeIndexConstIteratorType<Direction>>
GlobalFaithfulAbstraction::get_adjacent_edge_indices(Index vertex) const
{
    return get_graph().get_adjacent_edge_indices<Direction>(vertex);
}

template std::ranges::subrange<typename GlobalFaithfulAbstraction::AdjacentEdgeIndexConstIteratorType<ForwardTraversal>>
GlobalFaithfulAbstraction::get_adjacent_edge_indices<ForwardTraversal>(Index vertex) const;
template std::ranges::subrange<typename GlobalFaithfulAbstraction::AdjacentEdgeIndexConstIteratorType<BackwardTraversal>>
GlobalFaithfulAbstraction::get_adjacent_edge_indices<BackwardTraversal>(Index vertex) const;

ContinuousCost GlobalFaithfulAbstraction::get_edge_cost(Index edge) const
{
    return (m_use_unit_cost_one) ? 1 : m_abstractions->at(m_index).get_edge_cost(edge);
}

size_t GlobalFaithfulAbstraction::get_num_edges() const { return m_abstractions->at(m_index).get_num_edges(); }

/* Distances */
const ContinuousCostList& GlobalFaithfulAbstraction::get_goal_distances() const { return m_abstractions->at(m_index).get_goal_distances(); }

ContinuousCost GlobalFaithfulAbstraction::get_goal_distance(Index vertex) const { return m_abstractions->at(m_index).get_goal_distance(vertex); }

/* Additional */
const std::map<ContinuousCost, IndexList>& GlobalFaithfulAbstraction::get_states_by_goal_distance() const
{
    return m_abstractions->at(m_index).get_vertex_indices_by_goal_distance();
}

/**
 * Pretty printing
 */

std::ostream& operator<<(std::ostream& out, const GlobalFaithfulAbstraction& abstraction)
{
    // 2. Header
    out << "digraph {"
        << "\n"
        << "rankdir=\"LR\""
        << "\n";

    // 3. Draw states
    for (size_t state_index = 0; state_index < abstraction.get_num_vertices(); ++state_index)
    {
        out << "s" << state_index << "[";

        // goal marking
        if (abstraction.is_goal_vertex(state_index))
        {
            out << "peripheries=2,";
        }

        // label
        const auto& gfa_state = abstraction.get_vertices().at(state_index);
        out << "label=\"";
        out << "state_index=" << gfa_state.get_vertex_index() << " "
            << "global_state_index = " << gfa_state.get_global_index() << " "
            << "abstraction_index=" << gfa_state.get_faithful_abstraction_index() << " "
            << "abstract_state_index=" << gfa_state.get_faithful_abstraction_vertex_index() << "\n";
        const auto& fa_abstraction = abstraction.get_abstractions().at(gfa_state.get_faithful_abstraction_index());
        for (const auto& state : mimir::get_states(fa_abstraction.get_graph().get_vertices().at(gfa_state.get_faithful_abstraction_vertex_index())))
        {
            out << std::make_tuple(fa_abstraction.get_problem(), state, std::cref(*fa_abstraction.get_pddl_repositories())) << "\n";
        }
        out << "\"";  // end label

        out << "]\n";
    }

    // 4. Draw initial state and dangling edge
    out << "Dangling [ label = \"\", style = invis ]\n"
        << "{ rank = same; Dangling }\n"
        << "Dangling -> s" << abstraction.get_initial_vertex_index() << "\n";

    // 5. Group states with same distance together
    for (auto it = abstraction.get_states_by_goal_distance().rbegin(); it != abstraction.get_states_by_goal_distance().rend(); ++it)
    {
        const auto& [goal_distance, state_indices] = *it;
        out << "{ rank = same; ";
        for (auto state_index : state_indices)
        {
            out << "s" << state_index;
            if (state_index != state_indices.back())
            {
                out << ",";
            }
        }
        out << "}\n";
    }
    // 6. Draw transitions
    for (const auto& transition : abstraction.get_graph().get_edges())
    {
        // direction
        out << "s" << transition.get_source() << "->"
            << "s" << transition.get_target() << " [";

        // label
        out << "label=\"";
        for (const auto& action : get_actions(transition))
        {
            out << std::make_tuple(action, std::cref(*abstraction.get_pddl_repositories()), PlanActionFormatterTag {}) << "\n";
        }
        out << "\"";  // end label

        out << "]\n";
    }
    out << "}\n";

    return out;
}

}
