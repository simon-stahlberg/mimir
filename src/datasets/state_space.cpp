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
#include "mimir/datasets/object_graph.hpp"
#include "mimir/formalism/generalized_problem.hpp"
#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/parser.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/graphs/algorithms/nauty.hpp"
#include "mimir/graphs/bgl/graph_algorithms.hpp"
#include "mimir/graphs/bgl/static_graph_algorithms.hpp"
#include "mimir/graphs/static_graph.hpp"
#include "mimir/search/algorithms/brfs.hpp"
#include "mimir/search/algorithms/brfs/event_handlers/interface.hpp"
#include "mimir/search/algorithms/strategies/goal_strategy.hpp"
#include "mimir/search/algorithms/strategies/pruning_strategy.hpp"
#include "mimir/search/applicability.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/axiom_evaluators.hpp"
#include "mimir/search/delete_relaxed_problem_explorator.hpp"
#include "mimir/search/generalized_search_context.hpp"
#include "mimir/search/heuristics/blind.hpp"
#include "mimir/search/openlists/priority_queue.hpp"
#include "mimir/search/search_context.hpp"
#include "mimir/search/search_node.hpp"
#include "mimir/search/search_space.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/state_repository.hpp"

#include <deque>

using namespace mimir::formalism;
using namespace mimir::search;
using namespace mimir::graphs;

namespace mimir::graphs
{

std::ostream& operator<<(std::ostream& out, const ProblemVertex& vertex)
{
    out << "problem_v_idx=" << vertex.get_index() << "\n"  //
        << " state=";
    mimir::operator<<(out, std::make_tuple(get_state(vertex), std::cref(*get_problem(vertex))));
    out << "\n"
        << " unit_goal_dist=" << get_unit_goal_distance(vertex) << "\n"      //
        << " action_goal_dist=" << get_action_goal_distance(vertex) << "\n"  //
        << " is_initial=" << is_initial(vertex) << "\n"                      //
        << " is_goal=" << is_goal(vertex) << "\n"                            //
        << " is_unsolvable=" << is_unsolvable(vertex) << "\n"                //
        << " is_alive=" << is_alive(vertex);
    return out;
}

std::ostream& operator<<(std::ostream& out, const ProblemEdge& edge)
{
    out << "problem_e_idx=" << edge.get_index() << "\n"       //
        << " problem_src_idx=" << edge.get_source() << "\n"   //
        << " problem_dst_idx=" << edge.get_target() << "\n";  //
    out << " action=";
    mimir::operator<<(out, std::make_tuple(get_action(edge), std::cref(*get_problem(edge)), GroundActionImpl::PlanFormatterTag {}));
    out << "\n"
        << " action_cost=" << get_action_cost(edge);
    return out;
}

}

namespace mimir::datasets
{
/**
 * StateSpaceImpl
 */

struct SymmetriesData
{
    CertificateMaps certificate_maps;
    StateSet prunable_states;
    UnorderedSet<std::pair<graphs::VertexIndex, graphs::VertexIndex>> m_edges;

    SymmetriesData() : certificate_maps(), prunable_states() {}
};

/// @brief `SymmetryStatePruning` extends the brfs pruning strategy by additionally pruning symmetric states.
class SymmetryStatePruning : public IPruningStrategy
{
private:
    SymmetriesData& m_symm_data;

public:
    explicit SymmetryStatePruning(SymmetriesData& symm_data) : m_symm_data(symm_data) {}

    bool test_prune_initial_state(State state) override { return false; }
    bool test_prune_successor_state(State state, State succ_state, bool is_new_succ) override
    {
        return !is_new_succ || m_symm_data.prunable_states.contains(succ_state);
    }
};

class SymmetryReducedProblemGraphEventHandler : public brfs::EventHandlerBase<SymmetryReducedProblemGraphEventHandler>
{
private:
    const StateSpaceImpl::Options& m_options;
    graphs::StaticProblemGraph& m_graph;
    IndexSet& m_goal_vertices;
    SymmetriesData& m_symm_data;

    StateMap<graphs::VertexIndex> m_state_to_vertex_index;

    /* Implement AlgorithmEventHandlerBase interface */
    friend class brfs::EventHandlerBase<SymmetryReducedProblemGraphEventHandler>;

    auto compute_canonical_graph(State state) { return nauty::SparseGraph(create_object_graph(state, *m_problem)).canonize(); }

    void on_expand_state_impl(State state) {}

    void on_expand_goal_state_impl(State state) { m_goal_vertices.insert(m_state_to_vertex_index.at(state)); }

