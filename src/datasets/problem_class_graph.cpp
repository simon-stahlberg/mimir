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

namespace mimir::datasets
{
using CertificateToIndex =
    std::unordered_map<nauty_wrapper::Certificate, Index, loki::Hash<nauty_wrapper::Certificate>, loki::EqualTo<nauty_wrapper::Certificate>>;

class ProblemGraphBrFSAlgorithmEventHandler : public BrFSAlgorithmEventHandlerBase<ProblemGraphBrFSAlgorithmEventHandler>
{
private:
    const ProblemOptions& m_options;
    StaticProblemGraph& m_graph;
    CertificateToIndex& m_symmetries;

    /* Implement AlgorithmEventHandlerBase interface */
    friend class BrFSAlgorithmEventHandlerBase<ProblemGraphBrFSAlgorithmEventHandler>;

    void on_expand_state_impl(State state, Problem problem, const PDDLRepositories& pddl_repositories) const {}

    void on_expand_goal_state_impl(State state, Problem problem, const PDDLRepositories& pddl_repositories) const {}

    void on_generate_state_impl(State state, GroundAction action, ContinuousCost action_cost, Problem problem, const PDDLRepositories& pddl_repositories) const
    {
    }

    void on_generate_state_in_search_tree_impl(State state,
                                               GroundAction action,
                                               ContinuousCost action_cost,
                                               Problem problem,
                                               const PDDLRepositories& pddl_repositories) const
    {
    }

    void on_generate_state_not_in_search_tree_impl(State state,
                                                   GroundAction action,
                                                   ContinuousCost action_cost,
                                                   Problem problem,
                                                   const PDDLRepositories& pddl_repositories) const
    {
    }

    void on_finish_g_layer_impl(uint32_t g_value, uint64_t num_expanded_states, uint64_t num_generated_states) const {}

    void on_prune_state_impl(State state, Problem problem, const PDDLRepositories& pddl_repositories) const {}

    bool on_external_pruning_check_impl(State state, Problem problem, const PDDLRepositories& pddl_repositories) const
    {
        // TODO: skip symmetric nodes if symmetry pruning enabled.
        return false;
    }

    void on_start_search_impl(State start_state, Problem problem, const PDDLRepositories& pddl_repositories) const {}

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

    void on_solved_impl(const Plan& plan, const PDDLRepositories& pddl_repositories) const {}

    void on_unsolvable_impl() const {}

    void on_exhausted_impl() const {}

public:
    explicit ProblemGraphBrFSAlgorithmEventHandler(const ProblemOptions& options,
                                                   StaticProblemGraph& graph,
                                                   CertificateToIndex& symmetries,
                                                   bool quiet = true) :
        BrFSAlgorithmEventHandlerBase<ProblemGraphBrFSAlgorithmEventHandler>(quiet),
        m_options(options),
        m_graph(graph),
        m_symmetries(symmetries)
    {
    }
};

static std::optional<std::pair<StaticProblemGraph, CertificateToIndex>> compute_static_problem_graph(const ProblemContext& context,
                                                                                                     const ProblemOptions& options)
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
    auto symmetries = CertificateToIndex {};

    auto goal_test = std::make_shared<ProblemGoal>(problem);
    auto event_handler = std::make_shared<ProblemGraphBrFSAlgorithmEventHandler>(options, graph, symmetries);
    auto result = find_solution_brfs(applicable_action_generator,
                                     state_repository,
                                     state_repository->get_or_create_initial_state(),
                                     event_handler,
                                     goal_test,
                                     std::nullopt,
                                     true);

    if (result.status != EXHAUSTED)
    {
        return std::nullopt;
    }

    return std::make_pair(std::move(graph), std::move(symmetries));
}

static std::vector<std::pair<StaticProblemGraph, CertificateToIndex>> compute_static_problem_graphs(const ProblemContextList& contexts,
                                                                                                    const ProblemOptions& options)
{
    auto problem_graphs = std::vector<std::pair<StaticProblemGraph, CertificateToIndex>> {};
    for (const auto& context : contexts)
    {
        auto result = compute_static_problem_graph(context, options);

        if (!result)
        {
            continue;
        }

        problem_graphs.push_back(std::move(result.value()));
    }

    return problem_graphs;
}

ProblemClassGraph::ProblemClassGraph(const ProblemContextList& contexts, const ClassOptions& options)
{
    // Main idea:
    // 1. iterate over problems, expand then with symmetry pruning
    // 2. Instantiate problem graph followed by its part in the global graph.

    /* Step 1: Compute the `StaticProblemGraphs` and filter according to options.
        Note: we cannot insert certificates immediately as failures would result in unnecessary
    */

    auto static_problem_graphs = compute_static_problem_graphs(contexts, options.options);

    /* Step 2: Compute the `ClassGraph` by looping through the `StaticProblemGraphs`.
        Meanwhile, translate each `StaticProblemGraph` into a `ProblemGraph` that maps to the `ClassVertices` and `ClassEdges`
     */

    auto symmetries = CertificateToIndex {};
}

const ProblemGraphList& ProblemClassGraph::get_problem_graphs() const { return m_problem_graphs; }

const ClassGraph& ProblemClassGraph::get_class_graph() const { return m_class_graph; }

const ProblemGraph& ProblemClassGraph::get_problem_graph(const ClassVertex& vertex) const { return m_problem_graphs.at(get_problem_index(vertex)); }

const ProblemVertex& ProblemClassGraph::get_problem_vertex(const ClassVertex& vertex) const
{
    return m_problem_graphs.at(get_problem_index(vertex)).get_vertex(get_problem_vertex_index(vertex));
}

const ProblemGraph& ProblemClassGraph::get_problem_graph(const ClassEdge& edge) const { return m_problem_graphs.at(get_problem_index(edge)); }

const ProblemEdge& ProblemClassGraph::get_problem_edge(const ClassEdge& edge) const
{
    return m_problem_graphs.at(get_problem_index(edge)).get_edge(get_problem_edge_index(edge));
}

std::ostream& operator<<(std::ostream& out, const ProblemVertex& vertex)
{
    out << "vertex_index=" << vertex.get_index() << "\n"                     //
        << " class_vertex_index=" << get_class_vertex_index(vertex) << "\n"  //
        << " state_index=" << get_state(vertex)->get_index();
    return out;
}

std::ostream& operator<<(std::ostream& out, const ProblemEdge& edge)
{
    out << "edge_index=" << edge.get_index() << "\n"                     //
        << " source_index=" << edge.get_source() << "\n"                 //
        << " target_index=" << edge.get_target() << "\n"                 //
        << " class_vertex_index=" << get_class_edge_index(edge) << "\n"  //
        << " action_index=" << get_action(edge)->get_index();
    return out;
}

std::ostream& operator<<(std::ostream& out, const ClassVertex& vertex)
{
    out << "vertex_index=" << vertex.get_index()           //
        << " problem_index=" << get_problem_index(vertex)  //
        << " problem_vertex_index=" << get_problem_vertex_index(vertex);
    return out;
}

std::ostream& operator<<(std::ostream& out, const ClassEdge& edge)
{
    out << "edge_index=" << edge.get_index() << "\n"             //
        << " source_index=" << edge.get_source() << "\n"         //
        << " target_index=" << edge.get_target() << "\n"         //
        << " problem_index=" << get_problem_index(edge) << "\n"  //
        << " problem_vertex_index=" << get_problem_edge_index(edge);
    return out;
}

}
