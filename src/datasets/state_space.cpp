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
#include "mimir/search/openlists/priority_queue.hpp"

#include <algorithm>
#include <cstdlib>
#include <deque>

namespace mimir
{
/**
 * StateSpace
 */
StateSpace::StateSpace(Problem problem,
                       bool use_unit_cost_one,
                       std::shared_ptr<PDDLFactories> pddl_factories,
                       std::shared_ptr<IAAG> aag,
                       std::shared_ptr<SuccessorStateGenerator> ssg,
                       StateList states,
                       StateMap<StateIndex> state_to_index,
                       StateIndex initial_state,
                       StateIndexSet goal_states,
                       StateIndexSet deadend_states,
                       IndexGroupedVector<const Transition> transitions,
                       std::vector<double> goal_distances) :
    m_problem(problem),
    m_use_unit_cost_one(use_unit_cost_one),
    m_pddl_factories(std::move(pddl_factories)),
    m_aag(std::move(aag)),
    m_ssg(std::move(ssg)),
    m_states(std::move(states)),
    m_state_to_index(std::move(state_to_index)),
    m_initial_state(std::move(initial_state)),
    m_goal_states(std::move(goal_states)),
    m_deadend_states(std::move(deadend_states)),
    m_transitions(std::move(transitions)),
    m_goal_distances(std::move(goal_distances)),
    m_states_by_goal_distance()
{
    for (size_t state_id = 0; state_id < m_states.size(); ++state_id)
    {
        m_states_by_goal_distance[m_goal_distances.at(state_id)].push_back(state_id);
    }
}

std::optional<StateSpace> StateSpace::create(const fs::path& domain_filepath, const fs::path& problem_filepath, const StateSpaceOptions& options)
{
    auto parser = PDDLParser(domain_filepath, problem_filepath);
    auto aag = std::make_shared<GroundedAAG>(parser.get_problem(), parser.get_factories());
    auto ssg = std::make_shared<SuccessorStateGenerator>(aag);
    return StateSpace::create(parser.get_problem(), parser.get_factories(), aag, ssg, options);
}

std::optional<StateSpace> StateSpace::create(Problem problem,
                                             std::shared_ptr<PDDLFactories> factories,
                                             std::shared_ptr<IAAG> aag,
                                             std::shared_ptr<SuccessorStateGenerator> ssg,
                                             const StateSpaceOptions& options)
{
    auto stop_watch = StopWatch(options.timeout_ms);

    auto initial_state = ssg->get_or_create_initial_state();

    if (options.remove_if_unsolvable && !problem->static_goal_holds())
    {
        // Unsolvable
        return std::nullopt;
    }

    auto lifo_queue = std::deque<State>();
    lifo_queue.push_back(initial_state);

    const auto initial_state_index = 0;
    auto states = StateList { initial_state };
    auto state_to_index = StateMap<StateIndex> {};
    state_to_index[initial_state] = initial_state_index;
    auto transitions = TransitionList {};
    auto goal_states = StateIndexSet {};

    auto applicable_actions = GroundActionList {};
    stop_watch.start();
    while (!lifo_queue.empty() && !stop_watch.has_finished())
    {
        const auto state = lifo_queue.back();
        const auto state_index = state_to_index.at(state);
        lifo_queue.pop_back();
        if (state.literals_hold(problem->get_goal_condition<Fluent>()) && state.literals_hold(problem->get_goal_condition<Derived>()))
        {
            goal_states.insert(state_index);
        }

        aag->generate_applicable_actions(state, applicable_actions);
        for (const auto& action : applicable_actions)
        {
            const auto successor_state = ssg->get_or_create_successor_state(state, action);
            const auto it = state_to_index.find(successor_state);
            const bool exists = (it != state_to_index.end());
            if (exists)
            {
                const auto successor_state_index = it->second;
                transitions.emplace_back(transitions.size(), state_index, successor_state_index, action);
                continue;
            }

            const auto successor_state_index = states.size();
            if (successor_state_index >= options.max_num_states)
            {
                // Ran out of state resources
                return std::nullopt;
            }

            transitions.emplace_back(transitions.size(), state_index, successor_state_index, action);
            states.push_back(successor_state);
            state_to_index[successor_state] = successor_state_index;
            lifo_queue.push_back(successor_state);
        }
    }

    if (stop_watch.has_finished())
    {
        // Ran out of time
        return std::nullopt;
    }

    if (options.remove_if_unsolvable && goal_states.empty())
    {
        // Skip: unsolvable
        return std::nullopt;
    }

    auto goal_distances = mimir::compute_shortest_goal_distances(states.size(), goal_states, transitions, options.use_unit_cost_one);

    auto deadend_states = StateIndexSet {};
    for (StateIndex state_id = 0; state_id < states.size(); ++state_id)
    {
        if (goal_distances.at(state_id) == std::numeric_limits<double>::max())
        {
            deadend_states.insert(state_id);
        }
    }

    /* Sort transitions by source state and recreate indexing. */
    std::sort(transitions.begin(), transitions.end(), [](const auto& l, const auto& r) { return l.get_source_state() < r.get_source_state(); });
    auto transition_index = TransitionIndex { 0 };
    std::transform(transitions.begin(),
                   transitions.end(),
                   transitions.begin(),
                   [&transition_index](const auto& transition)
                   { return Transition(transition_index++, transition.get_source_state(), transition.get_target_state(), transition.get_creating_action()); });

    /* Group transitions by source. */
    auto grouped_transitions = IndexGroupedVector<const Transition>::create(
        std::move(transitions),
        [](const auto& l, const auto& r) { return l.get_source_state() != r.get_source_state(); },
        [](const auto& e) { return e.get_source_state(); },
        states.size());

    return StateSpace(problem,
                      options.use_unit_cost_one,
                      std::move(factories),
                      std::move(aag),
                      std::move(ssg),
                      std::move(states),
                      std::move(state_to_index),
                      initial_state_index,
                      std::move(goal_states),
                      std::move(deadend_states),
                      std::move(grouped_transitions),
                      std::move(goal_distances));
}

StateSpaceList StateSpace::create(const fs::path& domain_filepath, const std::vector<fs::path>& problem_filepaths, const StateSpacesOptions& options)
{
    auto memories = std::vector<std::tuple<Problem, std::shared_ptr<PDDLFactories>, std::shared_ptr<IAAG>, std::shared_ptr<SuccessorStateGenerator>>> {};
    for (const auto& problem_filepath : problem_filepaths)
    {
        auto parser = PDDLParser(domain_filepath, problem_filepath);
        auto aag = std::make_shared<GroundedAAG>(parser.get_problem(), parser.get_factories());
        auto ssg = std::make_shared<SuccessorStateGenerator>(aag);
        memories.emplace_back(parser.get_problem(), parser.get_factories(), aag, ssg);
    }

    return StateSpace::create(memories, options);
}

std::vector<StateSpace> StateSpace::create(
    const std::vector<std::tuple<Problem, std::shared_ptr<PDDLFactories>, std::shared_ptr<IAAG>, std::shared_ptr<SuccessorStateGenerator>>>& memories,
    const StateSpacesOptions& options)
{
    auto state_spaces = StateSpaceList {};
    auto pool = BS::thread_pool(options.num_threads);
    auto futures = std::vector<std::future<std::optional<StateSpace>>> {};

    for (const auto& [problem, factories, aag, ssg] : memories)
    {
        futures.push_back(pool.submit_task([problem, factories, aag, ssg, state_space_options = options.state_space_options]
                                           { return StateSpace::create(problem, factories, aag, ssg, state_space_options); }));
    }

    for (auto& future : futures)
    {
        auto state_space = future.get();
        if (state_space.has_value())
        {
            state_spaces.push_back(std::move(state_space.value()));
        }
    }

    if (options.sort_ascending_by_num_states)
    {
        std::sort(state_spaces.begin(), state_spaces.end(), [](const auto& l, const auto& r) { return l.get_num_states() < r.get_num_states(); });
    }

    return state_spaces;
}

/**
 * Extended functionality
 */

std::vector<double> StateSpace::compute_shortest_distances_from_states(const StateIndexList& states, const bool forward) const
{
    throw std::runtime_error("Not implemented");
}

std::vector<std::vector<double>> StateSpace::compute_pairwise_shortest_state_distances(const bool forward) const
{
    throw std::runtime_error("Not implemented");
}

/**
 *  Getters
 */

/* Meta data */
Problem StateSpace::get_problem() const { return m_problem; }
bool StateSpace::get_use_unit_cost_one() const { return m_use_unit_cost_one; }

/* Memory */
const std::shared_ptr<PDDLFactories>& StateSpace::get_pddl_factories() const { return m_pddl_factories; }

const std::shared_ptr<IAAG>& StateSpace::get_aag() const { return m_aag; }

const std::shared_ptr<SuccessorStateGenerator>& StateSpace::get_ssg() const { return m_ssg; }

/* States */
const StateList& StateSpace::get_states() const { return m_states; }

StateIndex StateSpace::get_state_index(State state) const { return m_state_to_index.at(state); }

StateIndex StateSpace::get_initial_state() const { return m_initial_state; }

const StateIndexSet& StateSpace::get_goal_states() const { return m_goal_states; }

const StateIndexSet& StateSpace::get_deadend_states() const { return m_deadend_states; }

TargetStateIndexIterator<Transition> StateSpace::get_target_states(StateIndex source) const
{
    return TargetStateIndexIterator<Transition>(m_transitions.at(source));
}

SourceStateIndexIterator<Transition> StateSpace::get_source_states(StateIndex target) const
{
    return SourceStateIndexIterator<Transition>(target, m_transitions.data());
}

size_t StateSpace::get_num_states() const { return get_states().size(); }

size_t StateSpace::get_num_goal_states() const { return get_goal_states().size(); }

size_t StateSpace::get_num_deadend_states() const { return get_deadend_states().size(); }

bool StateSpace::is_goal_state(StateIndex state) const { return get_goal_states().count(state); }

bool StateSpace::is_deadend_state(StateIndex state) const { return get_deadend_states().count(state); }

bool StateSpace::is_alive_state(StateIndex state) const { return !(get_goal_states().count(state) || get_deadend_states().count(state)); }

/* Transitions */
const TransitionList& StateSpace::get_transitions() const { return m_transitions.data(); }

TransitionCost StateSpace::get_transition_cost(TransitionIndex transition) const
{
    return (m_use_unit_cost_one) ? 1 : m_transitions.data().at(transition).get_cost();
}

ForwardTransitionIndexIterator<Transition> StateSpace::get_forward_transition_indices(StateIndex source) const
{
    return ForwardTransitionIndexIterator<Transition>(m_transitions.at(source));
}

BackwardTransitionIndexIterator<Transition> StateSpace::get_backward_transition_indices(StateIndex target) const
{
    return BackwardTransitionIndexIterator<Transition>(target, m_transitions.data());
}

ForwardTransitionIterator<Transition> StateSpace::get_forward_transitions(StateIndex source) const
{
    return ForwardTransitionIterator<Transition>(m_transitions.at(source));
}

BackwardTransitionIterator<Transition> StateSpace::get_backward_transitions(StateIndex target) const
{
    return BackwardTransitionIterator<Transition>(target, m_transitions.data());
}

size_t StateSpace::get_num_transitions() const { return m_transitions.data().size(); }

/* Distances */
const std::vector<double>& StateSpace::get_goal_distances() const { return m_goal_distances; }

double StateSpace::get_goal_distance(State state) const { return m_goal_distances.at(get_state_index(state)); }

double StateSpace::get_max_goal_distance() const { return *std::max_element(m_goal_distances.begin(), m_goal_distances.end()); }

/* Additional */
const std::map<double, StateIndexList>& StateSpace::get_states_by_goal_distance() const { return m_states_by_goal_distance; }

StateIndex StateSpace::sample_state_with_goal_distance(double goal_distance) const
{
    const auto& states = m_states_by_goal_distance.at(goal_distance);
    const auto index = std::rand() % static_cast<int>(states.size());
    return states.at(index);
}

/**
 * Distances
 */

std::vector<double>
compute_shortest_goal_distances(size_t num_total_states, const StateIndexSet& goal_states, const TransitionList& transitions, bool use_unit_cost_one)
{
    auto distances = std::vector<double>(num_total_states, std::numeric_limits<double>::max());
    auto closed = std::vector<bool>(num_total_states, false);
    auto priority_queue = PriorityQueue<StateIndex>();
    for (const auto& state : goal_states)
    {
        distances.at(state) = 0.;
        priority_queue.insert(0., state);
    }

    while (!priority_queue.empty())
    {
        const auto state_index = priority_queue.top();
        priority_queue.pop();
        const auto cost = distances.at(state_index);

        if (closed.at(state_index))
        {
            continue;
        }
        closed.at(state_index) = true;

        for (const auto& transition : transitions)
        {
            if (transition.get_target_state() != state_index)
            {
                continue;
            }
            const auto successor_state = transition.get_source_state();

            auto succ_cost = distances.at(successor_state);
            auto new_succ_cost = cost + ((use_unit_cost_one) ? 1. : transition.get_creating_action().get_cost());

            if (new_succ_cost < succ_cost)
            {
                distances.at(successor_state) = new_succ_cost;
                // decrease priority
                priority_queue.insert(new_succ_cost, successor_state);
            }
        }
    }

    return distances;
}

/**
 * Pretty printing
 */

std::ostream& operator<<(std::ostream& out, const StateSpace& state_space)
{
    // 2. Header
    out << "digraph {"
        << "\n"
        << "rankdir=\"LR\""
        << "\n";

    // 3. Draw states
    for (size_t state_index = 0; state_index < state_space.get_num_states(); ++state_index)
    {
        out << "s" << state_index << "[";
        if (state_space.is_goal_state(state_index))
        {
            out << "peripheries=2,";
        }

        // label
        out << "label=\"" << std::make_tuple(state_space.get_problem(), state_space.get_states().at(state_index), std::cref(*state_space.get_pddl_factories()))
            << "\"";

        out << "]\n";
    }

    // 4. Draw initial state and dangling edge
    out << "Dangling [ label = \"\", style = invis ]\n"
        << "{ rank = same; Dangling }\n"
        << "Dangling -> s" << state_space.get_initial_state() << "\n";

    // 5. Group states with same distance together
    for (auto it = state_space.get_states_by_goal_distance().rbegin(); it != state_space.get_states_by_goal_distance().rend(); ++it)
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
    for (const auto& transition : state_space.get_transitions())
    {
        // direction
        out << "s" << transition.get_source_state() << "->"
            << "s" << transition.get_target_state() << " [";

        // label
        out << "label=\"" << transition.get_creating_action() << "\"";

        out << "]\n";
    }
    out << "}\n";

    return out;
}
}
