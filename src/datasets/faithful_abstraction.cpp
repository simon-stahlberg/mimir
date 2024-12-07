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
#include "mimir/search/axiom_evaluators/grounded.hpp"
#include "mimir/search/delete_relaxed_problem_explorator.hpp"

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
                                         std::shared_ptr<PDDLRepositories> factories,
                                         std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                                         std::shared_ptr<StateRepository> state_repository,
                                         typename FaithfulAbstraction::GraphType graph,
                                         std::shared_ptr<const StateList> states_by_abstract_state,
                                         StateMap<Index> state_to_vertex_index,
                                         Index initial_vertex_index,
                                         IndexSet goal_vertex_indices,
                                         IndexSet deadend_vertex_indices,
                                         std::shared_ptr<const GroundActionList> ground_actions_by_source_and_target,
                                         ContinuousCostList goal_distances) :
    m_problem(problem),
    m_mark_true_goal_literals(mark_true_goal_literals),
    m_use_unit_cost_one(use_unit_cost_one),
    m_pddl_repositories(std::move(factories)),
    m_applicable_action_generator(std::move(applicable_action_generator)),
    m_state_repository(std::move(state_repository)),
    m_graph(std::move(graph)),
    m_states_by_abstract_state(std::move(states_by_abstract_state)),
    m_state_to_vertex_index(std::move(state_to_vertex_index)),
    m_initial_vertex_index(initial_vertex_index),
    m_goal_vertex_indices(std::move(goal_vertex_indices)),
    m_deadend_vertex_indices(std::move(deadend_vertex_indices)),
    m_ground_actions_by_source_and_target(std::move(ground_actions_by_source_and_target)),
    m_goal_distances(std::move(goal_distances)),
    m_vertex_indices_by_goal_distance()
{
    /* Ensure correctness. */

    // Check correct state ordering
    for (Index i = 0; i < get_num_vertices(); ++i)
    {
        assert(get_vertices().at(i).get_index() == i && "State index does not match its position in the list");
    }

    // Check correct transition ordering
    for (Index i = 0; i < get_num_edges(); ++i)
    {
        assert(get_edges().at(i).get_index() == i && "Transition index does not match its position in the list");
    }

    /* Additional */
    for (Index vertex = 0; vertex < m_graph.get_num_vertices(); ++vertex)
    {
        m_vertex_indices_by_goal_distance[m_goal_distances.at(vertex)].push_back(vertex);
    }
}

std::optional<FaithfulAbstraction>
FaithfulAbstraction::create(const fs::path& domain_filepath, const fs::path& problem_filepath, const FaithfulAbstractionOptions& options)
{
    auto parser = PDDLParser(domain_filepath, problem_filepath);
    auto delete_relaxed_problem_explorator = DeleteRelaxedProblemExplorator(parser.get_problem(), parser.get_pddl_repositories());
    auto applicable_action_generator = delete_relaxed_problem_explorator.create_grounded_applicable_action_generator();
    auto axiom_evaluator = delete_relaxed_problem_explorator.create_grounded_axiom_evaluator();
    auto state_repository = std::make_shared<StateRepository>(std::dynamic_pointer_cast<IAxiomEvaluator>(axiom_evaluator));
    return FaithfulAbstraction::create(parser.get_problem(), parser.get_pddl_repositories(), applicable_action_generator, state_repository, options);
}