    void on_generate_state_impl(State state, GroundAction action, ContinuousCost action_cost, State successor_state)
    {
        const auto source_v_idx = m_state_to_vertex_index.at(state);

        auto certificate = compute_canonical_graph(successor_state);
        auto it = m_symm_data.certificate_maps.cert_to_v_idx.find(certificate);
        const auto is_symmetric = (it != m_symm_data.certificate_maps.cert_to_v_idx.end());

        if (is_symmetric)
        {
            /* Existing class re-encountered: add edge if between existing representative states! */

            /* Always add novel edges between symmetric states. */

            const auto target_v_idx = it->second;
            if (m_symm_data.m_edges.emplace(source_v_idx, target_v_idx).second)  ///< avoid adding parallel edges
            {
                m_graph.add_directed_edge(source_v_idx, target_v_idx, action, m_problem, action_cost);
            }

            /* Always mark symmetric states as prunable. */
            m_symm_data.prunable_states.insert(successor_state);
        }
        else
        {
            /* New class determined: add vertex and edge! */

            assert(m_graph.get_num_vertices() == m_state_to_vertex_index.size());

            const auto target_v_idx = m_graph.add_vertex(successor_state, m_problem, DiscreteCost(0), ContinuousCost(0), false, false, false, false);

            m_symm_data.certificate_maps.state_to_cert.emplace(successor_state, certificate);
            m_symm_data.certificate_maps.cert_to_v_idx.emplace(certificate, target_v_idx);

            m_state_to_vertex_index.emplace(successor_state, target_v_idx);
            m_graph.add_directed_edge(source_v_idx, target_v_idx, action, m_problem, action_cost);
        }
    }

    void on_generate_state_in_search_tree_impl(State state, GroundAction action, ContinuousCost action_cost, State successor_state) {}

    void on_generate_state_not_in_search_tree_impl(State state, GroundAction action, ContinuousCost action_cost, State successor_state) {}

    void on_finish_g_layer_impl(uint32_t g_value, uint64_t num_expanded_states, uint64_t num_generated_states) {}

    void on_start_search_impl(State start_state)
    {
        const auto v_idx = m_graph.add_vertex(start_state, m_problem, DiscreteCost(0), ContinuousCost(0), false, false, false, false);
        m_state_to_vertex_index.emplace(start_state, v_idx);

        const auto certificate = compute_canonical_graph(start_state);
        m_symm_data.certificate_maps.state_to_cert.emplace(start_state, certificate);
        m_symm_data.certificate_maps.cert_to_v_idx.emplace(certificate, v_idx);
    }

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

    void on_solved_impl(const Plan& plan) {}

    void on_unsolvable_impl() {}

    void on_exhausted_impl() {}

public:
    explicit SymmetryReducedProblemGraphEventHandler(Problem problem,
                                                     const StateSpaceImpl::Options& options,
                                                     graphs::StaticProblemGraph& graph,
                                                     IndexSet& goal_vertices,
                                                     SymmetriesData& symm_data,
                                                     bool quiet = true) :
        brfs::EventHandlerBase<SymmetryReducedProblemGraphEventHandler>(problem, quiet),
        m_options(options),
        m_graph(graph),
        m_goal_vertices(goal_vertices),
        m_symm_data(symm_data)
    {
    }
};

class ProblemGraphEventHandler : public brfs::EventHandlerBase<ProblemGraphEventHandler>
{
private:
    const StateSpaceImpl::Options& m_options;
    graphs::StaticProblemGraph& m_graph;
    IndexSet& m_goal_vertices;

    StateMap<graphs::VertexIndex> m_state_to_vertex_index;

    /* Implement AlgorithmEventHandlerBase interface */
    friend class EventHandlerBase<ProblemGraphEventHandler>;

    void on_expand_state_impl(State state)
    {
        // if (!m_state_to_vertex_index.contains(state))
        //     m_state_to_vertex_index.emplace(state, m_graph.add_vertex(graphs::VertexIndex(-1), state, nullptr));
    }

    void on_expand_goal_state_impl(State state) { m_goal_vertices.insert(m_state_to_vertex_index.at(state)); }

    void on_generate_state_impl(State state, GroundAction action, ContinuousCost action_cost, State successor_state)
    {
        const auto source_vertex_index = m_state_to_vertex_index.at(state);
        const auto target_vertex_index = m_state_to_vertex_index.contains(successor_state) ?
                                             m_state_to_vertex_index.at(successor_state) :
                                             m_graph.add_vertex(successor_state, m_problem, DiscreteCost(0), ContinuousCost(0), false, false, false, false);
        m_state_to_vertex_index.emplace(successor_state, target_vertex_index);
        m_graph.add_directed_edge(source_vertex_index, target_vertex_index, action, m_problem, action_cost);
    }

