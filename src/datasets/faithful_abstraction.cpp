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
#include "mimir/algorithms/nauty.hpp"
#include "mimir/common/equal_to.hpp"
#include "mimir/common/timers.hpp"
#include "mimir/graphs/static_graph_boost_adapter.hpp"

#include <algorithm>
#include <cstdlib>
#include <deque>
#include <pthread.h>
#include <thread>

namespace mimir
{

/**
 * FaithfulAbstraction
 */

FaithfulAbstraction::FaithfulAbstraction(Problem problem,
                                         bool mark_true_goal_literals,
                                         bool use_unit_cost_one,
                                         std::shared_ptr<PDDLFactories> factories,
                                         std::shared_ptr<IApplicableActionGenerator> aag,
                                         std::shared_ptr<StateRepository> ssg,
                                         typename FaithfulAbstraction::GraphType graph,
                                         std::shared_ptr<const StateList> concrete_states_by_abstract_state,
                                         StateMap<StateIndex> concrete_to_abstract_state,
                                         StateIndex initial_state,
                                         StateIndexSet goal_states,
                                         StateIndexSet deadend_states,
                                         std::shared_ptr<const GroundActionList> ground_actions_by_source_and_target,
                                         DistanceList goal_distances) :
    m_problem(problem),
    m_mark_true_goal_literals(mark_true_goal_literals),
    m_use_unit_cost_one(use_unit_cost_one),
    m_pddl_factories(std::move(factories)),
    m_aag(std::move(aag)),
    m_ssg(std::move(ssg)),
    m_graph(std::move(graph)),
    m_concrete_states_by_abstract_state(std::move(concrete_states_by_abstract_state)),
    m_concrete_to_abstract_state(std::move(concrete_to_abstract_state)),
    m_initial_state(initial_state),
    m_goal_states(std::move(goal_states)),
    m_deadend_states(std::move(deadend_states)),
    m_ground_actions_by_source_and_target(std::move(ground_actions_by_source_and_target)),
    m_goal_distances(std::move(goal_distances)),
    m_states_by_goal_distance()
{
    /* Ensure correctness. */

    // Check correct state ordering
    for (size_t i = 0; i < get_num_states(); ++i)
    {
        assert(get_states().at(i).get_index() == static_cast<StateIndex>(i) && "State index does not match its position in the list");
    }

    // Check correct transition ordering
    for (size_t i = 0; i < get_num_transitions(); ++i)
    {
        assert(get_transitions().at(i).get_index() == static_cast<TransitionIndex>(i) && "Transition index does not match its position in the list");
    }

    /* Additional */
    for (size_t state_index = 0; state_index < m_graph.get_num_vertices(); ++state_index)
    {
        m_states_by_goal_distance[m_goal_distances.at(state_index)].push_back(state_index);
    }
}

std::optional<FaithfulAbstraction>
FaithfulAbstraction::create(const fs::path& domain_filepath, const fs::path& problem_filepath, const FaithfulAbstractionOptions& options)
{
    auto parser = PDDLParser(domain_filepath, problem_filepath);
    auto aag = std::make_shared<LiftedApplicableActionGenerator>(parser.get_problem(), parser.get_pddl_factories());
    auto ssg = std::make_shared<StateRepository>(aag);

    return FaithfulAbstraction::create(parser.get_problem(), parser.get_pddl_factories(), aag, ssg, options);
}

std::optional<FaithfulAbstraction> FaithfulAbstraction::create(Problem problem,
                                                               std::shared_ptr<PDDLFactories> factories,
                                                               std::shared_ptr<IApplicableActionGenerator> aag,
                                                               std::shared_ptr<StateRepository> ssg,
                                                               const FaithfulAbstractionOptions& options)
{
    auto stop_watch = StopWatch(options.timeout_ms);

    auto initial_state = ssg->get_or_create_initial_state();

    if (options.remove_if_unsolvable && !problem->static_goal_holds())
    {
        // Unsolvable
        return std::nullopt;
    }

    auto concrete_to_abstract_state = StateMap<StateIndex> {};
    auto abstract_states_by_certificate =
        std::unordered_map<std::shared_ptr<const Certificate>, StateIndex, UniqueCertificateSharedPtrHash, UniqueCertificateSharedPtrEqualTo> {};

    /* Initialize for initial state. */
    const auto color_function = ProblemColorFunction(problem);

    auto object_graph_pruning_strategy = std::unique_ptr<ObjectGraphPruningStrategy> { nullptr };
    if (options.pruning_strategy == ObjectGraphPruningStrategyEnum::StaticScc)
    {
        auto state_space_options = StateSpaceOptions();
        state_space_options.max_num_states = options.max_num_concrete_states;
        state_space_options.timeout_ms = options.timeout_ms;
        auto scc_pruning_strategy = ObjectGraphStaticSccPruningStrategy::create(problem, factories, aag, ssg, state_space_options);
        if (!scc_pruning_strategy.has_value())
        {
            return std::nullopt;
        }
        object_graph_pruning_strategy = std::make_unique<ObjectGraphStaticSccPruningStrategy>(std::move(scc_pruning_strategy.value()));
    }
    else
    {
        object_graph_pruning_strategy = std::make_unique<ObjectGraphPruningStrategy>(ObjectGraphPruningStrategy());
    }
    assert(object_graph_pruning_strategy);

    const auto object_graph = create_object_graph(color_function,
                                                  *factories,
                                                  problem,
                                                  initial_state,
                                                  initial_state.get_index(),
                                                  options.mark_true_goal_literals,
                                                  *object_graph_pruning_strategy);
    // std::cout << problem->get_filepath().value() << std::endl;
    // std::cout << std::make_tuple(std::cref(object_graph), std::cref(color_function)) << std::endl;
    auto certificate = std::make_shared<const Certificate>(object_graph.get_num_vertices(),
                                                           object_graph.get_num_edges(),
                                                           nauty_wrapper::SparseGraph(object_graph).compute_certificate(),
                                                           compute_sorted_vertex_colors(object_graph));
    const auto abstract_initial_state_index = 0;
    abstract_states_by_certificate.emplace(std::move(certificate), abstract_initial_state_index);
    concrete_to_abstract_state.emplace(initial_state, abstract_initial_state_index);

    /* Initialize search. */
    auto lifo_queue = std::deque<State>();
    lifo_queue.push_back(initial_state);
    auto transitions = GroundActionEdgeList {};
    auto abstract_goal_states = StateIndexSet {};
    auto applicable_actions = GroundActionList {};
    auto next_abstract_state_index = StateIndex { 1 };
    stop_watch.start();

    while (!lifo_queue.empty() && !stop_watch.has_finished())
    {
        const auto state = lifo_queue.back();
        const auto abstract_state_index = concrete_to_abstract_state.at(state);

        lifo_queue.pop_back();

        if (state.literals_hold(problem->get_goal_condition<Fluent>()) && state.literals_hold(problem->get_goal_condition<Derived>()))
        {
            abstract_goal_states.insert(abstract_state_index);
        }

        aag->generate_applicable_actions(state, applicable_actions);

        for (const auto& action : applicable_actions)
        {
            const auto successor_state = ssg->get_or_create_successor_state(state, action);

            // Regenerate concrete state
            const auto concrete_successor_state_exists = concrete_to_abstract_state.count(successor_state);
            if (concrete_successor_state_exists)
            {
                const auto abstract_successor_state_index = concrete_to_abstract_state.at(successor_state);
                transitions.emplace_back(transitions.size(), abstract_state_index, abstract_successor_state_index, action);
                continue;
            }

            // Compute certificate of successor state
            const auto object_graph = create_object_graph(color_function,
                                                          *factories,
                                                          problem,
                                                          successor_state,
                                                          successor_state.get_index(),
                                                          options.mark_true_goal_literals,
                                                          *object_graph_pruning_strategy);
            // std::cout << std::make_tuple(std::cref(object_graph), std::cref(color_function)) << std::endl;

            auto certificate = std::make_shared<const Certificate>(object_graph.get_num_vertices(),
                                                                   object_graph.get_num_edges(),
                                                                   nauty_wrapper::SparseGraph(object_graph).compute_certificate(),
                                                                   compute_sorted_vertex_colors(object_graph));
            const auto it = abstract_states_by_certificate.find(certificate);

            // Regenerate abstract state
            const auto abstract_state_exists = (it != abstract_states_by_certificate.end());

            if (abstract_state_exists)
            {
                /* Add concrete state to abstraction mapping. */
                concrete_to_abstract_state.emplace(successor_state, it->second);
            }
            else
            {
                /* Generate new abstract state and add concrete state to abstraction mapping.  */
                const auto abstract_successor_state_index = next_abstract_state_index++;
                abstract_states_by_certificate.emplace(std::move(certificate), abstract_successor_state_index);
                concrete_to_abstract_state.emplace(successor_state, abstract_successor_state_index);

                if (next_abstract_state_index >= options.max_num_abstract_states)
                {
                    // Ran out of state resources
                    return std::nullopt;
                }
            }

            const auto abstract_successor_state_index = concrete_to_abstract_state.at(successor_state);
            transitions.emplace_back(transitions.size(), abstract_state_index, abstract_successor_state_index, action);
            concrete_to_abstract_state.emplace(successor_state, abstract_successor_state_index);

            if (concrete_to_abstract_state.size() >= options.max_num_concrete_states)
            {
                // Ran out of state resources
                return std::nullopt;
            }

            if (options.compute_complete_abstraction_mapping || !abstract_state_exists)
            {
                lifo_queue.push_back(successor_state);
            }
        }
    }

    if (stop_watch.has_finished())
    {
        // Ran out of time
        return std::nullopt;
    }

    if (options.remove_if_unsolvable && abstract_goal_states.empty())
    {
        // Skip: unsolvable
        return std::nullopt;
    }

    const auto num_abstract_states = next_abstract_state_index;

    /* Sort concrete states by abstract state */
    auto concrete_states_by_abstract_state = std::make_shared<StateList>();
    concrete_states_by_abstract_state->reserve(num_abstract_states);
    auto concrete_states_begin_by_abstract_state = std::vector<StateIndex> {};
    for (StateIndex cur_abstract_state_index = 0; cur_abstract_state_index < num_abstract_states; ++cur_abstract_state_index)
    {
        concrete_states_begin_by_abstract_state.push_back(concrete_states_by_abstract_state->size());
        for (const auto& [concrete_state, abstract_state_index] : concrete_to_abstract_state)
        {
            if (abstract_state_index == cur_abstract_state_index)
            {
                concrete_states_by_abstract_state->push_back(concrete_state);
            }
        }
    }
    concrete_states_begin_by_abstract_state.push_back(concrete_states_by_abstract_state->size());
    assert(concrete_states_begin_by_abstract_state.size() == num_abstract_states + 1);

    /* Construct abstract states and sort by index. */
    auto abstract_states = FaithfulAbstractStateVertexList {};
    abstract_states.reserve(num_abstract_states);
    for (const auto& [certificate, abstract_state_index] : abstract_states_by_certificate)
    {
        abstract_states.emplace_back(
            abstract_state_index,
            std::span<State>(concrete_states_by_abstract_state->begin() + concrete_states_begin_by_abstract_state.at(abstract_state_index),
                             concrete_states_by_abstract_state->begin() + concrete_states_begin_by_abstract_state.at(abstract_state_index + 1)),
            certificate);
    }
    std::sort(abstract_states.begin(), abstract_states.end(), [](const auto& l, const auto& r) { return l.get_index() < r.get_index(); });

    /* Sort concrete transitions by source and target state. */
    std::sort(transitions.begin(),
              transitions.end(),
              [](const auto& l, const auto& r)
              {
                  if (l.get_source() == r.get_source())
                  {
                      return l.get_target() < r.get_target();
                  }
                  return l.get_source() < r.get_source();
              });

    /* Group ground actions by source and target and store persistent to be able to use span. */
    auto ground_actions_by_source_and_target = std::make_shared<GroundActionList>();
    std::transform(transitions.begin(),
                   transitions.end(),
                   std::back_inserter(*ground_actions_by_source_and_target),
                   [](const auto& transition) { return get_creating_action(transition); });

    /* Group concrete transitions by source and target */
    auto grouped_transitions = IndexGroupedVector<const GroundActionEdge>::create(
        std::move(transitions),
        [](const auto& l, const auto& r) { return ((l.get_source() != r.get_source()) || (l.get_target() != r.get_target())); });

    /* Create abstract transitions from groups. */
    auto abstract_transitions = GroundActionsEdgeList {};
    abstract_transitions.reserve(grouped_transitions.size());
    auto accumulated_transitions = 0;
    for (const auto& group : grouped_transitions)
    {
        assert(!group.empty());

        abstract_transitions.emplace_back(abstract_transitions.size(),
                                          group.front().get_source(),
                                          group.front().get_target(),
                                          std::span<const GroundAction>(ground_actions_by_source_and_target->begin() + accumulated_transitions,
                                                                        ground_actions_by_source_and_target->begin() + accumulated_transitions + group.size()));
        accumulated_transitions += group.size();
    }

    /* Create graph */
    auto graph = StaticGraph<FaithfulAbstractStateVertex, GroundActionsEdge>();
    for (const auto& abstract_state : abstract_states)
    {
        graph.add_vertex(mimir::get_states(abstract_state), mimir::get_certificate(abstract_state));
    }
    for (const auto& abstract_transition : abstract_transitions)
    {
        graph.add_directed_edge(abstract_transition.get_source(), abstract_transition.get_target(), get_actions(abstract_transition));
    }
    auto bidirectional_graph = typename FaithfulAbstraction::GraphType(std::move(graph));

    /* Compute abstract goal distances */

    auto abstract_goal_distances = DistanceList {};
    if (options.use_unit_cost_one
        || std::all_of(bidirectional_graph.get_edges().begin(),
                       bidirectional_graph.get_edges().end(),
                       [](const auto& transition) { return get_cost(transition) == 1; }))
    {
        auto [predecessors_, goal_distances_] = breadth_first_search(TraversalDirectionTaggedType(bidirectional_graph, BackwardTraversal()),
                                                                     abstract_goal_states.begin(),
                                                                     abstract_goal_states.end());
        abstract_goal_distances = std::move(goal_distances_);
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
                                                                        abstract_goal_states.begin(),
                                                                        abstract_goal_states.end());
        abstract_goal_distances = std::move(goal_distances_);
    }

