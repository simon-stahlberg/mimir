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
#include "mimir/search/axiom_evaluators/grounded.hpp"
#include "mimir/search/delete_relaxed_problem_explorator.hpp"

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
                       std::shared_ptr<PDDLRepositories> pddl_repositories,
                       std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                       std::shared_ptr<StateRepository> state_repository,
                       typename StateSpace::GraphType graph,
                       StateMap<Index> state_to_vertex_index,
                       Index initial_vertex_index,
                       IndexSet goal_vertex_indices,
                       IndexSet deadend_vertex_indices,
                       ContinuousCostList goal_distances) :
    m_problem(problem),
    m_use_unit_cost_one(use_unit_cost_one),
    m_pddl_repositories(std::move(pddl_repositories)),
    m_applicable_action_generator(std::move(applicable_action_generator)),
    m_state_repository(std::move(state_repository)),
    m_graph(std::move(graph)),
    m_state_to_vertex_index(std::move(state_to_vertex_index)),
    m_initial_vertex_index(std::move(initial_vertex_index)),
    m_goal_vertex_indices(std::move(goal_vertex_indices)),
    m_deadend_vertex_indices(std::move(deadend_vertex_indices)),
    m_goal_distances(std::move(goal_distances)),
    m_vertex_indices_by_goal_distance()
{
    for (Index vertex = 0; vertex < m_graph.get_num_vertices(); ++vertex)
    {
        m_vertex_indices_by_goal_distance[m_goal_distances.at(vertex)].push_back(vertex);
    }
}

std::optional<StateSpace> StateSpace::create(const fs::path& domain_filepath, const fs::path& problem_filepath, const StateSpaceOptions& options)
{
    auto parser = PDDLParser(domain_filepath, problem_filepath);
    auto delete_relaxed_problem_explorator = DeleteRelaxedProblemExplorator(parser.get_problem(), parser.get_pddl_repositories());
    auto applicable_action_generator = delete_relaxed_problem_explorator.create_grounded_applicable_action_generator();
    auto axiom_evaluator = delete_relaxed_problem_explorator.create_grounded_axiom_evaluator();
    auto state_repository = std::make_shared<StateRepository>(std::dynamic_pointer_cast<IAxiomEvaluator>(axiom_evaluator));
    return StateSpace::create(parser.get_problem(), parser.get_pddl_repositories(), applicable_action_generator, state_repository, options);
}

