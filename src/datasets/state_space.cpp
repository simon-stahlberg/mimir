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
#include "mimir/graphs/static_graph_boost_adapter.hpp"

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
                       std::shared_ptr<IApplicableActionGenerator> aag,
                       std::shared_ptr<StateRepository> ssg,
                       typename StateSpace::GraphType graph,
                       StateMap<StateIndex> state_to_index,
                       StateIndex initial_state,
                       StateIndexSet goal_states,
                       StateIndexSet deadend_states,
                       DistanceList goal_distances) :
    m_problem(problem),
    m_use_unit_cost_one(use_unit_cost_one),
    m_pddl_factories(std::move(pddl_factories)),
    m_aag(std::move(aag)),
    m_ssg(std::move(ssg)),
    m_graph(std::move(graph)),
    m_state_to_index(std::move(state_to_index)),
    m_initial_state(std::move(initial_state)),
    m_goal_states(std::move(goal_states)),
    m_deadend_states(std::move(deadend_states)),
    m_goal_distances(std::move(goal_distances)),
    m_states_by_goal_distance()
{
    for (size_t state_index = 0; state_index < m_graph.get_num_vertices(); ++state_index)
    {
        m_states_by_goal_distance[m_goal_distances.at(state_index)].push_back(state_index);
    }
}

std::optional<StateSpace> StateSpace::create(const fs::path& domain_filepath, const fs::path& problem_filepath, const StateSpaceOptions& options)
{
    auto parser = PDDLParser(domain_filepath, problem_filepath);
    auto aag = std::make_shared<GroundedApplicableActionGenerator>(parser.get_problem(), parser.get_pddl_factories());
    auto ssg = std::make_shared<StateRepository>(aag);
    return StateSpace::create(parser.get_problem(), parser.get_pddl_factories(), aag, ssg, options);
}