    /* Compute deadend states. */
    auto abstract_deadend_states = StateIndexSet {};
    for (StateIndex abstract_state_index = 0; abstract_state_index < num_abstract_states; ++abstract_state_index)
    {
        if (abstract_goal_distances.at(abstract_state_index) == DISTANCE_INFINITY)
        {
            abstract_deadend_states.insert(abstract_state_index);
        }
    }

    return FaithfulAbstraction(problem,
                               options.mark_true_goal_literals,
                               options.use_unit_cost_one,
                               std::move(factories),
                               std::move(aag),
                               std::move(ssg),
                               std::move(bidirectional_graph),
                               const_pointer_cast<const StateList>(concrete_states_by_abstract_state),
                               std::move(concrete_to_abstract_state),
                               abstract_initial_state_index,
                               std::move(abstract_goal_states),
                               std::move(abstract_deadend_states),
                               const_pointer_cast<const GroundActionList>(ground_actions_by_source_and_target),
                               std::move(abstract_goal_distances));
}

std::vector<FaithfulAbstraction>
FaithfulAbstraction::create(const fs::path& domain_filepath, const std::vector<fs::path>& problem_filepaths, const FaithfulAbstractionsOptions& options)
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

    return FaithfulAbstraction::create(memories, options);
}

std::vector<FaithfulAbstraction> FaithfulAbstraction::create(
    const std::vector<std::tuple<Problem, std::shared_ptr<PDDLFactories>, std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>>>&
        memories,
    const FaithfulAbstractionsOptions& options)
{
    auto abstractions_data = std::vector<FaithfulAbstraction> {};
    auto pool = BS::thread_pool(options.num_threads);
    auto futures = std::vector<std::future<std::optional<FaithfulAbstraction>>> {};

    for (const auto& [problem, factories, aag, ssg] : memories)
    {
        futures.push_back(pool.submit_task([problem, factories, aag, ssg, fa_options = options.fa_options]
                                           { return FaithfulAbstraction::create(problem, factories, aag, ssg, fa_options); }));
    }

    for (auto& future : futures)
    {
        auto abstraction_data = future.get();
        if (abstraction_data.has_value())
        {
            abstractions_data.push_back(std::move(abstraction_data.value()));
        }
    }

    if (options.sort_ascending_by_num_states)
    {
        std::sort(abstractions_data.begin(), abstractions_data.end(), [](const auto& l, const auto& r) { return l.get_num_states() < r.get_num_states(); });
    }

    return abstractions_data;
}