std::optional<StateSpace> StateSpace::create(Problem problem,
                                             std::shared_ptr<PDDLRepositories> factories,
                                             std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                                             std::shared_ptr<StateRepository> state_repository,
                                             const StateSpaceOptions& options)
{
    auto stop_watch = StopWatch(options.timeout_ms);

    auto initial_state = state_repository->get_or_create_initial_state();

    if (options.remove_if_unsolvable && !problem->static_goal_holds())
    {
        // Unsolvable
        return std::nullopt;
    }

    auto graph = StaticGraph<StateVertex, GroundActionEdge>();
    const auto initial_vertex_index = graph.add_vertex(State(initial_state));
    auto goal_vertex_indices = IndexSet {};
    auto state_to_vertex_index = StateMap<Index> {};
    state_to_vertex_index.emplace(initial_state, initial_vertex_index);

    auto lifo_queue = std::deque<StateVertex>();
    lifo_queue.push_back(graph.get_vertices().at(initial_vertex_index));

    stop_watch.start();
    while (!lifo_queue.empty() && !stop_watch.has_finished())
    {
        const auto vertex = lifo_queue.back();
        const auto vertex_index = vertex.get_index();
        lifo_queue.pop_back();
        if (mimir::get_state(vertex)->literals_hold(problem->get_goal_condition<Fluent>())
            && mimir::get_state(vertex)->literals_hold(problem->get_goal_condition<Derived>()))
        {
            goal_vertex_indices.insert(vertex_index);
        }

        for (const auto& action : applicable_action_generator->create_applicable_action_generator(mimir::get_state(vertex)))
        {
            const auto [successor_state, action_cost] = state_repository->get_or_create_successor_state(mimir::get_state(vertex), action);
            const auto it = state_to_vertex_index.find(successor_state);
            const bool exists = (it != state_to_vertex_index.end());
            if (exists)
            {
                const auto successor_vertex_index = it->second;
                graph.add_directed_edge(vertex_index, successor_vertex_index, action, action_cost);
                continue;
            }

            const auto successor_vertex_index = graph.add_vertex(successor_state);
            if (successor_vertex_index >= options.max_num_states)
            {
                // Ran out of state resources
                return std::nullopt;
            }

            graph.add_directed_edge(vertex_index, successor_vertex_index, action, action_cost);
            state_to_vertex_index.emplace(successor_state, successor_vertex_index);
            lifo_queue.push_back(graph.get_vertices().at(successor_vertex_index));
        }
    }

    if (stop_watch.has_finished())
    {
        // Ran out of time
        return std::nullopt;
    }

    if (options.remove_if_unsolvable && goal_vertex_indices.empty())
    {
        // Skip: unsolvable
        return std::nullopt;
    }

    auto bidirectional_graph = typename StateSpace::GraphType(std::move(graph));

    auto goal_distances = ContinuousCostList {};
    if (options.use_unit_cost_one
        || std::all_of(bidirectional_graph.get_edges().begin(),
                       bidirectional_graph.get_edges().end(),
                       [](const auto& transition) { return get_cost(transition) == 1; }))
    {
        auto [predecessors_, goal_distances_] = breadth_first_search(TraversalDirectionTaggedType(bidirectional_graph, BackwardTraversal()),
                                                                     goal_vertex_indices.begin(),
                                                                     goal_vertex_indices.end());
        goal_distances = std::move(goal_distances_);
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
                                                                        goal_vertex_indices.begin(),
                                                                        goal_vertex_indices.end());
        goal_distances = std::move(goal_distances_);
    }

    auto deadend_vertex_indices = IndexSet {};
    for (Index vertex = 0; vertex < bidirectional_graph.get_num_vertices(); ++vertex)
    {
        if (goal_distances.at(vertex) == std::numeric_limits<ContinuousCost>::infinity())
        {
            deadend_vertex_indices.insert(vertex);
        }
    }

    return StateSpace(problem,
                      options.use_unit_cost_one,
                      std::move(factories),
                      std::move(applicable_action_generator),
                      std::move(state_repository),
                      std::move(bidirectional_graph),
                      std::move(state_to_vertex_index),
                      initial_vertex_index,
                      std::move(goal_vertex_indices),
                      std::move(deadend_vertex_indices),
                      std::move(goal_distances));
}

StateSpaceList StateSpace::create(const fs::path& domain_filepath, const std::vector<fs::path>& problem_filepaths, const StateSpacesOptions& options)
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

    return StateSpace::create(memories, options);
}

std::vector<StateSpace> StateSpace::create(
    const std::vector<std::tuple<Problem, std::shared_ptr<PDDLRepositories>, std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>>>&
        memories,
    const StateSpacesOptions& options)
{
    auto state_spaces = StateSpaceList {};
    auto pool = BS::thread_pool(options.num_threads);
    auto futures = std::vector<std::future<std::optional<StateSpace>>> {};

    for (const auto& [problem, factories, applicable_action_generator, state_repository] : memories)
    {
        futures.push_back(
            pool.submit_task([problem, factories, applicable_action_generator, state_repository, state_space_options = options.state_space_options]
                             { return StateSpace::create(problem, factories, applicable_action_generator, state_repository, state_space_options); }));
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
        std::sort(state_spaces.begin(), state_spaces.end(), [](const auto& l, const auto& r) { return l.get_num_vertices() < r.get_num_vertices(); });
    }

    return state_spaces;
}

/**
 * Extended functionality
 */

template<IsTraversalDirection Direction>
ContinuousCostList StateSpace::compute_shortest_distances_from_vertices(const IndexList& states) const
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

