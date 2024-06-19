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

static std::vector<int> compute_shortest_distances_from_states_impl(const size_t num_total_states,
                                                                    const AbstractStateIdList& states,
                                                                    const std::vector<AbstractStateIdList>& transitions)
{
    auto distances = std::vector<int>(num_total_states, -1);
    auto fifo_queue = std::deque<int>();
    for (const auto& state : states)
    {
        distances.at(state) = 0;
        fifo_queue.push_back(state);
    }

    while (!fifo_queue.empty())
    {
        const auto& state_id = fifo_queue.front();
        fifo_queue.pop_front();
        const auto cost = distances.at(state_id);

        for (const auto& successor_state_id : transitions.at(state_id))
        {
            if (distances.at(successor_state_id) != -1)
            {
                continue;
            }

            distances.at(successor_state_id) = cost + 1;

            fifo_queue.push_back(successor_state_id);
        }
    }
    return distances;
}

/**
 * AbstractState
 */

FaithfulAbstractionImpl::AbstractState::AbstractState(AbstractStateId id, State state) : m_id(id), m_state(state) {}

AbstractStateId FaithfulAbstractionImpl::AbstractState::get_id() const { return m_id; }

State FaithfulAbstractionImpl::AbstractState::get_state() const { return m_state; }

/**
 * FaithfulAbstraction
 */

FaithfulAbstractionImpl::FaithfulAbstractionImpl(std::unique_ptr<PDDLParser>&& parser,
                                                 std::shared_ptr<GroundedAAG> aag,
                                                 std::shared_ptr<SSG> ssg,
                                                 AbstractStateList abstract_states,
                                                 std::unordered_map<std::string, AbstractStateId> abstract_states_by_certificate,
                                                 AbstractStateId abstract_initial_state,
                                                 size_t num_abstract_transitions,
                                                 std::vector<AbstractStateIdList> forward_successors,
                                                 std::vector<AbstractStateIdList> backward_successors,
                                                 AbstractStateIdSet abstract_goal_states,
                                                 AbstractStateIdSet abstract_deadend_states,
                                                 std::vector<int> abstract_goal_distances) :
    m_parser(std::move(parser)),
    m_aag(std::move(aag)),
    m_ssg(std::move(ssg)),
    m_abstract_states(std::move(abstract_states)),
    m_abstract_states_by_certificate(std::move(abstract_states_by_certificate)),
    m_abstract_initial_state(abstract_initial_state),
    m_num_abstract_transitions(num_abstract_transitions),
    m_forward_successors(std::move(forward_successors)),
    m_backward_successors(std::move(backward_successors)),
    m_abstract_goal_states(std::move(abstract_goal_states)),
    m_abstract_deadend_states(std::move(abstract_deadend_states)),
    m_abstract_goal_distances(std::move(abstract_goal_distances))
{
}