/**
 * Abstraction functionality
 */

StateIndex FaithfulAbstraction::get_abstract_state_index(State concrete_state) const
{
    if (m_concrete_to_abstract_state.count(concrete_state))
    {
        return m_concrete_to_abstract_state.at(concrete_state);
    }
    throw std::runtime_error("Failed to access abstract state of concrete state. Did you forget to compute the complete abstraction mapping?");
}

/**
 * Extended functionality
 */

template<IsTraversalDirection Direction>
DistanceList FaithfulAbstraction::compute_shortest_distances_from_states(const StateIndexList& states) const
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

template DistanceList FaithfulAbstraction::compute_shortest_distances_from_states<ForwardTraversal>(const StateIndexList& states) const;
template DistanceList FaithfulAbstraction::compute_shortest_distances_from_states<BackwardTraversal>(const StateIndexList& states) const;

template<IsTraversalDirection Direction>
DistanceMatrix FaithfulAbstraction::compute_pairwise_shortest_state_distances() const
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

template DistanceMatrix FaithfulAbstraction::compute_pairwise_shortest_state_distances<ForwardTraversal>() const;
template DistanceMatrix FaithfulAbstraction::compute_pairwise_shortest_state_distances<BackwardTraversal>() const;

/**
 * Getters
 */

/* Meta data */
Problem FaithfulAbstraction::get_problem() const { return m_problem; }

