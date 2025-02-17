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

#include "mimir/datasets/problem_class_graph.hpp"

#include "mimir/algorithms/nauty.hpp"
#include "mimir/common/timers.hpp"
#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/graphs/static_graph.hpp"
#include "mimir/graphs/static_graph_boost_adapter.hpp"
#include "mimir/search/algorithms/brfs.hpp"
#include "mimir/search/algorithms/brfs/event_handlers/interface.hpp"
#include "mimir/search/algorithms/strategies/goal_strategy.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/heuristics/blind.hpp"
#include "mimir/search/metric.hpp"
#include "mimir/search/openlists/priority_queue.hpp"
#include "mimir/search/search_node.hpp"
#include "mimir/search/search_space.hpp"
#include "mimir/search/state_repository.hpp"

namespace mimir
{
/**
 * ProblemStateSpace
 */

ProblemStateSpace::ProblemStateSpace(ProblemGraph graph,
                                     IndexSet goal_vertices,
                                     IndexSet unsolvable_vertices,
                                     DiscreteCostList unit_goal_distances,
                                     ContinuousCostList action_goal_distances) :
    m_graph(std::move(graph)),
    m_goal_vertices(std::move(goal_vertices)),
    m_unsolvable_vertices(std::move(unsolvable_vertices)),
    m_unit_goal_distances(std::move(unit_goal_distances)),
    m_action_goal_distances(std::move(action_goal_distances))
{
}

const ProblemGraph& ProblemStateSpace::get_graph() const { return m_graph; }
const IndexSet& ProblemStateSpace::get_goal_vertices() const { return m_goal_vertices; }
const IndexSet& ProblemStateSpace::get_unsolvable_vertices() const { return m_unsolvable_vertices; }
const DiscreteCostList& ProblemStateSpace::get_unit_goal_distances() const { return m_unit_goal_distances; }
const ContinuousCostList& ProblemStateSpace::get_action_goal_distances() const { return m_action_goal_distances; }

/**
 * ClassStateSpace
 */

ClassStateSpace::ClassStateSpace(ClassGraph graph, IndexSet goal_vertices, IndexSet unsolvable_vertices) :
    m_graph(std::move(graph)),
    m_goal_vertices(std::move(goal_vertices)),
    m_unsolvable_vertices(std::move(unsolvable_vertices))
{
}

const ClassGraph& ClassStateSpace::get_graph() const { return m_graph; }
const IndexSet& ClassStateSpace::get_goal_vertices() const { return m_goal_vertices; }
const IndexSet& ClassStateSpace::get_unsolvable_vertices() const { return m_unsolvable_vertices; }

/**
 * ProblemClassStateSpace
 */

using CertificateToIndex =
    std::unordered_map<nauty_wrapper::Certificate, Index, loki::Hash<nauty_wrapper::Certificate>, loki::EqualTo<nauty_wrapper::Certificate>>;

class ProblemGraphBrFSAlgorithmEventHandler : public BrFSAlgorithmEventHandlerBase<ProblemGraphBrFSAlgorithmEventHandler>
{
private:
    const ProblemOptions& m_options;
    StaticProblemGraph& m_graph;
    IndexSet& m_goal_vertices;
    CertificateToIndex& m_symmetries;

    StateMap<VertexIndex> m_state_to_vertex_index;

    /* Implement AlgorithmEventHandlerBase interface */
    friend class BrFSAlgorithmEventHandlerBase<ProblemGraphBrFSAlgorithmEventHandler>;

    void on_expand_state_impl(State state, Problem problem, const PDDLRepositories& pddl_repositories)
    {
        if (!m_state_to_vertex_index.contains(state))
            m_state_to_vertex_index.emplace(state, m_graph.add_vertex(VertexIndex(-1), state));
    }

    void on_expand_goal_state_impl(State state, Problem problem, const PDDLRepositories& pddl_repositories)
    {
        m_goal_vertices.insert(m_state_to_vertex_index.at(state));
    }

    void on_generate_state_impl(State state,
                                GroundAction action,
                                ContinuousCost action_cost,
                                State successor_state,
                                Problem problem,
                                const PDDLRepositories& pddl_repositories)
    {
        const auto source_vertex_index = m_state_to_vertex_index.at(state);
        const auto target_vertex_index = m_state_to_vertex_index.contains(successor_state) ? m_state_to_vertex_index.at(successor_state) :
                                                                                             m_graph.add_vertex(VertexIndex(-1), successor_state);
        m_state_to_vertex_index.emplace(successor_state, target_vertex_index);
        m_graph.add_directed_edge(source_vertex_index, target_vertex_index, EdgeIndex(-1), action, action_cost);
    }