std::optional<StateSpace> StateSpace::create(Problem problem,
                                             std::shared_ptr<PDDLFactories> factories,
                                             std::shared_ptr<IApplicableActionGenerator> aag,
                                             std::shared_ptr<StateRepository> ssg,
                                             const StateSpaceOptions& options)
{
    auto stop_watch = StopWatch(options.timeout_ms);

    auto initial_state = ssg->get_or_create_initial_state();

    if (options.remove_if_unsolvable && !problem->static_goal_holds())
    {
        // Unsolvable
        return std::nullopt;
    }

    auto graph = StaticGraph<StateVertex, GroundActionEdge>();
    const auto initial_state_index = graph.add_vertex(State(initial_state));
    auto goal_states = StateIndexSet {};
    auto state_to_index = StateMap<StateIndex> {};
    state_to_index.emplace(initial_state, initial_state_index);

    auto lifo_queue = std::deque<StateVertex>();
    lifo_queue.push_back(graph.get_vertices().at(initial_state_index));

    auto applicable_actions = GroundActionList {};
    stop_watch.start();
    while (!lifo_queue.empty() && !stop_watch.has_finished())
    {
        const auto state = lifo_queue.back();
        const auto state_index = state.get_index();
        lifo_queue.pop_back();
        if (mimir::get_state(state).literals_hold(problem->get_goal_condition<Fluent>())
            && mimir::get_state(state).literals_hold(problem->get_goal_condition<Derived>()))
        {
            goal_states.insert(state_index);
        }

        aag->generate_applicable_actions(mimir::get_state(state), applicable_actions);
        for (const auto& action : applicable_actions)
        {
            const auto successor_state = ssg->get_or_create_successor_state(mimir::get_state(state), action);
            const auto it = state_to_index.find(successor_state);
            const bool exists = (it != state_to_index.end());
            if (exists)
            {
                const auto successor_state_index = it->second;
                graph.add_directed_edge(state_index, successor_state_index, action);
                continue;
            }

            const auto successor_state_index = graph.add_vertex(successor_state);
            if (successor_state_index >= options.max_num_states)
            {
                // Ran out of state resources
                return std::nullopt;
            }

            graph.add_directed_edge(state_index, successor_state_index, action);
            state_to_index.emplace(successor_state, successor_state_index);
            lifo_queue.push_back(graph.get_vertices().at(successor_state_index));
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

    auto bidirectional_graph = typename StateSpace::GraphType(std::move(graph));

    auto goal_distances = DistanceList {};
    if (options.use_unit_cost_one
        || std::all_of(bidirectional_graph.get_edges().begin(),
                       bidirectional_graph.get_edges().end(),
                       [](const auto& transition) { return get_cost(transition) == 1; }))
    {
        auto [predecessors_, goal_distances_] =
            breadth_first_search(TraversalDirectionTaggedType(bidirectional_graph, BackwardTraversal()), goal_states.begin(), goal_states.end());
        goal_distances = std::move(goal_distances_);
    }
    else
    {
        auto transition_costs = TransitionCostList {};
        transition_costs.reserve(bidirectional_graph.get_num_edges());
        for (const auto& transition : bidirectional_graph.get_edges())
        {
            transition_costs.push_back(get_cost(transition));
        }
        auto [predecessors_, goal_distances_] = dijkstra_shortest_paths(TraversalDirectionTaggedType(bidirectional_graph, BackwardTraversal()),
                                                                        transition_costs,
                                                                        goal_states.begin(),
                                                                        goal_states.end());
        goal_distances = std::move(goal_distances_);
    }

    auto deadend_states = StateIndexSet {};
    for (StateIndex state_index = 0; state_index < bidirectional_graph.get_num_vertices(); ++state_index)
    {
        if (goal_distances.at(state_index) == DISTANCE_INFINITY)
        {
            deadend_states.insert(state_index);
        }
    }

    return StateSpace(problem,
                      options.use_unit_cost_one,
                      std::move(factories),
                      std::move(aag),
                      std::move(ssg),
                      std::move(bidirectional_graph),
                      std::move(state_to_index),
                      initial_state_index,
                      std::move(goal_states),
                      std::move(deadend_states),
                      std::move(goal_distances));
}

StateSpaceList StateSpace::create(const fs::path& domain_filepath, const std::vector<fs::path>& problem_filepaths, const StateSpacesOptions& options)
{
    auto memories =
        std::vector<std::tuple<Problem, std::shared_ptr<PDDLFactories>, std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>>> {};
    for (const auto& problem_filepath : problem_filepaths)
    {
        auto parser = PDDLParser(domain_filepath, problem_filepath);
        auto aag = std::make_shared<GroundedApplicableActionGenerator>(parser.get_problem(), parser.get_pddl_factories());
        auto ssg = std::make_shared<StateRepository>(aag);
        memories.emplace_back(parser.get_problem(), parser.get_pddl_factories(), aag, ssg);
    }

    return StateSpace::create(memories, options);
}

std::vector<StateSpace> StateSpace::create(
    const std::vector<std::tuple<Problem, std::shared_ptr<PDDLFactories>, std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>>>&
        memories,
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

template<IsTraversalDirection Direction>
DistanceList StateSpace::compute_shortest_distances_from_states(const StateIndexList& states) const
{
    auto distances = DistanceList {};
    if (m_use_unit_cost_one
        || std::all_of(m_graph.get_edges().begin(), m_graph.get_edges().end(), [](const auto& transition) { return get_cost(transition) == 1; }))
    {
        auto [predecessors_, distances_] = breadth_first_search(TraversalDirectionTaggedType(m_graph, Direction()), states.begin(), states.end());
        distances = std::move(distances_);
    }
    else
    {
        auto transition_costs = TransitionCostList {};
        transition_costs.reserve(m_graph.get_num_edges());
        for (const auto& transition : m_graph.get_edges())
        {
            transition_costs.push_back(get_cost(transition));
        }
        auto [predecessors_, distances_] =
            dijkstra_shortest_paths(TraversalDirectionTaggedType(m_graph, Direction()), transition_costs, states.begin(), states.end());
        distances = std::move(distances_);
    }

    return distances;
}

template DistanceList StateSpace::compute_shortest_distances_from_states<ForwardTraversal>(const StateIndexList& states) const;
template DistanceList StateSpace::compute_shortest_distances_from_states<BackwardTraversal>(const StateIndexList& states) const;

template<IsTraversalDirection Direction>
DistanceMatrix StateSpace::compute_pairwise_shortest_state_distances() const
{
    auto transition_costs = TransitionCostList {};
    if (m_use_unit_cost_one)
    {
        transition_costs = TransitionCostList(m_graph.get_num_edges(), 1);
    }
    else
    {
        transition_costs.reserve(m_graph.get_num_edges());
        for (const auto& transition : m_graph.get_edges())
        {
            transition_costs.push_back(get_cost(transition));
        }
    }

    return floyd_warshall_all_pairs_shortest_paths(TraversalDirectionTaggedType(m_graph, Direction()), transition_costs).get_matrix();
}

template DistanceMatrix StateSpace::compute_pairwise_shortest_state_distances<ForwardTraversal>() const;
template DistanceMatrix StateSpace::compute_pairwise_shortest_state_distances<BackwardTraversal>() const;

/**
 *  Getters
 */

/* Meta data */
Problem StateSpace::get_problem() const { return m_problem; }
bool StateSpace::get_use_unit_cost_one() const { return m_use_unit_cost_one; }

/* Memory */
const std::shared_ptr<PDDLFactories>& StateSpace::get_pddl_factories() const { return m_pddl_factories; }

const std::shared_ptr<IApplicableActionGenerator>& StateSpace::get_aag() const { return m_aag; }

const std::shared_ptr<StateRepository>& StateSpace::get_ssg() const { return m_ssg; }

/* Graph */
const typename StateSpace::GraphType& StateSpace::get_graph() const { return m_graph; }

/* States */
const StateVertexList& StateSpace::get_states() const { return m_graph.get_vertices(); }

const StateVertex& StateSpace::get_state(StateIndex state) const { return m_graph.get_vertices().at(state); }

template<IsTraversalDirection Direction>
std::ranges::subrange<StateSpace::AdjacentVertexConstIteratorType<Direction>> StateSpace::get_adjacent_states(StateIndex state) const
{
    return m_graph.get_adjacent_vertices<Direction>(state);
}

template std::ranges::subrange<StateSpace::AdjacentVertexConstIteratorType<ForwardTraversal>>
StateSpace::get_adjacent_states<ForwardTraversal>(StateIndex state) const;
template std::ranges::subrange<StateSpace::AdjacentVertexConstIteratorType<BackwardTraversal>>
StateSpace::get_adjacent_states<BackwardTraversal>(StateIndex state) const;

template<IsTraversalDirection Direction>
std::ranges::subrange<StateSpace::AdjacentVertexIndexConstIteratorType<Direction>> StateSpace::get_adjacent_state_indices(StateIndex state) const
{
    return m_graph.get_adjacent_vertex_indices<Direction>(state);
}

template std::ranges::subrange<StateSpace::AdjacentVertexIndexConstIteratorType<ForwardTraversal>>
StateSpace::get_adjacent_state_indices<ForwardTraversal>(StateIndex state) const;
template std::ranges::subrange<StateSpace::AdjacentVertexIndexConstIteratorType<BackwardTraversal>>
StateSpace::get_adjacent_state_indices<BackwardTraversal>(StateIndex state) const;

StateIndex StateSpace::get_state_index(State state) const { return m_state_to_index.at(state); }

StateIndex StateSpace::get_initial_state() const { return m_initial_state; }

const StateIndexSet& StateSpace::get_goal_states() const { return m_goal_states; }

const StateIndexSet& StateSpace::get_deadend_states() const { return m_deadend_states; }

size_t StateSpace::get_num_states() const { return m_graph.get_num_vertices(); }

size_t StateSpace::get_num_goal_states() const { return get_goal_states().size(); }

size_t StateSpace::get_num_deadend_states() const { return get_deadend_states().size(); }

bool StateSpace::is_goal_state(StateIndex state) const { return get_goal_states().count(state); }

bool StateSpace::is_deadend_state(StateIndex state) const { return get_deadend_states().count(state); }

bool StateSpace::is_alive_state(StateIndex state) const { return !(get_goal_states().count(state) || get_deadend_states().count(state)); }

/* Transitions */
const GroundActionEdgeList& StateSpace::get_transitions() const { return m_graph.get_edges(); }

template<IsTraversalDirection Direction>
std::ranges::subrange<StateSpace::AdjacentEdgeConstIteratorType<Direction>> StateSpace::get_adjacent_transitions(StateIndex state) const
{
    return m_graph.get_adjacent_edges<Direction>(state);
}

template std::ranges::subrange<StateSpace::AdjacentEdgeConstIteratorType<ForwardTraversal>>
StateSpace::get_adjacent_transitions<ForwardTraversal>(StateIndex state) const;
template std::ranges::subrange<StateSpace::AdjacentEdgeConstIteratorType<BackwardTraversal>>
StateSpace::get_adjacent_transitions<BackwardTraversal>(StateIndex state) const;

template<IsTraversalDirection Direction>
std::ranges::subrange<StateSpace::AdjacentEdgeIndexConstIteratorType<Direction>> StateSpace::get_adjacent_transition_indices(StateIndex state) const
{
    return m_graph.get_adjacent_edge_indices<Direction>(state);
}

template std::ranges::subrange<StateSpace::AdjacentEdgeIndexConstIteratorType<ForwardTraversal>>
StateSpace::get_adjacent_transition_indices<ForwardTraversal>(StateIndex state) const;
template std::ranges::subrange<StateSpace::AdjacentEdgeIndexConstIteratorType<BackwardTraversal>>
StateSpace::get_adjacent_transition_indices<BackwardTraversal>(StateIndex state) const;

TransitionCost StateSpace::get_transition_cost(TransitionIndex transition) const
{
    return (m_use_unit_cost_one) ? 1 : get_cost(m_graph.get_edges().at(transition));
}

size_t StateSpace::get_num_transitions() const { return m_graph.get_num_edges(); }

/* Distances */
const DistanceList& StateSpace::get_goal_distances() const { return m_goal_distances; }

Distance StateSpace::get_goal_distance(StateIndex state) const { return m_goal_distances.at(state); }

Distance StateSpace::get_max_goal_distance() const { return *std::max_element(m_goal_distances.begin(), m_goal_distances.end()); }

/* Additional */
const std::map<Distance, StateIndexList>& StateSpace::get_states_by_goal_distance() const { return m_states_by_goal_distance; }

StateIndex StateSpace::sample_state_with_goal_distance(Distance goal_distance) const
{
    const auto& states = m_states_by_goal_distance.at(goal_distance);
    const auto index = std::rand() % static_cast<int>(states.size());
    return states.at(index);
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
        out << "label=\""
            << std::make_tuple(state_space.get_problem(),
                               mimir::get_state(state_space.get_graph().get_vertices().at(state_index)),
                               std::cref(*state_space.get_pddl_factories()))
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
    for (const auto& transition : state_space.get_graph().get_edges())
    {
        // direction
        out << "s" << transition.get_source() << "->"
            << "s" << transition.get_target() << " [";

        // label
        out << "label=\"" << get_creating_action(transition) << "\"";

        out << "]\n";
    }
    out << "}\n";

    return out;
}
}