bool FaithfulAbstraction::get_mark_true_goal_literals() const { return m_mark_true_goal_literals; }

bool FaithfulAbstraction::get_use_unit_cost_one() const { return m_use_unit_cost_one; }

/* Memory */
const std::shared_ptr<PDDLFactories>& FaithfulAbstraction::get_pddl_factories() const { return m_pddl_factories; }

const std::shared_ptr<IApplicableActionGenerator>& FaithfulAbstraction::get_aag() const { return m_aag; }

const std::shared_ptr<StateRepository>& FaithfulAbstraction::get_ssg() const { return m_ssg; }

/* Graph */
const typename FaithfulAbstraction::GraphType& FaithfulAbstraction::get_graph() const { return m_graph; }

/* States */
const FaithfulAbstractStateVertexList& FaithfulAbstraction::get_states() const { return m_graph.get_vertices(); }

template<IsTraversalDirection Direction>
std::ranges::subrange<typename FaithfulAbstraction::AdjacentVertexConstIteratorType<Direction>> FaithfulAbstraction::get_adjacent_states(StateIndex state) const
{
    return m_graph.get_adjacent_vertices<Direction>(state);
}

template std::ranges::subrange<typename FaithfulAbstraction::AdjacentVertexConstIteratorType<ForwardTraversal>>
FaithfulAbstraction::get_adjacent_states<ForwardTraversal>(StateIndex state) const;
template std::ranges::subrange<typename FaithfulAbstraction::AdjacentVertexConstIteratorType<BackwardTraversal>>
FaithfulAbstraction::get_adjacent_states<BackwardTraversal>(StateIndex state) const;