    void on_generate_state_in_search_tree_impl(State state,
                                               GroundAction action,
                                               ContinuousCost action_cost,
                                               State successor_state,
                                               Problem problem,
                                               const PDDLRepositories& pddl_repositories)
    {
    }

    void on_generate_state_not_in_search_tree_impl(State state,
                                                   GroundAction action,
                                                   ContinuousCost action_cost,
                                                   State successor_state,
                                                   Problem problem,
                                                   const PDDLRepositories& pddl_repositories)
    {
    }

    void on_finish_g_layer_impl(uint32_t g_value, uint64_t num_expanded_states, uint64_t num_generated_states) {}

    void on_start_search_impl(State start_state, Problem problem, const PDDLRepositories& pddl_repositories) {}

    void on_end_search_impl(uint64_t num_reached_fluent_atoms,
                            uint64_t num_reached_derived_atoms,
                            uint64_t num_bytes_for_unextended_state_portion,
                            uint64_t num_bytes_for_extended_state_portion,
                            uint64_t num_bytes_for_nodes,
                            uint64_t num_bytes_for_actions,
                            uint64_t num_bytes_for_axioms,
                            uint64_t num_states,
                            uint64_t num_nodes,
                            uint64_t num_actions,
                            uint64_t num_axioms) const
    {
    }

    void on_solved_impl(const Plan& plan, const PDDLRepositories& pddl_repositories) {}

    void on_unsolvable_impl() {}

