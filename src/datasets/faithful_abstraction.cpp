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

#include "mimir/datasets/faithful_abstraction.hpp"

#include "mimir/algorithms/BS_thread_pool.hpp"
#include "mimir/common/timers.hpp"

#include <algorithm>
#include <cstdlib>
#include <deque>

namespace mimir
{

/**
 * FaithfulAbstractState
 */

FaithfulAbstractState::FaithfulAbstractState(StateId id, State state, std::string certificate) : m_id(id), m_state(state), m_certificate(std::move(certificate))
{
}

StateId FaithfulAbstractState::get_id() const { return m_id; }

State FaithfulAbstractState::get_state() const { return m_state; }

const std::string& FaithfulAbstractState::get_certificate() const { return m_certificate; }

/**
 * FaithfulAbstraction
 */

FaithfulAbstraction::FaithfulAbstraction(fs::path domain_filepath,
                                         fs::path problem_filepath,
                                         std::shared_ptr<PDDLParser> parser,
                                         std::shared_ptr<LiftedAAG> aag,
                                         std::shared_ptr<SSG> ssg,
                                         FaithfulAbstractStateList states,
                                         StateId initial_state,
                                         StateIdSet goal_states,
                                         StateIdSet deadend_states,
                                         size_t num_transitions,
                                         std::vector<TransitionList> forward_transitions,
                                         std::vector<TransitionList> backward_transitions,
                                         std::vector<int> goal_distances) :
    m_domain_filepath(std::move(domain_filepath)),
    m_problem_filepath(std::move(problem_filepath)),
    m_parser(std::move(parser)),
    m_aag(std::move(aag)),
    m_ssg(std::move(ssg)),
    m_states(std::move(states)),
    m_initial_state(initial_state),
    m_goal_states(std::move(goal_states)),
    m_deadend_states(std::move(deadend_states)),
    m_num_transitions(num_transitions),
    m_forward_transitions(std::move(forward_transitions)),
    m_backward_transitions(std::move(backward_transitions)),
    m_goal_distances(std::move(goal_distances))
{
}

std::optional<FaithfulAbstraction>
FaithfulAbstraction::create(const fs::path& domain_filepath, const fs::path& problem_filepath, const size_t max_num_states, const size_t timeout_ms)
{
    auto stop_watch = StopWatch(timeout_ms);

    auto pddl_parser = std::make_shared<PDDLParser>(domain_filepath, problem_filepath);
    const auto problem = pddl_parser->get_problem();
    const auto& factories = pddl_parser->get_factories();
    auto aag = std::make_shared<LiftedAAG>(problem, pddl_parser->get_factories());
    auto ssg = std::make_shared<SuccessorStateGenerator>(aag);
    auto initial_state = ssg->get_or_create_initial_state();

    if (!problem->static_goal_holds())
    {
        // Unsolvable
        return std::nullopt;
    }

    auto nauty_graph = nauty_wrapper::Graph();
    auto object_graph_factory = ObjectGraphFactory(problem, factories);
    auto concrete_to_abstract_state = std::unordered_map<State, StateId, StateHash> {};
    auto certificate_and_coloring = std::stringstream();

    auto abstract_states = FaithfulAbstractStateList {};
    auto abstract_states_by_certificate = std::unordered_map<std::string, StateId> {};

    auto abstract_initial_state_id = abstract_states.size();
    const auto& object_graph = object_graph_factory.create(initial_state);
    object_graph.get_digraph().to_nauty_graph(nauty_graph);
    certificate_and_coloring.str("");  // clears the stringstream
    certificate_and_coloring << nauty_graph.compute_certificate(object_graph.get_lab(), object_graph.get_ptn());
    certificate_and_coloring << object_graph.get_sorted_vertex_colors();
    abstract_states_by_certificate.emplace(certificate_and_coloring.str(), abstract_initial_state_id);
    concrete_to_abstract_state.emplace(initial_state, abstract_initial_state_id);
    auto abstract_initial_state = FaithfulAbstractState(abstract_initial_state_id, initial_state, certificate_and_coloring.str());
    abstract_states.push_back(abstract_initial_state);

    auto lifo_queue = std::deque<FaithfulAbstractState>();
    lifo_queue.push_back(abstract_initial_state);

    auto num_transitions = (size_t) 0;
    auto forward_successors = std::vector<TransitionList>(1);
    auto backward_successors = std::vector<TransitionList>(1);
    auto abstract_goal_states = StateIdSet {};

    auto applicable_actions = GroundActionList {};
    stop_watch.start();
    while (!lifo_queue.empty() && !stop_watch.has_finished())
    {
        const auto abstract_state = lifo_queue.back();
        const auto abstract_state_id = abstract_state.get_id();
        const auto state = abstract_state.get_state();

        lifo_queue.pop_back();

        if (state.literals_hold(problem->get_fluent_goal_condition()) && state.literals_hold(problem->get_derived_goal_condition()))
        {
            abstract_goal_states.insert(abstract_state_id);
        }

        aag->generate_applicable_actions(state, applicable_actions);

        for (const auto& action : applicable_actions)
        {
            const auto num_states = ssg->get_state_count();
            const auto successor_state = ssg->get_or_create_successor_state(state, action);

            // Regenerate concrete state
            const auto concrete_state_exists = successor_state.get_id() < num_states;
            if (concrete_state_exists)
            {
                const auto& abstract_successor_state = abstract_states.at(concrete_to_abstract_state.at(successor_state));
                const auto abstract_successor_state_id = abstract_successor_state.get_id();
                forward_successors.at(abstract_state_id).emplace_back(abstract_successor_state_id, action);
                backward_successors.at(abstract_successor_state_id).emplace_back(abstract_state_id, action);
                ++num_transitions;
                continue;
            }

            // Compute certificate of successor state
            const auto& object_graph = object_graph_factory.create(successor_state);
            object_graph.get_digraph().to_nauty_graph(nauty_graph);
            certificate_and_coloring.str("");  // clears the stringstream
            certificate_and_coloring << nauty_graph.compute_certificate(object_graph.get_lab(), object_graph.get_ptn());
            certificate_and_coloring << object_graph.get_sorted_vertex_colors();
            const auto it = abstract_states_by_certificate.find(certificate_and_coloring.str());

            // Regenerate abstract state
            const auto abstract_state_exists = (it != abstract_states_by_certificate.end());
            if (abstract_state_exists)
            {
                const auto& abstract_successor_state = abstract_states.at(it->second);
                const auto abstract_successor_state_id = abstract_successor_state.get_id();
                concrete_to_abstract_state.emplace(successor_state, abstract_successor_state_id);
                forward_successors.at(abstract_state_id).emplace_back(abstract_successor_state_id, action);
                backward_successors.at(abstract_successor_state_id).emplace_back(abstract_state_id, action);
                ++num_transitions;
                continue;
            }

            const auto abstract_successor_state_id = abstract_states.size();
            const auto abstract_successor_state = FaithfulAbstractState(abstract_successor_state_id, successor_state, certificate_and_coloring.str());
            abstract_states.push_back(abstract_successor_state);
            if (abstract_states.size() >= max_num_states)
            {
                // Ran out of state resources
                return std::nullopt;
            }
            abstract_states_by_certificate.emplace(certificate_and_coloring.str(), abstract_successor_state_id);
            concrete_to_abstract_state.emplace(successor_state, abstract_successor_state_id);

            forward_successors.resize(abstract_states.size());
            backward_successors.resize(abstract_states.size());
            forward_successors.at(abstract_state.get_id()).emplace_back(abstract_successor_state_id, action);
            backward_successors.at(abstract_successor_state_id).emplace_back(abstract_state.get_id(), action);
            ++num_transitions;

            lifo_queue.push_back(abstract_successor_state);
            // Abstract states are stored by index
            assert(abstract_states.back().get_id() == static_cast<int>(abstract_states.size()) - 1);
        }
    }

    if (stop_watch.has_finished())
    {
        // Ran out of time
        return std::nullopt;
    }

    auto abstract_goal_distances = mimir::compute_shortest_distances_from_states(abstract_states.size(),
                                                                                 StateIdList { abstract_goal_states.begin(), abstract_goal_states.end() },
                                                                                 backward_successors);

    auto abstract_deadend_states = StateIdSet {};
    for (const auto& state : abstract_states)
    {
        if (abstract_goal_distances.at(state.get_id()) == -1)
        {
            abstract_deadend_states.insert(state.get_id());
        }
    }

    return FaithfulAbstraction(domain_filepath,
                               problem_filepath,
                               std::move(pddl_parser),
                               std::move(aag),
                               std::move(ssg),
                               std::move(abstract_states),
                               abstract_initial_state_id,
                               std::move(abstract_goal_states),
                               std::move(abstract_deadend_states),
                               num_transitions,
                               std::move(forward_successors),
                               std::move(backward_successors),
                               std::move(abstract_goal_distances));
}

std::vector<FaithfulAbstraction> FaithfulAbstraction::create(const fs::path& domain_filepath,
                                                             const std::vector<fs::path>& problem_filepaths,
                                                             const size_t max_num_states,
                                                             const size_t timeout_ms,
                                                             const size_t num_threads)
{
    auto abstractions = std::vector<FaithfulAbstraction> {};
    auto pool = BS::thread_pool(num_threads);
    auto futures = std::vector<std::future<std::optional<FaithfulAbstraction>>> {};

    for (const auto& problem_filepath : problem_filepaths)
    {
        futures.push_back(pool.submit_task([domain_filepath, problem_filepath, max_num_states, timeout_ms]
                                           { return FaithfulAbstraction::create(domain_filepath, problem_filepath, max_num_states, timeout_ms); }));
    }

    for (auto& future : futures)
    {
        auto abstraction = future.get();
        if (abstraction.has_value())
        {
            abstractions.push_back(std::move(abstraction.value()));
        }
    }

    return abstractions;
}

std::vector<int> FaithfulAbstraction::compute_shortest_distances_from_states(const StateIdList& abstract_states, bool forward) const
{
    return mimir::compute_shortest_distances_from_states(*this, abstract_states, forward);
}

std::vector<std::vector<int>> FaithfulAbstraction::compute_pairwise_shortest_state_distances(bool forward) const
{
    return mimir::compute_pairwise_shortest_state_distances(*this, forward);
}

/**
 * Getters
 */

// Meta data
const fs::path& FaithfulAbstraction::get_domain_filepath() const { return m_domain_filepath; }

const fs::path& FaithfulAbstraction::get_problem_filepath() const { return m_problem_filepath; }

// Memory
const std::shared_ptr<PDDLParser>& FaithfulAbstraction::get_pddl_parser() const { return m_parser; }

const std::shared_ptr<PDDLFactories>& FaithfulAbstraction::get_pddl_factories() const { return m_parser->get_factories(); }

const std::shared_ptr<LiftedAAG>& FaithfulAbstraction::get_aag() const { return m_aag; }

const std::shared_ptr<SuccessorStateGenerator>& FaithfulAbstraction::get_ssg() const { return m_ssg; }

// States
const FaithfulAbstractStateList& FaithfulAbstraction::get_states() const { return m_states; }

const std::vector<TransitionList>& FaithfulAbstraction::get_forward_transitions() const { return m_forward_transitions; }

const std::vector<TransitionList>& FaithfulAbstraction::get_backward_transitions() const { return m_backward_transitions; }

StateId FaithfulAbstraction::get_initial_state() const { return m_initial_state; }

const StateIdSet& FaithfulAbstraction::get_goal_states() const { return m_goal_states; }

const StateIdSet& FaithfulAbstraction::get_deadend_states() const { return m_deadend_states; }

size_t FaithfulAbstraction::get_num_states() const { return m_states.size(); }

// Transitions
size_t FaithfulAbstraction::get_num_transitions() const { return m_num_transitions; }

size_t FaithfulAbstraction::get_num_goal_states() const { return m_goal_states.size(); }

size_t FaithfulAbstraction::get_num_deadend_states() const { return m_deadend_states.size(); }

// Distances
const std::vector<int>& FaithfulAbstraction::get_goal_distances() const { return m_goal_distances; }

}