template<IsTraversalDirection Direction>
std::ranges::subrange<typename FaithfulAbstraction::AdjacentVertexIndexConstIteratorType<Direction>>
FaithfulAbstraction::get_adjacent_state_indices(StateIndex state) const
{
    return m_graph.get_adjacent_vertex_indices<Direction>(state);
}

template std::ranges::subrange<typename FaithfulAbstraction::AdjacentVertexIndexConstIteratorType<ForwardTraversal>>
FaithfulAbstraction::get_adjacent_state_indices<ForwardTraversal>(StateIndex state) const;
template std::ranges::subrange<typename FaithfulAbstraction::AdjacentVertexIndexConstIteratorType<BackwardTraversal>>
FaithfulAbstraction::get_adjacent_state_indices<BackwardTraversal>(StateIndex state) const;

const StateMap<StateIndex>& FaithfulAbstraction::get_concrete_to_abstract_state() const { return m_concrete_to_abstract_state; }

StateIndex FaithfulAbstraction::get_initial_state() const { return m_initial_state; }

const StateIndexSet& FaithfulAbstraction::get_goal_states() const { return m_goal_states; }

const StateIndexSet& FaithfulAbstraction::get_deadend_states() const { return m_deadend_states; }

size_t FaithfulAbstraction::get_num_states() const { return m_graph.get_num_vertices(); }