std::optional<FaithfulAbstraction> FaithfulAbstraction::create(Problem problem,
                                                               std::shared_ptr<PDDLRepositories> factories,
                                                               std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                                                               std::shared_ptr<StateRepository> state_repository,
                                                               const FaithfulAbstractionOptions& options)
{
    auto stop_watch = StopWatch(options.timeout_ms);

    auto initial_state = state_repository->get_or_create_initial_state();

    if (options.remove_if_unsolvable && !problem->static_goal_holds())
    {
        // Unsolvable
        return std::nullopt;
    }

    auto concrete_to_abstract_state = StateMap<Index> {};
    auto abstract_states_by_certificate = std::unordered_map<std::shared_ptr<const nauty_wrapper::Certificate>,
                                                             Index,
                                                             nauty_wrapper::UniqueCertificateSharedPtrHash,
                                                             nauty_wrapper::UniqueCertificateSharedPtrEqualTo> {};

    /* Initialize for initial state. */
    const auto color_function = ProblemColorFunction(problem);

    auto object_graph_pruning_strategy = std::unique_ptr<ObjectGraphPruningStrategy> { nullptr };
    if (options.pruning_strategy == ObjectGraphPruningStrategyEnum::StaticScc)
    {
        auto state_space_options = StateSpaceOptions();
        state_space_options.max_num_states = options.max_num_concrete_states;
        state_space_options.timeout_ms = options.timeout_ms;
        auto scc_pruning_strategy =
            ObjectGraphStaticSccPruningStrategy::create(problem, factories, applicable_action_generator, state_repository, state_space_options);
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
                                                  initial_state->get_index(),
                                                  options.mark_true_goal_literals,
                                                  *object_graph_pruning_strategy);

    auto certificate = std::make_shared<const nauty_wrapper::Certificate>(nauty_wrapper::SparseGraph(object_graph).compute_certificate());
    const auto abstract_initial_state_index = 0;
    abstract_states_by_certificate.emplace(std::move(certificate), abstract_initial_state_index);
    concrete_to_abstract_state.emplace(initial_state, abstract_initial_state_index);

    /* Initialize search. */
    auto lifo_queue = std::deque<State>();
    lifo_queue.push_back(initial_state);
    auto transitions = GroundActionEdgeList {};
    auto abstract_goal_states = IndexSet {};
    auto next_abstract_state_index = Index { 1 };
    stop_watch.start();

    while (!lifo_queue.empty() && !stop_watch.has_finished())
    {
        const auto state = lifo_queue.back();
        const auto abstract_state_index = concrete_to_abstract_state.at(state);

        lifo_queue.pop_back();

        if (state->literals_hold(problem->get_goal_condition<Fluent>()) && state->literals_hold(problem->get_goal_condition<Derived>()))
        {
            abstract_goal_states.insert(abstract_state_index);
        }

        for (const auto& action : applicable_action_generator->create_applicable_action_generator(state))
        {
            const auto [successor_state, action_cost] = state_repository->get_or_create_successor_state(state, action);

            // Regenerate concrete state
            const auto concrete_successor_state_exists = concrete_to_abstract_state.count(successor_state);
            if (concrete_successor_state_exists)
            {
                const auto abstract_successor_state_index = concrete_to_abstract_state.at(successor_state);
                transitions.emplace_back(transitions.size(), abstract_state_index, abstract_successor_state_index, action, action_cost);
                continue;
            }

            // Compute certificate of successor state
            const auto object_graph = create_object_graph(color_function,
                                                          *factories,
                                                          problem,
                                                          successor_state,
                                                          successor_state->get_index(),
                                                          options.mark_true_goal_literals,
                                                          *object_graph_pruning_strategy);

            auto certificate = std::make_shared<const nauty_wrapper::Certificate>(nauty_wrapper::SparseGraph(object_graph).compute_certificate());
            const auto it = abstract_states_by_certificate.find(certificate);

            // Regenerate abstract state
            const auto abstract_state_exists = (it != abstract_states_by_certificate.end());

            if (abstract_state_exists)
            {
                const auto abstract_successor_state_index = it->second;
                transitions.emplace_back(transitions.size(),
                                         abstract_state_index,
                                         abstract_successor_state_index,
                                         action,
                                         action_cost);  // TODO: options.compute_complete_abstraction_mapping = True -> must filter duplicate transitions later
                                                        // because it also depends on the source state.

                /* Add concrete state to abstraction mapping. */
                if (options.compute_complete_abstraction_mapping)
                {
                    concrete_to_abstract_state.emplace(successor_state, abstract_successor_state_index);
                    lifo_queue.push_back(successor_state);
                }
            }
            else
            {
                /* Generate new abstract state and add concrete state to abstraction mapping.  */
                const auto abstract_successor_state_index = next_abstract_state_index++;
                abstract_states_by_certificate.emplace(std::move(certificate), abstract_successor_state_index);

                transitions.emplace_back(transitions.size(), abstract_state_index, abstract_successor_state_index, action, action_cost);

                concrete_to_abstract_state.emplace(successor_state, abstract_successor_state_index);
                lifo_queue.push_back(successor_state);
            }

            if (concrete_to_abstract_state.size() >= options.max_num_concrete_states)
            {
                // Ran out of state resources
                return std::nullopt;
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
    auto concrete_states_begin_by_abstract_state = std::vector<Index> {};
    for (Index cur_abstract_state_index = 0; cur_abstract_state_index < num_abstract_states; ++cur_abstract_state_index)
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

    // Ensure that each abstract state and abstract transition contain a single representative.
    assert(options.compute_complete_abstraction_mapping
           || std::all_of(abstract_states.begin(), abstract_states.end(), [](const auto& state) { return get_states(state).size() == 1; }));

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

        // Cost of abstract transition is minimum action_cost among all concrete transitions.
        auto abstract_transition_cost = std::numeric_limits<ContinuousCost>::infinity();
        for (const auto& edge : group)
        {
            abstract_transition_cost = std::min(abstract_transition_cost, get_cost(edge));
        }

        abstract_transitions.emplace_back(abstract_transitions.size(),
                                          group.front().get_source(),
                                          group.front().get_target(),
                                          std::span<const GroundAction>(ground_actions_by_source_and_target->begin() + accumulated_transitions,
                                                                        ground_actions_by_source_and_target->begin() + accumulated_transitions + group.size()),
                                          abstract_transition_cost);
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
        graph.add_directed_edge(abstract_transition.get_source(),
                                abstract_transition.get_target(),
                                get_actions(abstract_transition),
                                get_cost(abstract_transition));
    }
    auto bidirectional_graph = typename FaithfulAbstraction::GraphType(std::move(graph));

    /* Compute abstract goal distances */

    auto abstract_goal_distances = ContinuousCostList {};
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
        auto transition_costs = ContinuousCostList {};
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
    auto abstract_deadend_states = IndexSet {};
    for (Index abstract_state_index = 0; abstract_state_index < num_abstract_states; ++abstract_state_index)
    {
        if (abstract_goal_distances.at(abstract_state_index) == std::numeric_limits<ContinuousCost>::infinity())
        {
            abstract_deadend_states.insert(abstract_state_index);
        }
    }

    return FaithfulAbstraction(problem,
                               options.mark_true_goal_literals,
                               options.use_unit_cost_one,
                               std::move(factories),
                               std::move(applicable_action_generator),
                               std::move(state_repository),
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
        std::vector<std::tuple<Problem, std::shared_ptr<PDDLRepositories>, std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>>> {};
    for (const auto& problem_filepath : problem_filepaths)
    {
        auto parser = PDDLParser(domain_filepath, problem_filepath);
        auto delete_relaxed_problem_explorator = DeleteRelaxedProblemExplorator(parser.get_problem(), parser.get_pddl_repositories());
        auto applicable_action_generator = delete_relaxed_problem_explorator.create_grounded_applicable_action_generator();
        auto axiom_evaluator = delete_relaxed_problem_explorator.create_grounded_axiom_evaluator();
        auto state_repository = std::make_shared<StateRepository>(std::dynamic_pointer_cast<IAxiomEvaluator>(axiom_evaluator));
        memories.emplace_back(parser.get_problem(), parser.get_pddl_repositories(), applicable_action_generator, state_repository);
    }

    return FaithfulAbstraction::create(memories, options);
}

std::vector<FaithfulAbstraction> FaithfulAbstraction::create(
    const std::vector<std::tuple<Problem, std::shared_ptr<PDDLRepositories>, std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>>>&
        memories,
    const FaithfulAbstractionsOptions& options)
{
    auto abstractions_data = std::vector<FaithfulAbstraction> {};
    auto pool = BS::thread_pool(options.num_threads);
    auto futures = std::vector<std::future<std::optional<FaithfulAbstraction>>> {};

    for (const auto& [problem, factories, applicable_action_generator, state_repository] : memories)
    {
        futures.push_back(
            pool.submit_task([problem, factories, applicable_action_generator, state_repository, fa_options = options.fa_options]
                             { return FaithfulAbstraction::create(problem, factories, applicable_action_generator, state_repository, fa_options); }));
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
        std::sort(abstractions_data.begin(), abstractions_data.end(), [](const auto& l, const auto& r) { return l.get_num_vertices() < r.get_num_vertices(); });
    }

    return abstractions_data;
}

/**
 * Abstraction functionality
 */

Index FaithfulAbstraction::get_vertex_index(State state) const
{
    if (m_state_to_vertex_index.count(state))
    {
        return m_state_to_vertex_index.at(state);
    }
    throw std::runtime_error("Failed to access vertex of state. Did you forget to compute the complete abstraction mapping?");
}

/**
 * Extended functionality
 */

template<IsTraversalDirection Direction>
ContinuousCostList FaithfulAbstraction::compute_shortest_distances_from_vertices(const IndexList& states) const
{
    auto distances = ContinuousCostList {};
    if (m_use_unit_cost_one
        || std::all_of(m_graph.get_edges().begin(), m_graph.get_edges().end(), [](const auto& transition) { return get_cost(transition) == 1; }))
    {
        auto [predecessors_, distances_] = breadth_first_search(TraversalDirectionTaggedType(m_graph, Direction()), states.begin(), states.end());
        distances = std::move(distances_);
    }
    else
    {
        auto transition_costs = ContinuousCostList {};
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

template ContinuousCostList FaithfulAbstraction::compute_shortest_distances_from_vertices<ForwardTraversal>(const IndexList& states) const;
template ContinuousCostList FaithfulAbstraction::compute_shortest_distances_from_vertices<BackwardTraversal>(const IndexList& states) const;

template<IsTraversalDirection Direction>
ContinuousCostMatrix FaithfulAbstraction::compute_pairwise_shortest_vertex_distances() const
{
    auto transition_costs = ContinuousCostList {};
    if (m_use_unit_cost_one)
    {
        transition_costs = ContinuousCostList(m_graph.get_num_edges(), 1);
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

template ContinuousCostMatrix FaithfulAbstraction::compute_pairwise_shortest_vertex_distances<ForwardTraversal>() const;
template ContinuousCostMatrix FaithfulAbstraction::compute_pairwise_shortest_vertex_distances<BackwardTraversal>() const;

/**
 * Getters
 */

/* Meta data */
Problem FaithfulAbstraction::get_problem() const { return m_problem; }

bool FaithfulAbstraction::get_mark_true_goal_literals() const { return m_mark_true_goal_literals; }

bool FaithfulAbstraction::get_use_unit_cost_one() const { return m_use_unit_cost_one; }

/* Memory */
const std::shared_ptr<PDDLRepositories>& FaithfulAbstraction::get_pddl_repositories() const { return m_pddl_repositories; }

const std::shared_ptr<IApplicableActionGenerator>& FaithfulAbstraction::get_applicable_action_generator() const { return m_applicable_action_generator; }

const std::shared_ptr<StateRepository>& FaithfulAbstraction::get_state_repository() const { return m_state_repository; }

/* Graph */
const typename FaithfulAbstraction::GraphType& FaithfulAbstraction::get_graph() const { return m_graph; }

/* States */
const FaithfulAbstractStateVertexList& FaithfulAbstraction::get_vertices() const { return m_graph.get_vertices(); }

template<IsTraversalDirection Direction>
std::ranges::subrange<typename FaithfulAbstraction::AdjacentVertexConstIteratorType<Direction>> FaithfulAbstraction::get_adjacent_vertices(Index vertex) const
{
    return m_graph.get_adjacent_vertices<Direction>(vertex);
}

template std::ranges::subrange<typename FaithfulAbstraction::AdjacentVertexConstIteratorType<ForwardTraversal>>
FaithfulAbstraction::get_adjacent_vertices<ForwardTraversal>(Index vertex) const;
template std::ranges::subrange<typename FaithfulAbstraction::AdjacentVertexConstIteratorType<BackwardTraversal>>
FaithfulAbstraction::get_adjacent_vertices<BackwardTraversal>(Index vertex) const;

template<IsTraversalDirection Direction>
std::ranges::subrange<typename FaithfulAbstraction::AdjacentVertexIndexConstIteratorType<Direction>>
FaithfulAbstraction::get_adjacent_vertex_indices(Index vertex) const
{
    return m_graph.get_adjacent_vertex_indices<Direction>(vertex);
}

template std::ranges::subrange<typename FaithfulAbstraction::AdjacentVertexIndexConstIteratorType<ForwardTraversal>>
FaithfulAbstraction::get_adjacent_vertex_indices<ForwardTraversal>(Index vertex) const;
template std::ranges::subrange<typename FaithfulAbstraction::AdjacentVertexIndexConstIteratorType<BackwardTraversal>>
FaithfulAbstraction::get_adjacent_vertex_indices<BackwardTraversal>(Index vertex) const;

const StateMap<Index>& FaithfulAbstraction::get_state_to_vertex_index() const { return m_state_to_vertex_index; }

Index FaithfulAbstraction::get_initial_vertex_index() const { return m_initial_vertex_index; }

const IndexSet& FaithfulAbstraction::get_goal_vertex_indices() const { return m_goal_vertex_indices; }

const IndexSet& FaithfulAbstraction::get_deadend_vertex_indices() const { return m_deadend_vertex_indices; }

size_t FaithfulAbstraction::get_num_vertices() const { return m_graph.get_num_vertices(); }

size_t FaithfulAbstraction::get_num_goal_vertices() const { return get_goal_vertex_indices().size(); }

size_t FaithfulAbstraction::get_num_deadend_vertices() const { return get_deadend_vertex_indices().size(); }

bool FaithfulAbstraction::is_goal_vertex(Index vertex) const { return get_goal_vertex_indices().count(vertex); }

bool FaithfulAbstraction::is_deadend_vertex(Index vertex) const { return get_deadend_vertex_indices().count(vertex); }

bool FaithfulAbstraction::is_alive_vertex(Index vertex) const
{
    return !(get_goal_vertex_indices().count(vertex) || get_deadend_vertex_indices().count(vertex));
}

/* Transitions */

const GroundActionsEdgeList& FaithfulAbstraction::get_edges() const { return m_graph.get_edges(); }

const GroundActionsEdge& FaithfulAbstraction::get_edge(Index edge) const { return get_edges().at(edge); }

template<IsTraversalDirection Direction>
std::ranges::subrange<typename FaithfulAbstraction::AdjacentEdgeConstIteratorType<Direction>> FaithfulAbstraction::get_adjacent_edges(Index vertex) const
{
    return m_graph.get_adjacent_edges<Direction>(vertex);
}

template std::ranges::subrange<typename FaithfulAbstraction::AdjacentEdgeConstIteratorType<ForwardTraversal>>
FaithfulAbstraction::get_adjacent_edges<ForwardTraversal>(Index vertex) const;
template std::ranges::subrange<typename FaithfulAbstraction::AdjacentEdgeConstIteratorType<BackwardTraversal>>
FaithfulAbstraction::get_adjacent_edges<BackwardTraversal>(Index vertex) const;

template<IsTraversalDirection Direction>
std::ranges::subrange<typename FaithfulAbstraction::AdjacentEdgeIndexConstIteratorType<Direction>>
FaithfulAbstraction::get_adjacent_edge_indices(Index vertex) const
{
    return m_graph.get_adjacent_edge_indices<Direction>(vertex);
}

template std::ranges::subrange<typename FaithfulAbstraction::AdjacentEdgeIndexConstIteratorType<ForwardTraversal>>
FaithfulAbstraction::get_adjacent_edge_indices<ForwardTraversal>(Index vertex) const;
template std::ranges::subrange<typename FaithfulAbstraction::AdjacentEdgeIndexConstIteratorType<BackwardTraversal>>
FaithfulAbstraction::get_adjacent_edge_indices<BackwardTraversal>(Index vertex) const;

ContinuousCost FaithfulAbstraction::get_edge_cost(Index edge) const { return (m_use_unit_cost_one) ? 1 : get_cost(m_graph.get_edges().at(edge)); }

size_t FaithfulAbstraction::get_num_edges() const { return m_graph.get_num_edges(); }

/* Distances */
const ContinuousCostList& FaithfulAbstraction::get_goal_distances() const { return m_goal_distances; }

ContinuousCost FaithfulAbstraction::get_goal_distance(Index vertex) const { return m_goal_distances.at(vertex); }

/* Additional */
const std::map<ContinuousCost, IndexList>& FaithfulAbstraction::get_vertex_indices_by_goal_distance() const { return m_vertex_indices_by_goal_distance; }

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
    for (size_t vertex = 0; vertex < abstraction.get_num_vertices(); ++vertex)
    {
        out << "s" << vertex << "[";

        // goal marking
        if (abstraction.is_goal_vertex(vertex))
        {
            out << "peripheries=2,";
        }

        // label
        out << "label=\"";
        out << "vertex_index=" << vertex << "\n";
        for (const auto& state : mimir::get_states(abstraction.get_graph().get_vertices().at(vertex)))
        {
            out << std::make_tuple(abstraction.get_problem(), state, std::cref(*abstraction.get_pddl_repositories())) << "\n";
        }
        out << "\"";  // end label

        out << "]\n";
    }

    // 4. Draw initial state and dangling edge
    out << "Dangling [ label = \"\", style = invis ]\n"
        << "{ rank = same; Dangling }\n"
        << "Dangling -> s" << abstraction.get_initial_vertex_index() << "\n";

    // 5. Group states with same distance together
    for (auto it = abstraction.get_vertex_indices_by_goal_distance().rbegin(); it != abstraction.get_vertex_indices_by_goal_distance().rend(); ++it)
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
            out << std::make_tuple(action, std::cref(*abstraction.get_pddl_repositories()), PlanActionFormatterTag {}) << "\n";
        }
        out << "\"";  // end label

        out << "]\n";
    }
    out << "}\n";

    return out;
}
}
