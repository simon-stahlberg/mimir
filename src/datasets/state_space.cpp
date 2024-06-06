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

#include "mimir/datasets/state_space.hpp"

#include "mimir/common/timers.hpp"

#include <algorithm>
#include <cstdlib>
#include <deque>

namespace mimir
{

static std::vector<int>
compute_shortest_distances_from_states_impl(const size_t num_total_states, const StateList& states, const std::vector<Transitions>& transitions)
{
    auto distances = std::vector<int>(num_total_states, -1);
    auto fifo_queue = std::deque<int>();
    for (const auto& state : states)
    {
        distances[state.get_id()] = 0;
        fifo_queue.push_back(state.get_id());
    }

    while (!fifo_queue.empty())
    {
        const auto& state_id = fifo_queue.front();
        fifo_queue.pop_front();
        const auto cost = distances[state_id];

        for (const auto& transition : transitions[state_id])
        {
            const auto successor_state_id = transition.get_successor_state().get_id();

            if (distances[successor_state_id] != -1)
            {
                continue;
            }

            distances[successor_state_id] = cost + 1;

            fifo_queue.push_back(successor_state_id);
        }
    }
    return distances;
}

Transition::Transition(State successor_state, GroundAction creating_action) : m_successor_state(successor_state), m_creating_action(creating_action) {}

bool Transition::operator==(const Transition& other) const
{
    if (this != &other)
    {
        return (m_successor_state.get_id() == other.m_successor_state.get_id()) && (m_creating_action.get_id() == other.m_creating_action.get_id());
    }
    return true;
}

size_t Transition::hash() const { return loki::hash_combine(m_successor_state.get_id(), m_creating_action.get_id()); }

State Transition::get_successor_state() const { return m_successor_state; }

GroundAction Transition::get_creating_action() const { return m_creating_action; }

StateSpaceImpl::StateSpaceImpl(PDDLParser parser,
                               std::shared_ptr<GroundedAAG> aag,
                               std::shared_ptr<SuccessorStateGenerator> ssg,
                               StateList states,
                               State initial_state,
                               size_t num_transitions,
                               std::vector<Transitions> forward_transitions,
                               std::vector<Transitions> backward_transitions,
                               std::vector<int> goal_distances,
                               StateSet goal_states,
                               StateSet deadend_states) :
    m_parser(std::move(parser)),
    m_aag(std::move(aag)),
    m_ssg(std::move(ssg)),
    m_states(std::move(states)),
    m_initial_state(std::move(initial_state)),
    m_num_transitions(num_transitions),
    m_forward_transitions(std::move(forward_transitions)),
    m_backward_transitions(std::move(backward_transitions)),
    m_goal_distances(std::move(goal_distances)),
    m_goal_states(std::move(goal_states)),
    m_deadend_states(std::move(deadend_states)),
    m_state_indices(),
    m_states_by_goal_distance()
{
    for (size_t index = 0; index < m_states.size(); ++index)
    {
        auto state = m_states.at(index);
        m_state_indices.emplace(state, index);

        auto distance = m_goal_distances.at(index);
        auto& states_with_distance = m_states_by_goal_distance[distance];
        states_with_distance.emplace_back(state);
    }
}

std::shared_ptr<StateSpaceImpl>
StateSpaceImpl::create(const fs::path& domain_file_path, const fs::path& problem_file_path, const size_t max_num_states, const size_t timeout_ms)
{
    auto stop_watch = StopWatch(timeout_ms);

    auto pddl_parser = PDDLParser(domain_file_path, problem_file_path);
    const auto problem = pddl_parser.get_problem();
    auto aag = std::make_shared<GroundedAAG>(problem, pddl_parser.get_factories());
    auto ssg = std::make_shared<SuccessorStateGenerator>(aag);
    auto initial_state = ssg->get_or_create_initial_state();

    if (!problem->static_goal_holds())
    {
        // Unsolvable
        return nullptr;
    }

    auto lifo_queue = std::deque<State>();
    lifo_queue.push_back(initial_state);

    auto states = StateList { initial_state };
    auto num_transitions = (size_t) 0;
    auto forward_transitions = std::vector<Transitions>(1);
    auto backward_transitions = std::vector<Transitions>(1);
    auto goal_states = StateSet {};

    auto applicable_actions = GroundActionList {};
    stop_watch.start();
    while (!lifo_queue.empty() && !stop_watch.has_finished())
    {
        const auto state = lifo_queue.back();

        lifo_queue.pop_back();

        if (state.literals_hold(problem->get_fluent_goal_condition()) && state.literals_hold(problem->get_derived_goal_condition()))
        {
            goal_states.insert(state);
        }

        aag->generate_applicable_actions(state, applicable_actions);

        for (const auto& action : applicable_actions)
        {
            const auto successor_state = ssg->get_or_create_successor_state(state, action);

            forward_transitions.resize(ssg->get_state_count());
            backward_transitions.resize(ssg->get_state_count());

            forward_transitions[state.get_id()].emplace_back(successor_state, action);
            backward_transitions[successor_state.get_id()].emplace_back(state, action);
            ++num_transitions;

            if (successor_state.get_id() < states.size())
            {
                continue;
            }

            if (states.size() == max_num_states)
            {
                // Ran out of state resources
                return nullptr;
            }

            states.push_back(successor_state);
            lifo_queue.push_back(successor_state);
            // States are stored by index
            assert(states.back().get_id() == states.size() - 1);
        }
    }

    if (stop_watch.has_finished())
    {
        // Ran out of time
        return nullptr;
    }

    auto goal_distances =
        compute_shortest_distances_from_states_impl(states.size(), StateList { goal_states.begin(), goal_states.end() }, backward_transitions);

    auto deadend_states = StateSet {};
    for (const auto& state : states)
    {
        if (goal_distances[state.get_id()] == -1)
        {
            deadend_states.insert(state);
        }
    }

    // Must explicitly call constructor since it is private
    return std::shared_ptr<StateSpaceImpl>(new StateSpaceImpl(std::move(pddl_parser),
                                                              std::move(aag),
                                                              std::move(ssg),
                                                              std::move(states),
                                                              initial_state,
                                                              num_transitions,
                                                              std::move(forward_transitions),
                                                              std::move(backward_transitions),
                                                              std::move(goal_distances),
                                                              std::move(goal_states),
                                                              std::move(deadend_states)));
}

/* Extended functionality */
std::vector<int> StateSpaceImpl::compute_shortest_distances_from_states(const StateList& states, bool forward) const
{
    return compute_shortest_distances_from_states_impl(m_states.size(), states, (forward) ? m_forward_transitions : m_backward_transitions);
}

std::vector<std::vector<int>> StateSpaceImpl::compute_pairwise_shortest_state_distances(bool forward) const
{
    auto distances = std::vector<std::vector<int>> { m_states.size(), std::vector<int>(m_states.size(), -1) };
    const auto& transitions = (forward) ? m_forward_transitions : m_backward_transitions;

    // Initialize distance adjacency matrix
    for (size_t state_id = 0; state_id < m_states.size(); ++state_id)
    {
        distances[state_id][state_id] = 0;
        for (const auto& transition : transitions[state_id])
        {
            distances[state_id][transition.get_successor_state().get_id()] = 1;
        }
    }

    // Compute transitive closure
    for (size_t state_k = 0; state_k < m_states.size(); ++state_k)
    {
        for (size_t state_i = 0; state_i < m_states.size(); ++state_i)
        {
            for (size_t state_j = 0; state_j < m_states.size(); ++state_j)
            {
                if (distances[state_i][state_j] > distances[state_i][state_k] + distances[state_k][state_j])
                {
                    distances[state_i][state_j] = distances[state_i][state_k] + distances[state_k][state_j];
                }
            }
        }
    }

    return distances;
}

/* Getters */
Problem StateSpaceImpl::get_problem() const { return m_aag->get_problem(); }

const StateList& StateSpaceImpl::get_states() const { return m_states; }

State StateSpaceImpl::get_initial_state() const { return m_initial_state; }

const std::vector<Transitions>& StateSpaceImpl::get_forward_transitions() const { return m_forward_transitions; }

const std::vector<Transitions>& StateSpaceImpl::get_backward_transitions() const { return m_backward_transitions; }

const std::vector<int>& StateSpaceImpl::get_goal_distances() const { return m_goal_distances; }

int StateSpaceImpl::get_goal_distance(const State& state) const { return m_goal_distances.at(m_state_indices.at(state)); }

int StateSpaceImpl::get_max_goal_distance() const { return *std::max_element(m_goal_distances.begin(), m_goal_distances.end()); }

const StateSet& StateSpaceImpl::get_goal_states() const { return m_goal_states; }

const StateSet& StateSpaceImpl::get_deadend_states() const { return m_deadend_states; }

size_t StateSpaceImpl::get_num_states() const { return m_states.size(); }

size_t StateSpaceImpl::get_num_transitions() const { return m_num_transitions; }

size_t StateSpaceImpl::get_num_goal_states() const { return m_goal_states.size(); }

size_t StateSpaceImpl::get_num_deadend_states() const { return m_deadend_states.size(); }

bool StateSpaceImpl::is_deadend_state(const State& state) const { return m_goal_distances.at(m_state_indices.at(state)) < 0; }

State StateSpaceImpl::sample_state_with_goal_distance(int goal_distance) const
{
    const auto& states = m_states_by_goal_distance.at(goal_distance);
    const auto index = std::rand() % static_cast<int>(states.size());
    return states[index];
}

std::shared_ptr<GroundedAAG> StateSpaceImpl::get_aag() const { return m_aag; }

std::shared_ptr<SuccessorStateGenerator> StateSpaceImpl::get_ssg() const { return m_ssg; }

const PDDLParser& StateSpaceImpl::get_pddl_parser() const { return m_parser; }

PDDLFactories& StateSpaceImpl::get_factories() { return m_parser.get_factories(); }

}
