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

#include <deque>

namespace mimir
{

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
                               std::vector<double> goal_distances,
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
    m_deadend_states(std::move(deadend_states))
{
}

std::shared_ptr<const StateSpaceImpl>
StateSpaceImpl::create(const fs::path& domain_file_path, const fs::path& problem_file_path, const size_t max_num_states, const size_t timeout_ms)
{
    auto stop_watch = StopWatch(timeout_ms);

    auto pddl_parser = PDDLParser(domain_file_path, problem_file_path);
    auto aag = std::make_shared<GroundedAAG>(pddl_parser.get_problem(), pddl_parser.get_factories());
    auto ssg = std::make_shared<SuccessorStateGenerator>(aag);

    auto initial_state = ssg->get_or_create_initial_state();
    auto queue = std::deque<State>();
    queue.push_back(initial_state);

    auto num_states = ssg->get_state_count();
    auto num_transitions = (size_t) 0;
    auto forward_transitions = std::vector<Transitions> {};
    auto backward_transitions = std::vector<Transitions> {};

    auto applicable_actions = GroundActionList {};
    stop_watch.start();
    while (!queue.empty())
    {
        while (!stop_watch.has_finished())
        {
            const auto state = queue.back();
            queue.pop_back();

            aag->generate_applicable_actions(state, applicable_actions);

            for (const auto& action : applicable_actions)
            {
                const auto successor_state = ssg->get_or_create_successor_state(state, action);

                if (successor_state.get_id() < num_states)
                {
                    continue;
                }

                ++num_states;

                if (num_states == max_num_states)
                {
                    // Ran out of state resources
                    return nullptr;
                }

                forward_transitions.resize(num_states);
                backward_transitions.resize(num_states);

                forward_transitions[state.get_id()].emplace_back(successor_state, action);
                backward_transitions[successor_state.get_id()].emplace_back(state, action);
                ++num_transitions;
            }
        }

        if (!queue.empty())
        {
            // Ran out of time
            return nullptr;
        }
    }
    return nullptr;
}

/* Extended functionality */
std::vector<double> StateSpaceImpl::compute_distances_from_state(const State state) const
{
    auto distances = std::vector<double> {};
    // TODO
    return distances;
}

std::vector<std::vector<double>> StateSpaceImpl::compute_pairwise_state_distances() const
{
    auto distances = std::vector<std::vector<double>> {};
    // TODO
    return distances;
}

/* Getters */
const StateList& StateSpaceImpl::get_states() const { return m_states; }

State StateSpaceImpl::get_initial_state() const { return m_initial_state; }

const std::vector<Transitions>& StateSpaceImpl::get_forward_transitions() const { return m_forward_transitions; }

const std::vector<Transitions>& StateSpaceImpl::get_backward_transitions() const { return m_backward_transitions; }

const std::vector<double>& StateSpaceImpl::get_goal_distances() const { return m_goal_distances; }

const StateSet& StateSpaceImpl::get_goal_states() const { return m_goal_states; }

const StateSet& StateSpaceImpl::get_deadend_states() const { return m_deadend_states; }

size_t StateSpaceImpl::get_num_states() const { return m_states.size(); }

size_t StateSpaceImpl::get_num_transitions() const { return m_num_transitions; }

size_t StateSpaceImpl::get_num_goal_states() const { return m_goal_states.size(); }

size_t StateSpaceImpl::get_num_deadend_states() const { return m_deadend_states.size(); }
}