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

GlobalFaithfulAbstractState::GlobalFaithfulAbstractState(StateIndex index,
                                                         StateIndex global_index,
                                                         AbstractionIndex faithful_abstraction_index,
                                                         StateIndex faithful_abstract_state_index) :
    m_index(index),
    m_global_index(global_index),
    m_faithful_abstraction_index(faithful_abstraction_index),
    m_faithful_abstract_state_index(faithful_abstract_state_index)
{
}

bool GlobalFaithfulAbstractState::operator==(const GlobalFaithfulAbstractState& other) const
{
    if (this != &other)
    {
        return (m_global_index == other.m_global_index);
    }
    return true;
}

size_t GlobalFaithfulAbstractState::hash() const { return loki::hash_combine(m_global_index); }

StateIndex GlobalFaithfulAbstractState::get_index() const { return m_index; }

StateIndex GlobalFaithfulAbstractState::get_global_index() const { return m_global_index; }

AbstractionIndex GlobalFaithfulAbstractState::get_faithful_abstraction_index() const { return m_faithful_abstraction_index; }

StateIndex GlobalFaithfulAbstractState::get_faithful_abstract_state_index() const { return m_faithful_abstract_state_index; }

/**
 * GlobalFaithfulAbstraction
 */

GlobalFaithfulAbstraction::GlobalFaithfulAbstraction(bool mark_true_goal_literals,
                                                     bool use_unit_cost_one,
                                                     AbstractionIndex id,
                                                     std::shared_ptr<const FaithfulAbstractionList> abstractions,
                                                     GlobalFaithfulAbstractStateList states,
                                                     size_t num_isomorphic_states,
                                                     size_t num_non_isomorphic_states) :
    m_mark_true_goal_literals(mark_true_goal_literals),
    m_use_unit_cost_one(use_unit_cost_one),
    m_index(id),
    m_abstractions(std::move(abstractions)),
    m_states(std::move(states)),
    m_num_isomorphic_states(num_isomorphic_states),
    m_num_non_isomorphic_states(num_non_isomorphic_states)
{
    /* Ensure correctness. */

    // Check correct state ordering
    for (size_t i = 0; i < get_num_states(); ++i)
    {
        assert(get_states().at(i).get_index() == static_cast<StateIndex>(i) && "State index does not match its position in the list");
    }
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
    auto abstraction_index = AbstractionIndex { 0 };

    for (auto& faithful_abstraction : faithful_abstractions)
    {
        auto has_zero_non_isomorphic_states =
            certificate_to_global_state.count(faithful_abstraction.get_states().at(faithful_abstraction.get_initial_state()).get_certificate());

        if (has_zero_non_isomorphic_states)
        {
            // Skip: no non-isomorphic state
            continue;
        }

        auto num_isomorphic_states = 0;
        auto num_non_isomorphic_states = 0;
        auto states = GlobalFaithfulAbstractStateList {};
        for (const auto& state : faithful_abstraction.get_states())
        {
            // Ensure ordering consistent with state in faithful abstraction.
            assert(state.get_index() == states.size());

            auto it = certificate_to_global_state.find(state.get_certificate());

            if (it != certificate_to_global_state.end())
            {
                // Copy existing global state data.
                states.emplace_back(state.get_index(),
                                    it->second.get_global_index(),
                                    it->second.get_faithful_abstraction_index(),
                                    it->second.get_faithful_abstract_state_index());
                ++num_isomorphic_states;
            }
            else
            {
                // Create a new global state and add it to global mapping.
                certificate_to_global_state.emplace(
                    state.get_certificate(),
                    states.emplace_back(state.get_index(), certificate_to_global_state.size(), abstraction_index, state.get_index()));
                ++num_non_isomorphic_states;
            }
        }

        relevant_faithful_abstractions->push_back(std::move(faithful_abstraction));

        abstractions.push_back(GlobalFaithfulAbstraction(mark_true_goal_literals,
                                                         use_unit_cost_one,
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

StateIndex GlobalFaithfulAbstraction::get_abstract_state_index(State concrete_state) const
{
    return m_abstractions->at(m_index).get_abstract_state_index(concrete_state);
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

const StateMap<StateIndex>& GlobalFaithfulAbstraction::get_concrete_to_abstract_state() const
{
    return m_abstractions->at(m_index).get_concrete_to_abstract_state();
}

StateIndex GlobalFaithfulAbstraction::get_initial_state() const { return m_abstractions->at(m_index).get_initial_state(); }

const StateIndexSet& GlobalFaithfulAbstraction::get_goal_states() const { return m_abstractions->at(m_index).get_goal_states(); }

const StateIndexSet& GlobalFaithfulAbstraction::get_deadend_states() const { return m_abstractions->at(m_index).get_deadend_states(); }

TargetStateIndexIterator<AbstractTransition> GlobalFaithfulAbstraction::get_target_states(StateIndex source) const
{
    return m_abstractions->at(m_index).get_target_states(source);
}

SourceStateIndexIterator<AbstractTransition> GlobalFaithfulAbstraction::get_source_states(StateIndex target) const
{
    return m_abstractions->at(m_index).get_source_states(target);
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
const AbstractTransitionList& GlobalFaithfulAbstraction::get_transitions() const { return m_abstractions->at(m_index).get_transitions(); }

const BeginIndexList& GlobalFaithfulAbstraction::get_transitions_begin_by_source() const
{
    return m_abstractions->at(m_index).get_transitions_begin_by_source();
}

TransitionCost GlobalFaithfulAbstraction::get_transition_cost(TransitionIndex transition) const
{
    return (m_use_unit_cost_one) ? 1 : m_abstractions->at(m_index).get_transition_cost(transition);
}

ForwardTransitionIndexIterator<AbstractTransition> GlobalFaithfulAbstraction::get_forward_transition_indices(StateIndex source) const
{
    return m_abstractions->at(m_index).get_forward_transition_indices(source);
}

BackwardTransitionIndexIterator<AbstractTransition> GlobalFaithfulAbstraction::get_backward_transition_indices(StateIndex target) const
{
    return m_abstractions->at(m_index).get_backward_transition_indices(target);
}

ForwardTransitionIterator<AbstractTransition> GlobalFaithfulAbstraction::get_forward_transitions(StateIndex source) const
{
    return m_abstractions->at(m_index).get_forward_transitions(source);
}

BackwardTransitionIterator<AbstractTransition> GlobalFaithfulAbstraction::get_backward_transitions(StateIndex target) const
{
    return m_abstractions->at(m_index).get_backward_transitions(target);
}

size_t GlobalFaithfulAbstraction::get_num_transitions() const { return m_abstractions->at(m_index).get_num_transitions(); }

/* Distances */
const std::vector<double>& GlobalFaithfulAbstraction::get_goal_distances() const { return m_abstractions->at(m_index).get_goal_distances(); }

}