FaithfulAbstraction
FaithfulAbstractionImpl::create(const fs::path& domain_file_path, const fs::path& problem_file_path, const size_t max_num_states, const size_t timeout_ms)
{
    auto stop_watch = StopWatch(timeout_ms);

    auto pddl_parser = std::make_unique<PDDLParser>(domain_file_path, problem_file_path);
    const auto problem = pddl_parser->get_problem();
    const auto& factories = pddl_parser->get_factories();
    auto aag = std::make_shared<GroundedAAG>(problem, pddl_parser->get_factories());
    auto ssg = std::make_shared<SuccessorStateGenerator>(aag);
    auto initial_state = ssg->get_or_create_initial_state();

    if (!problem->static_goal_holds())
    {
        // Unsolvable
        return nullptr;
    }

    auto nauty_graph = nauty_wrapper::Graph();
    auto object_graph_factory = ObjectGraphFactory(problem, factories);
    auto concrete_to_abstract_state = std::unordered_map<State, AbstractStateId, StateHash> {};
    auto certificate_and_coloring = std::stringstream();

    auto abstract_states = AbstractStateList {};
    auto abstract_states_by_certificate = std::unordered_map<std::string, AbstractStateId> {};

    auto abstract_initial_state_id = abstract_states.size();
    auto abstract_initial_state = AbstractState(abstract_initial_state_id, initial_state);
    abstract_states.push_back(abstract_initial_state);
    const auto& object_graph = object_graph_factory.create(abstract_initial_state.get_state());
    object_graph.get_digraph().to_nauty_graph(nauty_graph);
    certificate_and_coloring.str("");  // clears the stringstream
    certificate_and_coloring << nauty_graph.compute_certificate(object_graph.get_lab(), object_graph.get_ptn());
    certificate_and_coloring << object_graph.get_sorted_vertex_colors();
    abstract_states_by_certificate.emplace(certificate_and_coloring.str(), abstract_initial_state_id);
    concrete_to_abstract_state.emplace(initial_state, abstract_initial_state_id);

    auto lifo_queue = std::deque<AbstractState>();
    lifo_queue.push_back(abstract_initial_state);

    auto num_transitions = (size_t) 0;
    auto forward_successors = std::vector<AbstractStateIdList>(1);
    auto backward_successors = std::vector<AbstractStateIdList>(1);
    auto abstract_goal_states = AbstractStateIdSet {};

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
                forward_successors.at(abstract_state_id).push_back(abstract_successor_state_id);
                backward_successors.at(abstract_successor_state_id).push_back(abstract_state_id);
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
                forward_successors.at(abstract_state_id).push_back(abstract_successor_state_id);
                backward_successors.at(abstract_successor_state_id).push_back(abstract_state_id);
                ++num_transitions;
                continue;
            }

            const auto abstract_successor_state_id = abstract_states.size();
            const auto abstract_successor_state = AbstractState(abstract_successor_state_id, successor_state);
            abstract_states.push_back(abstract_successor_state);
            if (abstract_states.size() == max_num_states)
            {
                // Ran out of state resources
                return nullptr;
            }
            abstract_states_by_certificate.emplace(certificate_and_coloring.str(), abstract_successor_state_id);
            concrete_to_abstract_state.emplace(successor_state, abstract_successor_state_id);

            forward_successors.resize(abstract_states.size());
            backward_successors.resize(abstract_states.size());
            forward_successors.at(abstract_state.get_id()).push_back(abstract_successor_state_id);
            backward_successors.at(abstract_successor_state_id).push_back(abstract_state.get_id());
            ++num_transitions;

            lifo_queue.push_back(abstract_successor_state);
            // Abstract states are stored by index
            assert(abstract_states.back().get_id() == static_cast<int>(abstract_states.size()) - 1);
        }
    }

    if (stop_watch.has_finished())
    {
        // Ran out of time
        return nullptr;
    }

    auto abstract_goal_distances = compute_shortest_distances_from_states_impl(abstract_states.size(),
                                                                               AbstractStateIdList { abstract_goal_states.begin(), abstract_goal_states.end() },
                                                                               backward_successors);

    auto abstract_deadend_states = AbstractStateIdSet {};
    for (const auto& state : abstract_states)
    {
        if (abstract_goal_distances.at(state.get_id()) == -1)
        {
            abstract_deadend_states.insert(state.get_id());
        }
    }

    return FaithfulAbstraction(new FaithfulAbstractionImpl(std::move(pddl_parser),
                                                           std::move(aag),
                                                           std::move(ssg),
                                                           std::move(abstract_states),
                                                           std::move(abstract_states_by_certificate),
                                                           abstract_initial_state_id,
                                                           num_transitions,
                                                           std::move(forward_successors),
                                                           std::move(backward_successors),
                                                           std::move(abstract_goal_states),
                                                           std::move(abstract_deadend_states),
                                                           std::move(abstract_goal_distances)));
}