    void on_exhausted_impl() {}

public:
    explicit ProblemGraphBrFSAlgorithmEventHandler(const ProblemOptions& options,
                                                   StaticProblemGraph& graph,
                                                   IndexSet& goal_vertices,
                                                   CertificateToIndex& symmetries,
                                                   bool quiet = true) :
        BrFSAlgorithmEventHandlerBase<ProblemGraphBrFSAlgorithmEventHandler>(quiet),
        m_options(options),
        m_graph(graph),
        m_goal_vertices(goal_vertices),
        m_symmetries(symmetries)
    {
    }
};

static std::optional<std::pair<ProblemStateSpace, CertificateToIndex>> compute_problem_graph(const ProblemContext& context, const ProblemOptions& options)
{
    const auto problem = context.problem;
    const auto applicable_action_generator = context.applicable_action_generator;
    const auto state_repository = context.state_repository;

    assert(problem);
    assert(applicable_action_generator);
    assert(state_repository);

    if (options.remove_if_unsolvable && !problem->static_goal_holds())
    {
        // Unsolvable
        return std::nullopt;
    }

    auto graph = StaticProblemGraph();
    auto goal_vertices = IndexSet {};
    auto symmetries = CertificateToIndex {};

    auto goal_test = std::make_shared<ProblemGoal>(problem);
    auto event_handler = std::make_shared<ProblemGraphBrFSAlgorithmEventHandler>(options, graph, goal_vertices, symmetries, false);
    auto result = find_solution_brfs(applicable_action_generator,
                                     state_repository,
                                     state_repository->get_or_create_initial_state(),
                                     event_handler,
                                     goal_test,
                                     std::nullopt,
                                     true);

    if (result.status != EXHAUSTED)
    {
        return std::nullopt;  ///< ran out of resources.
    }

    if (options.remove_if_unsolvable && goal_vertices.empty())
    {
        return std::nullopt;  ///< initial vertex is unsolvable.
    }

    /* Translate into bidirection graph for making subsequent operations more efficient */
    auto bidir_graph = ProblemGraph(std::move(graph));

    /* Compute unit goal distances. */
    auto [unit_predecessors, unit_goal_distances] =
        breadth_first_search(TraversalDirectionTaggedType(bidir_graph, BackwardTraversal()), goal_vertices.begin(), goal_vertices.end());

    if (options.remove_if_unsolvable && unit_goal_distances.at(0) == UNDEFINED_DISCRETE_COST)  // 0 is the index of the vertex for the initial state.
    {
        return std::nullopt;  ///< initial vertex is unsolvable.
    }

    /* Compute unsolvable vertices. */
    auto unsolvable_vertices = IndexSet {};
    for (VertexIndex v_idx = 0; v_idx < bidir_graph.get_num_vertices(); ++v_idx)
    {
        if (unit_goal_distances[v_idx] == UNDEFINED_DISCRETE_COST)
        {
            unsolvable_vertices.insert(v_idx);
        }
    }

    /* Compute action goal distances. */
    auto edge_action_costs = ContinuousCostList();
    edge_action_costs.reserve(bidir_graph.get_num_edges());
    for (const auto& edge : bidir_graph.get_edges())
    {
        edge_action_costs.push_back(get_action_cost(edge));
    }
    auto [action_predecessors, action_goal_distances] =
        dijkstra_shortest_paths(TraversalDirectionTaggedType(bidir_graph, BackwardTraversal()), edge_action_costs, goal_vertices.begin(), goal_vertices.end());

    return std::make_pair(ProblemStateSpace(std::move(bidir_graph),
                                            std::move(goal_vertices),
                                            std::move(unsolvable_vertices),
                                            std::move(unit_goal_distances),
                                            std::move(action_goal_distances)),
                          std::move(symmetries));
}

static std::vector<std::pair<ProblemStateSpace, CertificateToIndex>> compute_problem_graphs(const ProblemContextList& contexts, const ProblemOptions& options)
{
    auto problem_graphs = std::vector<std::pair<ProblemStateSpace, CertificateToIndex>> {};
    for (const auto& context : contexts)
    {
        auto result = compute_problem_graph(context, options);

        if (!result)
        {
            continue;
        }

        problem_graphs.push_back(std::move(result.value()));
    }

    return problem_graphs;
}

ProblemClassStateSpace::ProblemClassStateSpace(const ProblemContextList& contexts, const ClassOptions& options)
{
    // Main idea:
    // 1. iterate over problems, expand then with symmetry pruning
    // 2. Instantiate problem graph followed by its part in the global graph.

    /* Step 1: Compute the `StaticProblemGraphs` and filter according to options.
        Note: we cannot insert certificates immediately as failures would result in unnecessary
    */

    auto problem_graphs = compute_problem_graphs(contexts, options.options);

    if (options.sort_ascending_by_num_states)
    {
        std::sort(problem_graphs.begin(),
                  problem_graphs.end(),
                  [](auto&& lhs, auto&& rhs) { return lhs.first.get_graph().get_num_vertices() < rhs.first.get_graph().get_num_vertices(); });
    }

    /* Step 2: Compute the `ClassGraph` by looping through the `StaticProblemGraphs`.
        Meanwhile, translate each `StaticProblemGraph` into a `ProblemGraph` that maps to the `ClassVertices` and `ClassEdges`
     */

    auto class_graph = StaticClassGraph {};
    auto class_goal_vertices = IndexSet {};
    auto class_unsolvable_vertices = IndexSet {};

    if (options.symmetry_pruning)
    {
        auto symmetries = CertificateToIndex {};
    }
    else
    {
        /* Simple instantiation without symmetry reduction. */
        auto per_problem_vertex_to_class_vertex = std::vector<IndexList>(problem_graphs.size());

        auto problem_idx = Index(0);

        for (auto& [problem_state_space, problem_symmetries] : problem_graphs)
        {
            assert(problem_symmetries.empty());

            const auto& problem_graph = problem_state_space.get_graph();
            auto& problem_goal_vertices = problem_state_space.get_goal_vertices();
            auto& problem_unsolvable_vertices = problem_state_space.get_unsolvable_vertices();
            auto& problem_unit_goal_distances = problem_state_space.get_unit_goal_distances();
            auto& problem_action_goal_distances = problem_state_space.get_action_goal_distances();

            auto final_problem_graph = StaticProblemGraph();

            auto& problem_v_idx_to_class_v_idx = per_problem_vertex_to_class_vertex[problem_idx];
            problem_v_idx_to_class_v_idx.resize(problem_graph.get_num_vertices(), Index(-1));

            for (const auto& v : problem_graph.get_vertices())
            {
                const auto problem_v_idx = v.get_index();
                const auto class_v_idx = Index(class_graph.get_num_vertices());
                problem_v_idx_to_class_v_idx[problem_v_idx] = class_v_idx;
                const auto unit_goal_distance = problem_unit_goal_distances.at(problem_v_idx);
                const auto action_goal_distance = problem_action_goal_distances.at(problem_v_idx);
                const auto is_goal = (unit_goal_distance == 0);
                const auto is_unsolvable = (unit_goal_distance == UNDEFINED_DISCRETE_COST);

                class_graph.add_vertex(class_v_idx, problem_idx, problem_v_idx, unit_goal_distance, action_goal_distance, is_goal, is_unsolvable);
                final_problem_graph.add_vertex(class_v_idx, get_state(v));

                if (problem_goal_vertices.contains(problem_v_idx))
                {
                    class_goal_vertices.insert(class_v_idx);
                }
                if (problem_unsolvable_vertices.contains(problem_v_idx))
                {
                    class_unsolvable_vertices.insert(class_v_idx);
                }
            }
            for (const auto& e : problem_graph.get_edges())
            {
                const auto problem_e_idx = e.get_index();
                const auto class_e_idx = Index(class_graph.get_num_edges());
                const auto action_cost = get_action_cost(e);

                class_graph.add_directed_edge(problem_v_idx_to_class_v_idx.at(e.get_source()),
                                              problem_v_idx_to_class_v_idx.at(e.get_target()),
                                              class_e_idx,
                                              problem_idx,
                                              problem_e_idx,
                                              action_cost);
                final_problem_graph.add_directed_edge(e.get_source(), e.get_target(), class_e_idx, get_action(e), get_action_cost(e));
            }

            m_problem_state_spaces.push_back(ProblemStateSpace(ProblemGraph(std::move(final_problem_graph)),
                                                               std::move(problem_goal_vertices),
                                                               std::move(problem_unsolvable_vertices),
                                                               std::move(problem_unit_goal_distances),
                                                               std::move(problem_action_goal_distances)));

            ++problem_idx;
        }
    }

    m_class_state_space = ClassStateSpace(ClassGraph(std::move(class_graph)), std::move(class_goal_vertices), std::move(class_unsolvable_vertices));
}

const ProblemStateSpaceList& ProblemClassStateSpace::get_problem_state_spaces() const { return m_problem_state_spaces; }

const ClassStateSpace& ProblemClassStateSpace::get_class_state_space() const { return m_class_state_space; }

const ProblemStateSpace& ProblemClassStateSpace::get_problem_state_space(const ClassVertex& vertex) const
{
    return m_problem_state_spaces.at(get_problem_index(vertex));
}

const ProblemStateSpace& ProblemClassStateSpace::get_problem_state_space(const ClassEdge& edge) const
{
    return m_problem_state_spaces.at(get_problem_index(edge));
}

const ProblemVertex& ProblemClassStateSpace::get_problem_vertex(const ClassVertex& vertex) const
{
    return m_problem_state_spaces.at(get_problem_index(vertex)).get_graph().get_vertex(get_problem_vertex_index(vertex));
}

const ProblemEdge& ProblemClassStateSpace::get_problem_edge(const ClassEdge& edge) const
{
    return m_problem_state_spaces.at(get_problem_index(edge)).get_graph().get_edge(get_problem_edge_index(edge));
}

const ClassVertex& ProblemClassStateSpace::get_class_vertex(const ProblemVertex& vertex) const
{
    return m_class_state_space.get_graph().get_vertex(get_class_vertex_index(vertex));
}

const ClassEdge& ProblemClassStateSpace::get_class_edge(const ProblemEdge& edge) const
{
    return m_class_state_space.get_graph().get_edge(get_class_edge_index(edge));
}

ClassStateSpace ProblemClassStateSpace::create_induced_subspace(const ProblemVertexList& vertices) const {}

ClassStateSpace ProblemClassStateSpace::create_induced_subspace(const IndexList& problem_indices) const
{
    auto class_graph = StaticClassGraph();

    for (const auto p_idx : problem_indices)
    {
        const auto& pss = m_problem_state_spaces.at(p_idx);
    }
}

std::ostream& operator<<(std::ostream& out, const ProblemVertex& vertex)
{
    out << "problem_v_idx=" << vertex.get_index() << "\n"             //
        << " class_v_idx=" << get_class_vertex_index(vertex) << "\n"  //
        << " state_idx=" << get_state(vertex)->get_index();
    return out;
}

std::ostream& operator<<(std::ostream& out, const ProblemEdge& edge)
{
    out << "problem_e_idx=" << edge.get_index() << "\n"           //
        << " problem_src_idx=" << edge.get_source() << "\n"       //
        << " problem_dst_idx=" << edge.get_target() << "\n"       //
        << " class_v_idx=" << get_class_edge_index(edge) << "\n"  //
        << " action_idx=" << get_action(edge)->get_index();
    return out;
}

std::ostream& operator<<(std::ostream& out, const ClassVertex& vertex)
{
    out << "class_v_idx=" << vertex.get_index() << "\n"                      //
        << " class_v_idx=" << get_class_vertex_index(vertex) << "\n"         //
        << " problem_idx=" << get_problem_index(vertex) << "\n"              //
        << " problem_v_idx=" << get_problem_vertex_index(vertex) << "\n"     //
        << " unit_goal_dist=" << get_unit_goal_distance(vertex) << "\n"      //
        << " action_goal_dist=" << get_action_goal_distance(vertex) << "\n"  //
        << " is_goal=" << is_goal(vertex) << "\n"                            //
        << " is_unsolvable=" << is_unsolvable(vertex);
    return out;
}

std::ostream& operator<<(std::ostream& out, const ClassEdge& edge)
{
    out << "class_e_idx=" << edge.get_index() << "\n"                 //
        << " class_src_idx=" << edge.get_source() << "\n"             //
        << " class_dst_idx=" << edge.get_target() << "\n"             //
        << " class_e_idx=" << get_class_edge_index(edge) << "\n"      //
        << " problem_idx=" << get_problem_index(edge) << "\n"         //
        << " problem_e_idx=" << get_problem_edge_index(edge) << "\n"  //
        << " action_cost=" << get_action_cost(edge);
    return out;
}
}