template ContinuousCostList StateSpace::compute_shortest_distances_from_vertices<ForwardTraversal>(const IndexList& states) const;
template ContinuousCostList StateSpace::compute_shortest_distances_from_vertices<BackwardTraversal>(const IndexList& states) const;

template<IsTraversalDirection Direction>
ContinuousCostMatrix StateSpace::compute_pairwise_shortest_vertex_distances() const
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

template ContinuousCostMatrix StateSpace::compute_pairwise_shortest_vertex_distances<ForwardTraversal>() const;
template ContinuousCostMatrix StateSpace::compute_pairwise_shortest_vertex_distances<BackwardTraversal>() const;

/**
 *  Getters
 */

/* Meta data */
Problem StateSpace::get_problem() const { return m_problem; }
bool StateSpace::get_use_unit_cost_one() const { return m_use_unit_cost_one; }

/* Memory */
const std::shared_ptr<PDDLRepositories>& StateSpace::get_pddl_repositories() const { return m_pddl_repositories; }

const std::shared_ptr<IApplicableActionGenerator>& StateSpace::get_applicable_action_generator() const { return m_applicable_action_generator; }

const std::shared_ptr<StateRepository>& StateSpace::get_state_repository() const { return m_state_repository; }

/* Graph */
const typename StateSpace::GraphType& StateSpace::get_graph() const { return m_graph; }

/* States */
const StateVertexList& StateSpace::get_vertices() const { return m_graph.get_vertices(); }

const StateVertex& StateSpace::get_vertex(Index vertex) const { return m_graph.get_vertices().at(vertex); }

template<IsTraversalDirection Direction>
std::ranges::subrange<StateSpace::AdjacentVertexConstIteratorType<Direction>> StateSpace::get_adjacent_vertices(Index vertex) const
{
    return m_graph.get_adjacent_vertices<Direction>(vertex);
}

template std::ranges::subrange<StateSpace::AdjacentVertexConstIteratorType<ForwardTraversal>>
StateSpace::get_adjacent_vertices<ForwardTraversal>(Index vertex) const;
template std::ranges::subrange<StateSpace::AdjacentVertexConstIteratorType<BackwardTraversal>>
StateSpace::get_adjacent_vertices<BackwardTraversal>(Index vertex) const;

template<IsTraversalDirection Direction>
std::ranges::subrange<StateSpace::AdjacentVertexIndexConstIteratorType<Direction>> StateSpace::get_adjacent_vertex_indices(Index vertex) const
{
    return m_graph.get_adjacent_vertex_indices<Direction>(vertex);
}

template std::ranges::subrange<StateSpace::AdjacentVertexIndexConstIteratorType<ForwardTraversal>>
StateSpace::get_adjacent_vertex_indices<ForwardTraversal>(Index vertex) const;
template std::ranges::subrange<StateSpace::AdjacentVertexIndexConstIteratorType<BackwardTraversal>>
StateSpace::get_adjacent_vertex_indices<BackwardTraversal>(Index vertex) const;

Index StateSpace::get_vertex_index(State state) const { return m_state_to_vertex_index.at(state); }

Index StateSpace::get_initial_vertex_index() const { return m_initial_vertex_index; }

const IndexSet& StateSpace::get_goal_vertex_indices() const { return m_goal_vertex_indices; }

const IndexSet& StateSpace::get_deadend_vertex_indices() const { return m_deadend_vertex_indices; }

size_t StateSpace::get_num_vertices() const { return m_graph.get_num_vertices(); }

size_t StateSpace::get_num_goal_vertices() const { return get_goal_vertex_indices().size(); }

size_t StateSpace::get_num_deadend_vertices() const { return get_deadend_vertex_indices().size(); }

bool StateSpace::is_goal_vertex(Index vertex) const { return get_goal_vertex_indices().count(vertex); }

bool StateSpace::is_deadend_vertex(Index vertex) const { return get_deadend_vertex_indices().count(vertex); }

bool StateSpace::is_alive_vertex(Index vertex) const { return !(get_goal_vertex_indices().count(vertex) || get_deadend_vertex_indices().count(vertex)); }