FaithfulAbstractionList FaithfulAbstractionImpl::create(const fs::path& domain_file_path,
                                                        const std::vector<fs::path>& problem_file_paths,
                                                        const size_t max_num_states,
                                                        const size_t timeout_ms,
                                                        const size_t num_threads)
{
    auto abstractions = FaithfulAbstractionList {};
    auto pool = BS::thread_pool(num_threads);
    auto futures = std::vector<std::future<FaithfulAbstraction>> {};

    for (const auto& problem_file_path : problem_file_paths)
    {
        futures.push_back(pool.submit_task([domain_file_path, problem_file_path, max_num_states, timeout_ms]
                                           { return FaithfulAbstractionImpl::create(domain_file_path, problem_file_path, max_num_states, timeout_ms); }));
    }

    for (auto& future : futures)
    {
        const auto abstraction = future.get();
        if (abstraction)
        {
            abstractions.push_back(abstraction);
        }
    }

    return abstractions;
}

const FaithfulAbstractionImpl::AbstractStateList& FaithfulAbstractionImpl::get_abstract_states() const { return m_abstract_states; }

const std::unordered_map<std::string, AbstractStateId>& FaithfulAbstractionImpl::get_abstract_states_by_certificate() const
{
    return m_abstract_states_by_certificate;
}

const std::vector<AbstractStateIdList>& FaithfulAbstractionImpl::get_forward_successors() const { return m_forward_successors; }

const std::vector<AbstractStateIdList>& FaithfulAbstractionImpl::get_backward_successors() const { return m_backward_successors; }

AbstractStateId FaithfulAbstractionImpl::get_abstract_initial_state() const { return m_abstract_initial_state; }

const AbstractStateIdSet& FaithfulAbstractionImpl::get_abstract_goal_states() const { return m_abstract_goal_states; }

const AbstractStateIdSet& FaithfulAbstractionImpl::get_abstract_deadend_states() const { return m_abstract_deadend_states; }

const std::vector<int>& FaithfulAbstractionImpl::get_abstract_goal_distances() const { return m_abstract_goal_distances; }

size_t FaithfulAbstractionImpl::get_num_abstract_states() const { return m_abstract_states.size(); }

size_t FaithfulAbstractionImpl::get_num_abstract_transitions() const { return m_num_abstract_transitions; }

size_t FaithfulAbstractionImpl::get_num_abstract_goal_states() const { return m_abstract_goal_states.size(); }

size_t FaithfulAbstractionImpl::get_num_abstract_deadend_states() const { return m_abstract_deadend_states.size(); }

/**
 * CombinedAbstractState
 */

CombinedFaithfulAbstraction::CombinedAbstractState::CombinedAbstractState(AbstractionId abstraction_id, AbstractStateId abstract_state_id) :
    m_abstraction_id(abstraction_id),
    m_abstract_state_id(abstract_state_id)
{
}

AbstractionId CombinedFaithfulAbstraction::CombinedAbstractState::get_abstraction_id() const { return m_abstraction_id; }

AbstractStateId CombinedFaithfulAbstraction::CombinedAbstractState::get_abstract_state_id() const { return m_abstract_state_id; }

/**
 * CombinedFaithfulAbstraction
 */

CombinedFaithfulAbstraction::CombinedFaithfulAbstraction(FaithfulAbstractionList abstractions)
{
    // TODO
}

const FaithfulAbstractionList& CombinedFaithfulAbstraction::get_abstractions() const { return m_abstractions; }
const std::vector<CombinedFaithfulAbstraction::CombinedAbstractStateList>& CombinedFaithfulAbstraction::get_states_by_abstraction() const
{
    return m_states_by_abstraction;
}
const CombinedFaithfulAbstraction::CombinedAbstractStateList& CombinedFaithfulAbstraction::get_states(AbstractionId abstraction_id) const
{
    return m_states_by_abstraction.at(abstraction_id);
}

}
