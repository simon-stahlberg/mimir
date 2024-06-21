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

#include "mimir/algorithms/BS_thread_pool.hpp"
#include "mimir/common/timers.hpp"

#include <algorithm>
#include <cstdlib>
#include <deque>

namespace mimir
{

StateSpace::StateSpace(fs::path domain_filepath,
                       fs::path problem_filepath,
                       std::shared_ptr<PDDLParser> parser,
                       std::shared_ptr<GroundedAAG> aag,
                       std::shared_ptr<SuccessorStateGenerator> ssg,
                       StateList states,
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
    m_initial_state(std::move(initial_state)),
    m_goal_states(std::move(goal_states)),
    m_deadend_states(std::move(deadend_states)),
    m_num_transitions(num_transitions),
    m_forward_transitions(std::move(forward_transitions)),
    m_backward_transitions(std::move(backward_transitions)),
    m_goal_distances(std::move(goal_distances)),
    m_states_by_goal_distance()
{
    for (size_t state_id = 0; state_id < m_states.size(); ++state_id)
    {
        auto distance = m_goal_distances.at(state_id);
        m_states_by_goal_distance[distance].push_back(state_id);
    }
}

std::optional<StateSpace>
StateSpace::create(const fs::path& domain_filepath, const fs::path& problem_filepath, const size_t max_num_states, const size_t timeout_ms)
{
    auto stop_watch = StopWatch(timeout_ms);

    auto pddl_parser = std::make_shared<PDDLParser>(domain_filepath, problem_filepath);
    const auto problem = pddl_parser->get_problem();
    auto aag = std::make_shared<GroundedAAG>(problem, pddl_parser->get_factories());
    auto ssg = std::make_shared<SuccessorStateGenerator>(aag);
    auto initial_state = ssg->get_or_create_initial_state();

    if (!problem->static_goal_holds())
    {
        // Unsolvable
        return std::nullopt;
    }

    auto lifo_queue = std::deque<State>();
    lifo_queue.push_back(initial_state);

    auto states = StateList { initial_state };
    auto num_transitions = (size_t) 0;
    auto forward_transitions = std::vector<TransitionList>(1);
    auto backward_transitions = std::vector<TransitionList>(1);
    auto goal_states = StateIdSet {};

    auto applicable_actions = GroundActionList {};
    stop_watch.start();
    while (!lifo_queue.empty() && !stop_watch.has_finished())
    {
        const auto state = lifo_queue.back();

        lifo_queue.pop_back();

        if (state.literals_hold(problem->get_fluent_goal_condition()) && state.literals_hold(problem->get_derived_goal_condition()))
        {
            goal_states.insert(state.get_id());
        }

        aag->generate_applicable_actions(state, applicable_actions);

        for (const auto& action : applicable_actions)
        {
            const auto successor_state = ssg->get_or_create_successor_state(state, action);

            forward_transitions.resize(ssg->get_state_count());
            backward_transitions.resize(ssg->get_state_count());

            forward_transitions.at(state.get_id()).emplace_back(successor_state.get_id(), action);
            backward_transitions.at(successor_state.get_id()).emplace_back(state.get_id(), action);
            ++num_transitions;

            if (successor_state.get_id() < states.size())
            {
                continue;
            }

            if (states.size() >= max_num_states)
            {
                // Ran out of state resources
                return std::nullopt;
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
        return std::nullopt;
    }

    auto goal_distances =
        mimir::compute_shortest_distances_from_states(states.size(), StateIdList { goal_states.begin(), goal_states.end() }, backward_transitions);

    auto deadend_states = StateIdSet {};
    for (const auto& state : states)
    {
        if (goal_distances.at(state.get_id()) == -1)
        {
            deadend_states.insert(state.get_id());
        }
    }

    // Must explicitly call constructor since it is private
    return StateSpace(domain_filepath,
                      problem_filepath,
                      std::move(pddl_parser),
                      std::move(aag),
                      std::move(ssg),
                      std::move(states),
                      initial_state.get_id(),
                      std::move(goal_states),
                      std::move(deadend_states),
                      num_transitions,
                      std::move(forward_transitions),
                      std::move(backward_transitions),
                      std::move(goal_distances));
}

StateSpaceList StateSpace::create(const fs::path& domain_filepath,
                                  const std::vector<fs::path>& problem_file_paths,
                                  const size_t max_num_states,
                                  const size_t timeout_ms,
                                  const size_t num_threads)
{
    auto state_spaces = StateSpaceList {};
    auto pool = BS::thread_pool(num_threads);
    auto futures = std::vector<std::future<std::optional<StateSpace>>> {};

    for (const auto& problem_filepath : problem_file_paths)
    {
        futures.push_back(pool.submit_task([domain_filepath, problem_filepath, max_num_states, timeout_ms]
                                           { return StateSpace::create(domain_filepath, problem_filepath, max_num_states, timeout_ms); }));
    }

    for (auto& future : futures)
    {
        auto state_space = future.get();
        if (state_space.has_value())
        {
            state_spaces.push_back(std::move(state_space.value()));
        }
    }

    return state_spaces;
}

/* Extended functionality */
std::vector<int> StateSpace::compute_shortest_distances_from_states(const StateIdList& states, bool forward) const
{
    return mimir::compute_shortest_distances_from_states(*this, states, forward);
}

std::vector<std::vector<int>> StateSpace::compute_pairwise_shortest_state_distances(bool forward) const
{
    return mimir::compute_pairwise_shortest_state_distances(*this, forward);
}

/* Getters */
// Meta data
const fs::path& StateSpace::get_domain_filepath() const { return m_domain_filepath; }

const fs::path& StateSpace::get_problem_filepath() const { return m_problem_filepath; }

// Memory
const PDDLParser& StateSpace::get_pddl_parser() const { return *m_parser; }

const std::shared_ptr<PDDLFactories>& StateSpace::get_pddl_factories() const { return m_parser->get_factories(); }

const std::shared_ptr<GroundedAAG>& StateSpace::get_aag() const { return m_aag; }

const std::shared_ptr<SuccessorStateGenerator>& StateSpace::get_ssg() const { return m_ssg; }

Problem StateSpace::get_problem() const { return m_aag->get_problem(); }

// States
const StateList& StateSpace::get_states() const { return m_states; }

StateId StateSpace::get_initial_state() const { return m_initial_state; }

const StateIdSet& StateSpace::get_goal_states() const { return m_goal_states; }

const StateIdSet& StateSpace::get_deadend_states() const { return m_deadend_states; }

size_t StateSpace::get_num_states() const { return m_states.size(); }

size_t StateSpace::get_num_goal_states() const { return m_goal_states.size(); }

size_t StateSpace::get_num_deadend_states() const { return m_deadend_states.size(); }

bool StateSpace::is_deadend_state(const State& state) const { return m_goal_distances.at(state.get_id()) < 0; }

// Transitions
size_t StateSpace::get_num_transitions() const { return m_num_transitions; }

const std::vector<TransitionList>& StateSpace::get_forward_transitions() const { return m_forward_transitions; }

const std::vector<TransitionList>& StateSpace::get_backward_transitions() const { return m_backward_transitions; }

// Distances
const std::vector<int>& StateSpace::get_goal_distances() const { return m_goal_distances; }

int StateSpace::get_goal_distance(const State& state) const { return m_goal_distances.at(state.get_id()); }

int StateSpace::get_max_goal_distance() const { return *std::max_element(m_goal_distances.begin(), m_goal_distances.end()); }

// Additional
StateId StateSpace::sample_state_with_goal_distance(int goal_distance) const
{
    const auto& states = m_states_by_goal_distance.at(goal_distance);
    const auto index = std::rand() % static_cast<int>(states.size());
    return states.at(index);
}

}