size_t FaithfulAbstraction::get_num_goal_states() const { return get_goal_states().size(); }

size_t FaithfulAbstraction::get_num_deadend_states() const { return get_deadend_states().size(); }

bool FaithfulAbstraction::is_goal_state(StateIndex state) const { return get_goal_states().count(state); }

bool FaithfulAbstraction::is_deadend_state(StateIndex state) const { return get_deadend_states().count(state); }

bool FaithfulAbstraction::is_alive_state(StateIndex state) const { return !(get_goal_states().count(state) || get_deadend_states().count(state)); }

/* Transitions */

const GroundActionsEdgeList& FaithfulAbstraction::get_transitions() const { return m_graph.get_edges(); }

template<IsTraversalDirection Direction>
std::ranges::subrange<typename FaithfulAbstraction::AdjacentEdgeConstIteratorType<Direction>>
FaithfulAbstraction::get_adjacent_transitions(StateIndex state) const
{
    return m_graph.get_adjacent_edges<Direction>(state);
}

template std::ranges::subrange<typename FaithfulAbstraction::AdjacentEdgeConstIteratorType<ForwardTraversal>>
FaithfulAbstraction::get_adjacent_transitions<ForwardTraversal>(StateIndex state) const;
template std::ranges::subrange<typename FaithfulAbstraction::AdjacentEdgeConstIteratorType<BackwardTraversal>>
FaithfulAbstraction::get_adjacent_transitions<BackwardTraversal>(StateIndex state) const;

template<IsTraversalDirection Direction>
std::ranges::subrange<typename FaithfulAbstraction::AdjacentEdgeIndexConstIteratorType<Direction>>
FaithfulAbstraction::get_adjacent_transition_indices(StateIndex state) const
{
    return m_graph.get_adjacent_edge_indices<Direction>(state);
}

template std::ranges::subrange<typename FaithfulAbstraction::AdjacentEdgeIndexConstIteratorType<ForwardTraversal>>
FaithfulAbstraction::get_adjacent_transition_indices<ForwardTraversal>(StateIndex state) const;
template std::ranges::subrange<typename FaithfulAbstraction::AdjacentEdgeIndexConstIteratorType<BackwardTraversal>>
FaithfulAbstraction::get_adjacent_transition_indices<BackwardTraversal>(StateIndex state) const;

TransitionCost FaithfulAbstraction::get_transition_cost(TransitionIndex transition) const
{
    return (m_use_unit_cost_one) ? 1 : get_cost(m_graph.get_edges().at(transition));
}

size_t FaithfulAbstraction::get_num_transitions() const { return m_graph.get_num_edges(); }

/* Distances */
const DistanceList& FaithfulAbstraction::get_goal_distances() const { return m_goal_distances; }

Distance FaithfulAbstraction::get_goal_distance(StateIndex state) const { return m_goal_distances.at(state); }

/* Additional */
const std::map<Distance, StateIndexList>& FaithfulAbstraction::get_states_by_goal_distance() const { return m_states_by_goal_distance; }

/**
 * Pretty printing
 */

std::ostream& operator<<(std::ostream& out, const FaithfulAbstraction& abstraction)
{
    // 2. Header
    out << "digraph {"
        << "\n"
        << "rankdir=\"LR\""
        << "\n";

    // 3. Draw states
    for (size_t state_index = 0; state_index < abstraction.get_num_states(); ++state_index)
    {
        out << "s" << state_index << "[";

        // goal marking
        if (abstraction.is_goal_state(state_index))
        {
            out << "peripheries=2,";
        }

        // label
        out << "label=\"";
        out << "state_index=" << state_index << "\n";
        for (const auto& state : mimir::get_states(abstraction.get_graph().get_vertices().at(state_index)))
        {
            out << std::make_tuple(abstraction.get_problem(), state, std::cref(*abstraction.get_pddl_factories())) << "\n";
        }
        out << "\"";  // end label

        out << "]\n";
    }

    // 4. Draw initial state and dangling edge
    out << "Dangling [ label = \"\", style = invis ]\n"
        << "{ rank = same; Dangling }\n"
        << "Dangling -> s" << abstraction.get_initial_state() << "\n";

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
            out << action << "\n";
        }
        out << "\"";  // end label

        out << "]\n";
    }
    out << "}\n";

    return out;
}
}