/* Transitions */
const GroundActionEdgeList& StateSpace::get_edges() const { return m_graph.get_edges(); }

const GroundActionEdge& StateSpace::get_edge(Index edge) const { return get_edges().at(edge); }

template<IsTraversalDirection Direction>
std::ranges::subrange<StateSpace::AdjacentEdgeConstIteratorType<Direction>> StateSpace::get_adjacent_edges(Index vertex) const
{
    return m_graph.get_adjacent_edges<Direction>(vertex);
}

template std::ranges::subrange<StateSpace::AdjacentEdgeConstIteratorType<ForwardTraversal>>
StateSpace::get_adjacent_edges<ForwardTraversal>(Index vertex) const;
template std::ranges::subrange<StateSpace::AdjacentEdgeConstIteratorType<BackwardTraversal>>
StateSpace::get_adjacent_edges<BackwardTraversal>(Index vertex) const;

template<IsTraversalDirection Direction>
std::ranges::subrange<StateSpace::AdjacentEdgeIndexConstIteratorType<Direction>> StateSpace::get_adjacent_edge_indices(Index vertex) const
{
    return m_graph.get_adjacent_edge_indices<Direction>(vertex);
}

template std::ranges::subrange<StateSpace::AdjacentEdgeIndexConstIteratorType<ForwardTraversal>>
StateSpace::get_adjacent_edge_indices<ForwardTraversal>(Index vertex) const;
template std::ranges::subrange<StateSpace::AdjacentEdgeIndexConstIteratorType<BackwardTraversal>>
StateSpace::get_adjacent_edge_indices<BackwardTraversal>(Index vertex) const;

ContinuousCost StateSpace::get_edge_cost(Index edge) const { return (m_use_unit_cost_one) ? 1 : get_cost(m_graph.get_edges().at(edge)); }

size_t StateSpace::get_num_edges() const { return m_graph.get_num_edges(); }

/* Distances */
const ContinuousCostList& StateSpace::get_goal_distances() const { return m_goal_distances; }

ContinuousCost StateSpace::get_goal_distance(Index vertex) const { return m_goal_distances.at(vertex); }

ContinuousCost StateSpace::get_max_goal_distance() const { return *std::max_element(m_goal_distances.begin(), m_goal_distances.end()); }

/* Additional */
const std::map<ContinuousCost, IndexList>& StateSpace::get_vertex_indices_by_goal_distance() const { return m_vertex_indices_by_goal_distance; }

Index StateSpace::sample_vertex_index_with_goal_distance(ContinuousCost goal_distance) const
{
    const auto& vertex_indices = m_vertex_indices_by_goal_distance.at(goal_distance);
    const auto vertex_index = std::rand() % static_cast<int>(vertex_indices.size());
    return vertex_indices.at(vertex_index);
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
    for (Index vertex = 0; vertex < state_space.get_num_vertices(); ++vertex)
    {
        out << "s" << vertex << "[";
        if (state_space.is_goal_vertex(vertex))
        {
            out << "peripheries=2,";
        }

        // label
        out << "label=\""
            << std::make_tuple(state_space.get_problem(),
                               mimir::get_state(state_space.get_graph().get_vertices().at(vertex)),
                               std::cref(*state_space.get_pddl_repositories()))
            << "\"";

        out << "]\n";
    }

    // 4. Draw initial state and dangling edge
    out << "Dangling [ label = \"\", style = invis ]\n"
        << "{ rank = same; Dangling }\n"
        << "Dangling -> s" << state_space.get_initial_vertex_index() << "\n";

    // 5. Group states with same distance together
    for (auto it = state_space.get_vertex_indices_by_goal_distance().rbegin(); it != state_space.get_vertex_indices_by_goal_distance().rend(); ++it)
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
        out << "label=\"" << std::make_tuple(get_creating_action(transition), std::cref(*state_space.get_pddl_repositories()), PlanActionFormatterTag {})
            << "\"";

        out << "]\n";
    }
    out << "}\n";

    return out;
}
}