    void on_generate_state_in_search_tree_impl(State state, GroundAction action, ContinuousCost action_cost, State successor_state) {}

    void on_generate_state_not_in_search_tree_impl(State state, GroundAction action, ContinuousCost action_cost, State successor_state) {}

    void on_finish_g_layer_impl(uint32_t g_value, uint64_t num_expanded_states, uint64_t num_generated_states) {}

    void on_start_search_impl(State start_state)
    {
        const auto v_idx = m_graph.add_vertex(start_state, m_problem, DiscreteCost(0), ContinuousCost(0), false, false, false, false);
        m_state_to_vertex_index.emplace(start_state, v_idx);
    }

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

    void on_solved_impl(const Plan& plan) {}

    void on_unsolvable_impl() {}

    void on_exhausted_impl() {}

public:
    explicit ProblemGraphEventHandler(Problem problem,
                                      const StateSpaceImpl::Options& options,
                                      graphs::StaticProblemGraph& graph,
                                      IndexSet& goal_vertices,
                                      bool quiet = true) :
        brfs::EventHandlerBase<ProblemGraphEventHandler>(problem, quiet),
        m_options(options),
        m_graph(graph),
        m_goal_vertices(goal_vertices)
    {
    }
};

static std::optional<StateSpace>
perform_reachability_analysis(SearchContext context, graphs::StaticProblemGraph graph, IndexSet goal_vertices, const StateSpaceImpl::Options& options)
{
    if (options.remove_if_unsolvable && goal_vertices.empty())
    {
        return std::nullopt;  ///< initial vertex is unsolvable.
    }

    /* Translate into bidirection graph for making subsequent operations more efficient */
    auto bidir_graph = graphs::ProblemGraph(std::move(graph));

    /* Compute unit goal distances. */
    auto [unit_predecessors, unit_goal_distances] =
        graphs::bgl::breadth_first_search(graphs::DirectionTaggedType(bidir_graph, graphs::BackwardTag {}), goal_vertices.begin(), goal_vertices.end());

    if (options.remove_if_unsolvable && unit_goal_distances.at(0) == UNDEFINED_DISCRETE_COST)  // 0 is the index of the vertex for the initial state.
    {
        return std::nullopt;  ///< initial vertex is unsolvable.
    }

    /* Compute unsolvable vertices. */
    auto unsolvable_vertices = IndexSet {};
    for (graphs::VertexIndex v_idx = 0; v_idx < bidir_graph.get_num_vertices(); ++v_idx)
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
    auto [action_predecessors, action_goal_distances] = graphs::bgl::dijkstra_shortest_paths(graphs::DirectionTaggedType(bidir_graph, graphs::BackwardTag {}),
                                                                                             edge_action_costs,
                                                                                             goal_vertices.begin(),
                                                                                             goal_vertices.end());

    /* Construct final graph */
    auto final_graph = graphs::StaticProblemGraph {};

    for (const auto& v : bidir_graph.get_vertices())
    {
        const auto problem_v_idx = v.get_index();
        const auto unit_goal_distance = unit_goal_distances.at(problem_v_idx);
        const auto action_goal_distance = action_goal_distances.at(problem_v_idx);
        const auto is_initial = (problem_v_idx == 0);
        const auto is_goal = goal_vertices.contains(problem_v_idx);
        const auto is_unsolvable = unsolvable_vertices.contains(problem_v_idx);
        const auto is_alive = (!(is_goal || is_unsolvable));

        final_graph
            .add_vertex(graphs::get_state(v), graphs::get_problem(v), unit_goal_distance, action_goal_distance, is_initial, is_goal, is_unsolvable, is_alive);
    }
    for (const auto& e : bidir_graph.get_edges())
    {
        final_graph.add_directed_edge(e.get_source(), e.get_target(), e);
    }

    return std::make_shared<StateSpaceImpl>(options.symmetry_pruning,
                                            context,
                                            graphs::ProblemGraph(std::move(final_graph)),
                                            0,
                                            std::move(goal_vertices),
                                            std::move(unsolvable_vertices));
}

static std::optional<std::pair<StateSpace, CertificateMaps>> compute_problem_graph_with_symmetry_reduction(const SearchContext& context,
                                                                                                           const StateSpaceImpl::Options& options)
{
    auto graph = graphs::StaticProblemGraph();
    auto goal_vertices = IndexSet {};

    auto symm_data = SymmetriesData();

    const auto event_handler =
        std::make_shared<SymmetryReducedProblemGraphEventHandler>(context->get_problem(), options, graph, goal_vertices, symm_data, false);
    const auto pruning_strategy = std::make_shared<SymmetryStatePruning>(symm_data);
    const auto state_repository = context->get_state_repository();
    const auto goal_test = ProblemGoalStrategyImpl::create(context->get_problem());
    const auto [initial_state, initial_g_value] = state_repository->get_or_create_initial_state();
    const auto result = find_solution(context, initial_state, event_handler, goal_test, pruning_strategy, true);

    if (result.status != EXHAUSTED)
    {
        return std::nullopt;  ///< ran out of resources.
    }

    auto state_space = perform_reachability_analysis(context, std::move(graph), std::move(goal_vertices), options);

    if (!state_space)
    {
        return std::nullopt;
    }

    return std::make_pair(state_space.value(), std::move(symm_data.certificate_maps));
}

static std::optional<StateSpace> compute_problem_graph_without_symmetry_reduction(const SearchContext& context, const StateSpaceImpl::Options& options)
{
    auto graph = graphs::StaticProblemGraph();
    auto goal_vertices = IndexSet {};

    const auto state_repository = context->get_state_repository();
    const auto goal_test = ProblemGoalStrategyImpl::create(context->get_problem());
    const auto event_handler = std::make_shared<ProblemGraphEventHandler>(context->get_problem(), options, graph, goal_vertices, false);
    const auto pruning_strategy = DuplicatePruningStrategyImpl::create();
    const auto [initial_state, initial_g_value] = state_repository->get_or_create_initial_state();
    const auto result = find_solution(context, initial_state, event_handler, goal_test, pruning_strategy, true);

    if (result.status != EXHAUSTED)
    {
        return std::nullopt;  ///< ran out of resources.
    }

    return perform_reachability_analysis(context, std::move(graph), std::move(goal_vertices), options);
}

StateSpaceImpl::StateSpaceImpl(bool is_symmetry_reduced,
                               search::SearchContext context,
                               graphs::ProblemGraph graph,
                               Index initial_vertex,
                               IndexSet goal_vertices,
                               IndexSet unsolvable_vertices) :
    m_is_symmetry_reduced(is_symmetry_reduced),
    m_context(std::move(context)),
    m_graph(std::move(graph)),
    m_initial_vertex(initial_vertex),
    m_goal_vertices(std::move(goal_vertices)),
    m_unsolvable_vertices(std::move(unsolvable_vertices))
{
}

std::optional<std::pair<StateSpace, std::optional<CertificateMaps>>> StateSpaceImpl::create(search::SearchContext context, const Options& options)
{
    if (options.symmetry_pruning)
    {
        if (auto result = compute_problem_graph_with_symmetry_reduction(context, options))
        {
            return std::make_optional(std::make_pair(result->first, std::make_optional(result->second)));
        }
        return std::nullopt;
    }
    else
    {
        if (auto result = compute_problem_graph_without_symmetry_reduction(context, options))
        {
            return std::make_optional(std::make_pair(result.value(), std::nullopt));
        }
        return std::nullopt;
    }
}

std::vector<std::pair<StateSpace, std::optional<CertificateMaps>>> StateSpaceImpl::create(search::GeneralizedSearchContext contexts, const Options& options)
{
    auto state_spaces = std::vector<std::pair<StateSpace, std::optional<CertificateMaps>>> {};

    for (const auto& context : contexts->get_search_contexts())
    {
        if (options.remove_if_unsolvable && !context->get_problem()->static_goal_holds())
        {
            continue;
        }

        auto result = create(context, options);

        if (!result)
        {
            continue;
        }

        state_spaces.emplace_back(std::move(result.value()));
    }

    if (options.sort_ascending_by_num_states)
    {
        std::sort(state_spaces.begin(),
                  state_spaces.end(),
                  [](auto&& lhs, auto&& rhs) { return lhs.first->get_graph().get_num_vertices() < rhs.first->get_graph().get_num_vertices(); });
    }

    return state_spaces;
}

bool StateSpaceImpl::is_symmetry_reduced() const { return m_is_symmetry_reduced; }

const search::SearchContext& StateSpaceImpl::get_search_context() const { return m_context; }

const graphs::ProblemGraph& StateSpaceImpl::get_graph() const { return m_graph; }

Index StateSpaceImpl::get_initial_vertex() const { return m_initial_vertex; }

const IndexSet& StateSpaceImpl::get_goal_vertices() const { return m_goal_vertices; }

const IndexSet& StateSpaceImpl::get_unsolvable_vertices() const { return m_unsolvable_vertices; }
}
