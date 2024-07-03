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
#include "mimir/common/timers.hpp"

#include <algorithm>
#include <cstdlib>
#include <deque>

namespace mimir
{
/**
 * GlobalFaithfulAbstractState
 */

GlobalFaithfulAbstractState::GlobalFaithfulAbstractState(StateId id, AbstractionIndex abstraction_index, StateId abstract_state_id) :
    m_id(id),
    m_abstraction_index(abstraction_index),
    m_abstract_state_id(abstract_state_id)
{
}

bool GlobalFaithfulAbstractState::operator==(const GlobalFaithfulAbstractState& other) const
{
    if (this != &other)
    {
        // GlobalFaithfulAbstractStates are unique by id within a GlobalFaithfulAbstraction;
        return (m_id == other.m_id);
    }
    return true;
}

size_t GlobalFaithfulAbstractState::hash() const
{
    // GlobalFaithfulAbstractStates are unique by id within a GlobalFaithfulAbstraction;
    return loki::hash_combine(m_id);
}

StateId GlobalFaithfulAbstractState::get_id() const { return m_id; }

AbstractionIndex GlobalFaithfulAbstractState::get_abstraction_index() const { return m_abstraction_index; }

StateId GlobalFaithfulAbstractState::get_abstract_state_id() const { return m_abstract_state_id; }

/**
 * GlobalFaithfulAbstraction
 */

GlobalFaithfulAbstraction::GlobalFaithfulAbstraction(bool mark_true_goal_literals,
                                                     bool use_unit_cost_one,
                                                     AbstractionIndex id,
                                                     std::shared_ptr<FaithfulAbstractionList> abstractions,
                                                     GlobalFaithfulAbstractStateList states,
                                                     GlobalFaithfulAbstractStateMap<StateIndex> state_to_index,
                                                     size_t num_isomorphic_states,
                                                     size_t num_non_isomorphic_states) :
    m_mark_true_goal_literals(mark_true_goal_literals),
    m_use_unit_cost_one(use_unit_cost_one),
    m_index(id),
    m_abstractions(std::move(abstractions)),
    m_states(std::move(states)),
    m_state_to_index(std::move(state_to_index)),
    m_num_isomorphic_states(num_isomorphic_states),
    m_num_non_isomorphic_states(num_non_isomorphic_states),
    m_nauty_graph(),
    m_object_graph_factory(m_abstractions->at(m_index).get_pddl_parser()->get_problem(),
                           m_abstractions->at(m_index).get_pddl_parser()->get_factories(),
                           m_mark_true_goal_literals)
{
}

std::vector<GlobalFaithfulAbstraction> GlobalFaithfulAbstraction::create(const fs::path& domain_filepath,
                                                                         const std::vector<fs::path>& problem_filepaths,
                                                                         bool mark_true_goal_literals,
                                                                         bool use_unit_cost_one,
                                                                         bool remove_if_unsolvable,
                                                                         bool compute_complete_abstraction_mapping,
                                                                         bool sort_ascending_by_num_states,
                                                                         uint32_t max_num_states,
                                                                         uint32_t timeout_ms,
                                                                         uint32_t num_threads)
{
    auto memories = std::vector<std::tuple<std::shared_ptr<PDDLParser>, std::shared_ptr<IAAG>, std::shared_ptr<SuccessorStateGenerator>>> {};
    for (const auto& problem_filepath : problem_filepaths)
    {
        auto parser = std::make_shared<PDDLParser>(domain_filepath, problem_filepath);
        auto aag = std::make_shared<GroundedAAG>(parser->get_problem(), parser->get_factories());
        auto ssg = std::make_shared<SuccessorStateGenerator>(aag);
        memories.emplace_back(std::move(parser), std::move(aag), std::move(ssg));
    }

    return GlobalFaithfulAbstraction::create(memories,
                                             mark_true_goal_literals,
                                             use_unit_cost_one,
                                             remove_if_unsolvable,
                                             compute_complete_abstraction_mapping,
                                             sort_ascending_by_num_states,
                                             max_num_states,
                                             timeout_ms,
                                             num_threads);
}

std::vector<GlobalFaithfulAbstraction> GlobalFaithfulAbstraction::create(
    const std::vector<std::tuple<std::shared_ptr<PDDLParser>, std::shared_ptr<IAAG>, std::shared_ptr<SuccessorStateGenerator>>>& memories,
    bool mark_true_goal_literals,
    bool use_unit_cost_one,
    bool remove_if_unsolvable,
    bool compute_complete_abstraction_mapping,
    bool sort_ascending_by_num_states,
    uint32_t max_num_states,
    uint32_t timeout_ms,
    uint32_t num_threads)
{
    auto abstractions = std::vector<GlobalFaithfulAbstraction> {};
    auto faithful_abstractions = FaithfulAbstraction::create(memories,
                                                             mark_true_goal_literals,
                                                             use_unit_cost_one,
                                                             remove_if_unsolvable,
                                                             compute_complete_abstraction_mapping,
                                                             sort_ascending_by_num_states,
                                                             max_num_states,
                                                             timeout_ms,
                                                             num_threads);

    auto certificate_to_global_state = std::unordered_map<Certificate, GlobalFaithfulAbstractState, loki::Hash<Certificate>, loki::EqualTo<Certificate>> {};

    // An abstraction is considered relevant, if it contains at least one non-isomorphic state.
    auto relevant_faithful_abstractions = std::make_shared<FaithfulAbstractionList>();
    auto abstraction_id = AbstractionIndex { 0 };

    for (auto& faithful_abstraction : faithful_abstractions)
    {
        auto max_goal_distance = *std::max_element(faithful_abstraction.get_goal_distances().begin(), faithful_abstraction.get_goal_distances().end());

        auto is_relevant = false;
        for (size_t state_id = 0; state_id < faithful_abstraction.get_num_states(); ++state_id)
        {
            const auto& state = faithful_abstraction.get_states().at(state_id);
            if (faithful_abstraction.get_goal_distances().at(state_id) == max_goal_distance  //
                && !certificate_to_global_state.count(state.get_certificate()))
            {
                is_relevant = true;
                break;
            }
        }

        if (!is_relevant)
        {
            // Skip: no non-isomorphic state
            continue;
        }

        auto num_isomorphic_states = 0;
        auto num_non_isomorphic_states = 0;
        auto states = GlobalFaithfulAbstractStateList(
            faithful_abstraction.get_num_states(),
            GlobalFaithfulAbstractState(std::numeric_limits<StateId>::max(), std::numeric_limits<StateId>::max(), std::numeric_limits<StateId>::max()));
        auto state_to_index = GlobalFaithfulAbstractStateMap<StateIndex> {};
        for (size_t state_id = 0; state_id < faithful_abstraction.get_num_states(); ++state_id)
        {
            const auto& state = faithful_abstraction.get_states().at(state_id);
            auto it = certificate_to_global_state.find(state.get_certificate());

            if (it != certificate_to_global_state.end())
            {
                // Copy existing global state
                states.at(state_id) = it->second;
                state_to_index.emplace(it->second, state_id);
                ++num_isomorphic_states;
            }
            else
            {
                // Create new global state
                const auto new_global_state_id = certificate_to_global_state.size();
                auto new_global_state = GlobalFaithfulAbstractState(new_global_state_id, abstraction_id, state_id);
                certificate_to_global_state.emplace(state.get_certificate(), new_global_state);
                states.at(state_id) = new_global_state;
                state_to_index.emplace(new_global_state, state_id);
                ++num_non_isomorphic_states;
            }
        }

        // Ensure that all global states were correctly initialized.
        assert(std::all_of(states.begin(),
                           states.end(),
                           [](const GlobalFaithfulAbstractState& global_state)
                           {
                               return global_state.get_id() != std::numeric_limits<StateId>::max()
                                      && global_state.get_abstraction_index() != std::numeric_limits<StateId>::max()
                                      && global_state.get_abstract_state_id() != std::numeric_limits<StateId>::max();
                           }));

        // Constructor of GlobalFaithfulAbstraction requires this to come first.
        relevant_faithful_abstractions->push_back(std::move(faithful_abstraction));

        abstractions.push_back(GlobalFaithfulAbstraction(mark_true_goal_literals,
                                                         use_unit_cost_one,
                                                         abstraction_id,
                                                         relevant_faithful_abstractions,
                                                         std::move(states),
                                                         std::move(state_to_index),
                                                         num_isomorphic_states,
                                                         num_non_isomorphic_states));
        ++abstraction_id;
    }

    return abstractions;
}

/**
 * Abstraction functionality
 */

StateIndex GlobalFaithfulAbstraction::get_abstract_state_index(State concrete_state)
{
    // Cheap test.
    if (get_concrete_to_abstract_state().count(concrete_state))
    {
        return get_concrete_to_abstract_state().at(concrete_state);
    }

    // Expensive test.
    const auto& object_graph = m_object_graph_factory.create(concrete_state);
    object_graph.get_digraph().to_nauty_graph(m_nauty_graph);
    return get_states_by_certificate().at(Certificate(
        m_nauty_graph.compute_certificate(object_graph.get_partitioning().get_vertex_index_permutation(), object_graph.get_partitioning().get_partitioning()),
        object_graph.get_sorted_vertex_colors()));
}

/**
 * Extended functionality
 */

std::vector<double> GlobalFaithfulAbstraction::compute_shortest_distances_from_states(const StateIndexList& abstract_states, bool forward) const
{
    throw std::runtime_error("Not implemented");
}

std::vector<std::vector<double>> GlobalFaithfulAbstraction::compute_pairwise_shortest_state_distances(bool forward) const
{
    throw std::runtime_error("Not implemented");
}

/**
 * Getters
 */

/* Meta data */
bool GlobalFaithfulAbstraction::get_mark_true_goal_literals() const { return m_mark_true_goal_literals; }

bool GlobalFaithfulAbstraction::get_use_unit_cost_one() const { return m_use_unit_cost_one; }

AbstractionIndex GlobalFaithfulAbstraction::get_index() const { return m_index; }

/* Memory */
const std::shared_ptr<PDDLParser>& GlobalFaithfulAbstraction::get_pddl_parser() const { return m_abstractions->at(m_index).get_pddl_parser(); }

const std::shared_ptr<IAAG>& GlobalFaithfulAbstraction::get_aag() const { return m_abstractions->at(m_index).get_aag(); }

const std::shared_ptr<SuccessorStateGenerator>& GlobalFaithfulAbstraction::get_ssg() const { return m_abstractions->at(m_index).get_ssg(); }

const FaithfulAbstractionList& GlobalFaithfulAbstraction::get_abstractions() const { return *m_abstractions; }

/* States */
const GlobalFaithfulAbstractStateList& GlobalFaithfulAbstraction::get_states() const { return m_states; }

StateIndex GlobalFaithfulAbstraction::get_state_index(const GlobalFaithfulAbstractState& state) const { return m_state_to_index.at(state); }

const StateMap<StateIndex>& GlobalFaithfulAbstraction::get_concrete_to_abstract_state() const
{
    return m_abstractions->at(m_index).get_concrete_to_abstract_state();
}

const CertificateToStateIndexMap& GlobalFaithfulAbstraction::get_states_by_certificate() const
{
    return m_abstractions->at(m_index).get_states_by_certificate();
}

StateIndex GlobalFaithfulAbstraction::get_initial_state() const { return m_abstractions->at(m_index).get_initial_state(); }

const StateIndexSet& GlobalFaithfulAbstraction::get_goal_states() const { return m_abstractions->at(m_index).get_goal_states(); }

const StateIndexSet& GlobalFaithfulAbstraction::get_deadend_states() const { return m_abstractions->at(m_index).get_deadend_states(); }

const std::vector<StateIndexList>& GlobalFaithfulAbstraction::get_forward_successor_adjacency_lists() const
{
    return m_abstractions->at(m_index).get_forward_successor_adjacency_lists();
}

const std::vector<StateIndexList>& GlobalFaithfulAbstraction::get_backward_successor_adjacency_lists() const
{
    return m_abstractions->at(m_index).get_backward_successor_adjacency_lists();
}

size_t GlobalFaithfulAbstraction::get_num_states() const { return get_states().size(); }

size_t GlobalFaithfulAbstraction::get_num_goal_states() const { return get_goal_states().size(); }

size_t GlobalFaithfulAbstraction::get_num_deadend_states() const { return get_deadend_states().size(); }

bool GlobalFaithfulAbstraction::is_goal_state(StateIndex state) const { return get_goal_states().count(state); }

bool GlobalFaithfulAbstraction::is_deadend_state(StateIndex state) const { return get_deadend_states().count(state); }

bool GlobalFaithfulAbstraction::is_alive_state(StateIndex state) const { return !(get_goal_states().count(state) || get_deadend_states().count(state)); }

size_t GlobalFaithfulAbstraction::get_num_isomorphic_states() const { return m_num_isomorphic_states; }

size_t GlobalFaithfulAbstraction::get_num_non_isomorphic_states() const { return m_num_non_isomorphic_states; }

/* Transitions */
const TransitionList& GlobalFaithfulAbstraction::get_transitions() const { return m_abstractions->at(m_index).get_transitions(); }

TransitionCost GlobalFaithfulAbstraction::get_transition_cost(TransitionIndex transition) const
{
    return (m_use_unit_cost_one) ? 1 : m_abstractions->at(m_index).get_transition_cost(transition);
}

const std::vector<TransitionIndexList>& GlobalFaithfulAbstraction::get_forward_transition_adjacency_lists() const
{
    return m_abstractions->at(m_index).get_forward_transition_adjacency_lists();
}

const std::vector<TransitionIndexList>& GlobalFaithfulAbstraction::get_backward_transition_adjacency_lists() const
{
    return m_abstractions->at(m_index).get_backward_transition_adjacency_lists();
}

size_t GlobalFaithfulAbstraction::get_num_transitions() const { return m_abstractions->at(m_index).get_num_transitions(); }

/* Distances */
const std::vector<double>& GlobalFaithfulAbstraction::get_goal_distances() const { return m_abstractions->at(m_index).get_goal_distances(); }

}
