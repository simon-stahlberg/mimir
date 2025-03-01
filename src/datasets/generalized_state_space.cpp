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

#include "mimir/datasets/generalized_state_space.hpp"

#include "mimir/algorithms/nauty.hpp"
#include "mimir/common/timers.hpp"
#include "mimir/datasets/generalized_color_function.hpp"
#include "mimir/datasets/object_graph.hpp"
#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/parser.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/graphs/static_graph.hpp"
#include "mimir/graphs/static_graph_boost_adapter.hpp"
#include "mimir/search/algorithms/brfs.hpp"
#include "mimir/search/algorithms/brfs/event_handlers/interface.hpp"
#include "mimir/search/algorithms/strategies/goal_strategy.hpp"
#include "mimir/search/algorithms/strategies/pruning_strategy.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/axiom_evaluators.hpp"
#include "mimir/search/delete_relaxed_problem_explorator.hpp"
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

class ProblemStateSpace
{
private:
    ProblemGraph m_graph;
    IndexSet m_goal_vertices;
    IndexSet m_unsolvable_vertices;
    DiscreteCostList m_unit_goal_distances;
    ContinuousCostList m_action_goal_distances;

public:
    ProblemStateSpace() = default;
    ProblemStateSpace(ProblemGraph graph,
                      IndexSet goal_vertices,
                      IndexSet unsolvable_vertices,
                      DiscreteCostList unit_goal_distances,
                      ContinuousCostList action_goal_distances);
    ProblemStateSpace(const ProblemStateSpace& other) = delete;
    ProblemStateSpace& operator=(const ProblemStateSpace& other) = delete;
    ProblemStateSpace(ProblemStateSpace&& other) = default;
    ProblemStateSpace& operator=(ProblemStateSpace&& other) = default;

    const ProblemGraph& get_graph() const;
    const IndexSet& get_goal_vertices() const;
    const IndexSet& get_unsolvable_vertices() const;
    const DiscreteCostList& get_unit_goal_distances() const;
    const ContinuousCostList& get_action_goal_distances() const;
};

using ProblemStateSpaceList = std::vector<ProblemStateSpace>;

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

ClassStateSpace::ClassStateSpace(ClassGraph graph) : m_graph(std::move(graph)), m_goal_vertices(), m_unsolvable_vertices()
{
    for (const auto& vertex : m_graph.get_vertices())
    {
        if (is_initial(vertex))
        {
            m_initial_vertices.insert(vertex.get_index());
        }
        if (is_goal(vertex))
        {
            m_goal_vertices.insert(vertex.get_index());
        }
        else if (is_unsolvable(vertex))
        {
            m_unsolvable_vertices.insert(vertex.get_index());
        }
        else
        {
            m_alive_vertices.insert(vertex.get_index());
        }
    }
}

const ClassGraph& ClassStateSpace::get_graph() const { return m_graph; }
const IndexSet& ClassStateSpace::get_initial_vertices() const { return m_initial_vertices; }
const IndexSet& ClassStateSpace::get_goal_vertices() const { return m_goal_vertices; }
const IndexSet& ClassStateSpace::get_unsolvable_vertices() const { return m_unsolvable_vertices; }
const IndexSet& ClassStateSpace::get_alive_vertices() const { return m_alive_vertices; }

/**
 * GeneralizedStateSpace
 */

using CertificateToIndex = std::unordered_map<std::unique_ptr<const nauty_wrapper::Certificate>,
                                              Index,
                                              nauty_wrapper::UniqueCertificateUniquePtrHash,
                                              nauty_wrapper::UniqueCertificateUniquePtrEqualTo>;

using CertificateSet = std::unordered_set<loki::ObserverPtr<const nauty_wrapper::Certificate>,
                                          loki::Hash<loki::ObserverPtr<const nauty_wrapper::Certificate>>,
                                          loki::EqualTo<loki::ObserverPtr<const nauty_wrapper::Certificate>>>;
using CertificateList = std::vector<std::unique_ptr<const nauty_wrapper::Certificate>>;

struct SymmetriesData
{
    const GeneralizedColorFunction& color_function;
    CertificateSet equiv_classes;
    CertificateList per_state_equiv_class;
    StateSet representative_states;

    explicit SymmetriesData(const GeneralizedColorFunction& color_function) : color_function(color_function), equiv_classes(), per_state_equiv_class() {}
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
        return !is_new_succ || m_symm_data.representative_states.contains(succ_state);
    }
};

class SymmetryReducedProblemGraphBrFSAlgorithmEventHandler : public BrFSAlgorithmEventHandlerBase<SymmetryReducedProblemGraphBrFSAlgorithmEventHandler>
{
private:
    const GeneralizedStateSpace::Options::ProblemSpecific& m_options;
    StaticProblemGraph& m_graph;
    IndexSet& m_goal_vertices;
    SymmetriesData& m_symm_data;

    StateMap<VertexIndex> m_state_to_vertex_index;

    /* Implement AlgorithmEventHandlerBase interface */
    friend class BrFSAlgorithmEventHandlerBase<SymmetryReducedProblemGraphBrFSAlgorithmEventHandler>;

    std::unique_ptr<const nauty_wrapper::Certificate> compute_certificate(State state, const ProblemImpl& problem)
    {
        const auto object_graph = create_object_graph(state, problem, m_symm_data.color_function, m_options.mark_true_goal_literals);

        return std::make_unique<const nauty_wrapper::Certificate>(nauty_wrapper::SparseGraph(object_graph).compute_certificate());
    }

    void on_expand_state_impl(State state, const ProblemImpl& problem)
    {
        if (!m_state_to_vertex_index.contains(state))
        {
            m_state_to_vertex_index.emplace(state, m_graph.add_vertex(VertexIndex(-1), state));
        }
    }

    void on_expand_goal_state_impl(State state, const ProblemImpl& problem) { m_goal_vertices.insert(m_state_to_vertex_index.at(state)); }

    void on_generate_state_impl(State state, GroundAction action, ContinuousCost action_cost, State successor_state, const ProblemImpl& problem)
    {
        const auto source_v_idx = m_state_to_vertex_index.at(state);

        auto certificate = compute_certificate(successor_state, problem);
        auto it = m_symm_data.equiv_classes.find(certificate.get());
        const auto is_symmetric = (it != m_symm_data.equiv_classes.end());

        if (is_symmetric)
        {
            /* Existing class re-encountered: add edge if between existing representative states! */

            if (m_symm_data.representative_states.contains(successor_state))
            {
                /* Always add edges between symmetric states. */
                const auto target_v_idx = m_state_to_vertex_index.at(successor_state);
                m_graph.add_directed_edge(source_v_idx, target_v_idx, EdgeIndex(-1), action, action_cost);
            }

            /* Always mark symmetric states as prunable. */
            m_symm_data.representative_states.insert(successor_state);
        }
        else
        {
            /* New class determined: add vertex and edge! */

            assert(m_graph.get_num_vertices() == m_symm_data.per_state_equiv_class.size());
            assert(m_graph.get_num_vertices() == m_state_to_vertex_index.size());

            m_symm_data.per_state_equiv_class.push_back(std::move(certificate));
            m_symm_data.equiv_classes.insert(m_symm_data.per_state_equiv_class.back().get());

            const auto target_v_idx = m_graph.add_vertex(VertexIndex(-1), successor_state);
            m_state_to_vertex_index.emplace(successor_state, target_v_idx);
            m_graph.add_directed_edge(source_v_idx, target_v_idx, EdgeIndex(-1), action, action_cost);
        }
    }

    void on_generate_state_in_search_tree_impl(State state, GroundAction action, ContinuousCost action_cost, State successor_state, const ProblemImpl& problem)
    {
    }

    void
    on_generate_state_not_in_search_tree_impl(State state, GroundAction action, ContinuousCost action_cost, State successor_state, const ProblemImpl& problem)
    {
    }

    void on_finish_g_layer_impl(uint32_t g_value, uint64_t num_expanded_states, uint64_t num_generated_states) {}

    void on_start_search_impl(State start_state, const ProblemImpl& problem)
    {
        const auto v_idx = m_graph.add_vertex(VertexIndex(-1), start_state);
        m_state_to_vertex_index.emplace(start_state, v_idx);

        /* Compute certificate for start state. */
        m_symm_data.per_state_equiv_class.push_back(compute_certificate(start_state, problem));
        m_symm_data.equiv_classes.insert(m_symm_data.per_state_equiv_class.back().get());
        m_symm_data.representative_states.insert(start_state);
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

    void on_solved_impl(const Plan& plan, const ProblemImpl& problem) {}

    void on_unsolvable_impl() {}

    void on_exhausted_impl() {}

public:
    explicit SymmetryReducedProblemGraphBrFSAlgorithmEventHandler(const GeneralizedStateSpace::Options::ProblemSpecific& options,
                                                                  StaticProblemGraph& graph,
                                                                  IndexSet& goal_vertices,
                                                                  SymmetriesData& symm_data,
                                                                  bool quiet = true) :
        BrFSAlgorithmEventHandlerBase<SymmetryReducedProblemGraphBrFSAlgorithmEventHandler>(quiet),
        m_options(options),
        m_graph(graph),
        m_goal_vertices(goal_vertices),
        m_symm_data(symm_data)
    {
    }
};

class ProblemGraphBrFSAlgorithmEventHandler : public BrFSAlgorithmEventHandlerBase<ProblemGraphBrFSAlgorithmEventHandler>
{
private:
    const GeneralizedStateSpace::Options::ProblemSpecific& m_options;
    StaticProblemGraph& m_graph;
    IndexSet& m_goal_vertices;

    StateMap<VertexIndex> m_state_to_vertex_index;

    /* Implement AlgorithmEventHandlerBase interface */
    friend class BrFSAlgorithmEventHandlerBase<ProblemGraphBrFSAlgorithmEventHandler>;

    void on_expand_state_impl(State state, const ProblemImpl& problem)
    {
        if (!m_state_to_vertex_index.contains(state))
            m_state_to_vertex_index.emplace(state, m_graph.add_vertex(VertexIndex(-1), state));
    }

    void on_expand_goal_state_impl(State state, const ProblemImpl& problem) { m_goal_vertices.insert(m_state_to_vertex_index.at(state)); }

    void on_generate_state_impl(State state, GroundAction action, ContinuousCost action_cost, State successor_state, const ProblemImpl& problem)
    {
        const auto source_vertex_index = m_state_to_vertex_index.at(state);
        const auto target_vertex_index = m_state_to_vertex_index.contains(successor_state) ? m_state_to_vertex_index.at(successor_state) :
                                                                                             m_graph.add_vertex(VertexIndex(-1), successor_state);
        m_state_to_vertex_index.emplace(successor_state, target_vertex_index);
        m_graph.add_directed_edge(source_vertex_index, target_vertex_index, EdgeIndex(-1), action, action_cost);
    }

    void on_generate_state_in_search_tree_impl(State state, GroundAction action, ContinuousCost action_cost, State successor_state, const ProblemImpl& problem)
    {
    }

    void
    on_generate_state_not_in_search_tree_impl(State state, GroundAction action, ContinuousCost action_cost, State successor_state, const ProblemImpl& problem)
    {
    }

    void on_finish_g_layer_impl(uint32_t g_value, uint64_t num_expanded_states, uint64_t num_generated_states) {}

    void on_start_search_impl(State start_state, const ProblemImpl& problem) {}

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

    void on_solved_impl(const Plan& plan, const ProblemImpl& problem) {}

    void on_unsolvable_impl() {}

    void on_exhausted_impl() {}

public:
    explicit ProblemGraphBrFSAlgorithmEventHandler(const GeneralizedStateSpace::Options::ProblemSpecific& options,
                                                   StaticProblemGraph& graph,
                                                   IndexSet& goal_vertices,
                                                   bool quiet = true) :
        BrFSAlgorithmEventHandlerBase<ProblemGraphBrFSAlgorithmEventHandler>(quiet),
        m_options(options),
        m_graph(graph),
        m_goal_vertices(goal_vertices)
    {
    }
};

static std::optional<std::pair<ProblemStateSpace, CertificateList>>
compute_problem_graph_with_symmetry_reduction(const SearchContext& context,
                                              const GeneralizedColorFunction& color_function,
                                              const GeneralizedStateSpace::Options::ProblemSpecific& options)
{
    const auto& problem = *context.get_problem();
    const auto applicable_action_generator = context.get_applicable_action_generator();
    const auto state_repository = context.get_state_repository();

    assert(applicable_action_generator);
    assert(state_repository);

    if (options.remove_if_unsolvable && !problem.static_goal_holds())
    {
        // Unsolvable
        return std::nullopt;
    }

    auto graph = StaticProblemGraph();
    auto goal_vertices = IndexSet {};

    auto symm_data = SymmetriesData(color_function);

    auto goal_test = std::make_shared<ProblemGoal>(context.get_problem());
    auto event_handler = std::make_shared<SymmetryReducedProblemGraphBrFSAlgorithmEventHandler>(options, graph, goal_vertices, symm_data, false);
    auto pruning_strategy = std::make_shared<SymmetryStatePruning>(symm_data);
    auto result = find_solution_brfs(context, state_repository->get_or_create_initial_state(), event_handler, goal_test, pruning_strategy, true);

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
                          std::move(symm_data.per_state_equiv_class));
}

static std::optional<ProblemStateSpace> compute_problem_graph_without_symmetry_reduction(const SearchContext& context,
                                                                                         const GeneralizedStateSpace::Options::ProblemSpecific& options)
{
    const auto& problem = *context.get_problem();
    const auto applicable_action_generator = context.get_applicable_action_generator();
    const auto state_repository = context.get_state_repository();

    assert(applicable_action_generator);
    assert(state_repository);

    if (options.remove_if_unsolvable && !problem.static_goal_holds())
    {
        // Unsolvable
        return std::nullopt;
    }

    auto graph = StaticProblemGraph();
    auto goal_vertices = IndexSet {};

    auto goal_test = std::make_shared<ProblemGoal>(context.get_problem());
    auto event_handler = std::make_shared<ProblemGraphBrFSAlgorithmEventHandler>(options, graph, goal_vertices, false);
    auto pruning_strategy = std::make_shared<DuplicateStatePruning>();
    auto result = find_solution_brfs(context, state_repository->get_or_create_initial_state(), event_handler, goal_test, pruning_strategy, true);

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

    return ProblemStateSpace(std::move(bidir_graph),
                             std::move(goal_vertices),
                             std::move(unsolvable_vertices),
                             std::move(unit_goal_distances),
                             std::move(action_goal_distances));
}

static std::vector<std::tuple<ProblemStateSpace, CertificateList, SearchContext>>
compute_problem_graphs_with_symmetry_reduction(const GeneralizedSearchContext& context, const GeneralizedStateSpace::Options::ProblemSpecific& options)
{
    auto color_function = GeneralizedColorFunction(context.get_generalized_problem());

    auto problem_graphs = std::vector<std::tuple<ProblemStateSpace, CertificateList, SearchContext>> {};
    for (const auto& search_context : context.get_search_contexts())
    {
        auto result = compute_problem_graph_with_symmetry_reduction(search_context, color_function, options);

        if (!result)
        {
            continue;
        }

        problem_graphs.emplace_back(std::move(result->first), std::move(result->second), search_context);
    }

    return problem_graphs;
}

static std::vector<ProblemStateSpace> compute_problem_graphs_without_symmetry_reduction(const GeneralizedSearchContext& context,
                                                                                        const GeneralizedStateSpace::Options::ProblemSpecific& options)
{
    auto problem_graphs = std::vector<ProblemStateSpace> {};
    for (const auto& search_context : context.get_search_contexts())
    {
        auto result = compute_problem_graph_without_symmetry_reduction(search_context, options);

        if (!result)
        {
            continue;
        }

        problem_graphs.emplace_back(std::move(result.value()));
    }

    return problem_graphs;
}

static std::tuple<ProblemGraphList, ClassStateSpace, GeneralizedSearchContext>
compute_problem_and_class_state_spaces_with_symmetry_reduction(const GeneralizedSearchContext& context, const GeneralizedStateSpace::Options& options)
{
    auto result = compute_problem_graphs_with_symmetry_reduction(context, options.problem_options);

    if (options.sort_ascending_by_num_states)
    {
        std::sort(result.begin(),
                  result.end(),
                  [](auto&& lhs, auto&& rhs) { return std::get<0>(lhs).get_graph().get_num_vertices() < std::get<0>(rhs).get_graph().get_num_vertices(); });
    }

    /* Step 2: Compute the `ClassGraph` by looping through the `StaticProblemGraphs`.
        Meanwhile, translate each `StaticProblemGraph` into a `ProblemGraph` that maps to the `ClassVertices` and `ClassEdges`
     */

    auto final_problems = ProblemList {};
    auto final_search_contexts = SearchContextList {};
    auto final_problem_graphs = ProblemGraphList {};

    auto class_graph = StaticClassGraph {};

    auto problem_vertex_certificate_to_class_v_idx = CertificateToIndex {};

    auto problem_idx = Index(0);

    for (auto& [problem_state_space, vertex_certificates, search_context] : result)
    {
        const auto& problem_graph = problem_state_space.get_graph();
        auto& problem_goal_vertices = problem_state_space.get_goal_vertices();
        auto& problem_unsolvable_vertices = problem_state_space.get_unsolvable_vertices();
        auto& problem_unit_goal_distances = problem_state_space.get_unit_goal_distances();
        auto& problem_action_goal_distances = problem_state_space.get_action_goal_distances();

        auto final_problem_graph = StaticProblemGraph();
        bool has_equivalent_initial_state = false;
        auto problem_v_idx_to_class_v_idx = std::unordered_map<Index, Index> {};
        auto problem_v_idx_to_final_v_idx = std::unordered_map<Index, Index> {};
        auto instantiated_class_v_idxs = IndexSet {};

        for (VertexIndex problem_v_idx = 0; problem_v_idx < problem_graph.get_num_vertices(); ++problem_v_idx)
        {
            const auto& v = problem_graph.get_vertex(problem_v_idx);
            const auto unit_goal_distance = problem_unit_goal_distances.at(problem_v_idx);
            const auto action_goal_distance = problem_action_goal_distances.at(problem_v_idx);
            const auto is_initial = (problem_v_idx == 0);
            const auto is_goal = problem_goal_vertices.contains(problem_v_idx);
            const auto is_unsolvable = problem_unsolvable_vertices.contains(problem_v_idx);
            const auto is_alive = (!(is_goal || is_unsolvable));

            auto result = problem_vertex_certificate_to_class_v_idx.emplace(std::move(vertex_certificates.at(problem_v_idx)), class_graph.get_num_vertices());

            if (result.second)
            {
                /* Discovered new class vertex. */

                const auto class_v_idx = result.first->second;

                class_graph.add_vertex(class_v_idx,
                                       problem_idx,
                                       problem_v_idx,
                                       unit_goal_distance,
                                       action_goal_distance,
                                       is_initial,
                                       is_goal,
                                       is_unsolvable,
                                       is_alive);
                const auto final_problem_v_idx = final_problem_graph.add_vertex(class_v_idx, get_state(v));

                problem_v_idx_to_final_v_idx.emplace(problem_v_idx, final_problem_v_idx);
                problem_v_idx_to_class_v_idx.emplace(problem_v_idx, class_v_idx);
                instantiated_class_v_idxs.insert(class_v_idx);
            }
            else
            {
                /* Encountered existing class vertex. */

                if (problem_v_idx == 0)
                {
                    has_equivalent_initial_state = true;
                    break;
                }

                /* We create a node in the problem graph that points to the class vertex.
                   We cannot skip it to be able to compute tuple graphs in the problem graph of a representative of a class vertex. */
                const auto class_v_idx = result.first->second;
                const auto final_problem_v_idx = final_problem_graph.add_vertex(class_v_idx, get_state(v));

                problem_v_idx_to_class_v_idx.emplace(problem_v_idx, class_v_idx);
                problem_v_idx_to_final_v_idx.emplace(problem_v_idx, final_problem_v_idx);
            }
        }

        if (has_equivalent_initial_state)
        {
            continue;
        }

        for (const auto& e : problem_graph.get_edges())
        {
            const auto problem_e_idx = e.get_index();
            const auto class_e_idx = Index(class_graph.get_num_edges());
            const auto action_cost = get_action_cost(e);

            const auto final_problem_src_v_idx = problem_v_idx_to_final_v_idx.at(e.get_source());
            const auto final_problem_dst_v_idx = problem_v_idx_to_final_v_idx.at(e.get_target());

            const auto class_src_v_idx = problem_v_idx_to_class_v_idx.at(e.get_source());
            const auto class_dst_v_idx = problem_v_idx_to_class_v_idx.at(e.get_target());

            /* Only instantiate class edge if the edge transitions between a newly instantiated class vertex. */
            if (instantiated_class_v_idxs.contains(class_src_v_idx) || instantiated_class_v_idxs.contains(class_dst_v_idx))
            {
                class_graph.add_directed_edge(class_src_v_idx, class_dst_v_idx, class_e_idx, problem_idx, problem_e_idx, action_cost);
            }

            /* Always instantiate a problem edge. Same reasons as for vertices above applies. */
            final_problem_graph.add_directed_edge(final_problem_src_v_idx, final_problem_dst_v_idx, class_e_idx, get_action(e), get_action_cost(e));
        }

        final_problem_graphs.push_back(ProblemGraph(std::move(final_problem_graph)));
        final_problems.push_back(search_context.get_problem());
        final_search_contexts.push_back(search_context);

        ++problem_idx;
    }

    auto class_state_space = ClassStateSpace(ClassGraph(std::move(class_graph)));

    return { std::move(final_problem_graphs),
             std::move(class_state_space),
             GeneralizedSearchContext(GeneralizedProblem(context.get_generalized_problem().get_domain(), final_problems), final_search_contexts) };
}

static std::tuple<ProblemGraphList, ClassStateSpace, GeneralizedSearchContext>
compute_problem_and_class_state_spaces_without_symmetry_reduction(const GeneralizedSearchContext& context, const GeneralizedStateSpace::Options& options)
{
    auto problem_graphs = compute_problem_graphs_without_symmetry_reduction(context, options.problem_options);

    if (options.sort_ascending_by_num_states)
    {
        std::sort(problem_graphs.begin(),
                  problem_graphs.end(),
                  [](auto&& lhs, auto&& rhs) { return lhs.get_graph().get_num_vertices() < rhs.get_graph().get_num_vertices(); });
    }

    /* Step 2: Compute the `ClassGraph` by looping through the `StaticProblemGraphs`.
        Meanwhile, translate each `StaticProblemGraph` into a `ProblemGraph` that maps to the `ClassVertices` and `ClassEdges`
     */

    auto final_problem_graphs = ProblemGraphList {};

    auto class_graph = StaticClassGraph {};

    /* Simple instantiation without symmetry reduction. */
    auto per_problem_vertex_to_class_vertex = std::vector<IndexList>(problem_graphs.size());

    auto problem_idx = Index(0);

    for (auto& problem_state_space : problem_graphs)
    {
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
            const auto is_initial = (problem_v_idx == 0);
            const auto is_goal = problem_goal_vertices.contains(problem_v_idx);
            const auto is_unsolvable = problem_unsolvable_vertices.contains(problem_v_idx);
            const auto is_alive = (!(is_goal || is_unsolvable));

            class_graph
                .add_vertex(class_v_idx, problem_idx, problem_v_idx, unit_goal_distance, action_goal_distance, is_initial, is_goal, is_unsolvable, is_alive);
            final_problem_graph.add_vertex(class_v_idx, get_state(v));
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

        final_problem_graphs.push_back(ProblemGraph(std::move(final_problem_graph)));

        ++problem_idx;
    }

    auto class_state_space = ClassStateSpace(ClassGraph(std::move(class_graph)));

    return { std::move(final_problem_graphs), std::move(class_state_space), context };
}

GeneralizedStateSpace::GeneralizedStateSpace(GeneralizedSearchContext context, const GeneralizedStateSpace::Options& options) : m_context(std::move(context))
{
    /* We write separate code for the two cases where symmetry pruning is either enabled or disabled
       because in the latter case we can write much simpler code and we dont have to always check again the option. */
    if (options.problem_options.symmetry_pruning)
    {
        auto [problem_state_spaces_, class_state_space_, context_] = compute_problem_and_class_state_spaces_with_symmetry_reduction(m_context, options);
        m_problem_graphs = std::move(problem_state_spaces_);
        m_class_state_space = std::move(class_state_space_);
        m_context = std::move(context_);
    }
    else
    {
        auto [problem_state_spaces_, class_state_space_, context_] = compute_problem_and_class_state_spaces_without_symmetry_reduction(m_context, options);
        m_problem_graphs = std::move(problem_state_spaces_);
        m_class_state_space = std::move(class_state_space_);
        m_context = std::move(context_);
    }
}

const GeneralizedSearchContext& GeneralizedStateSpace::get_generalized_search_context() const { return m_context; }

const ProblemGraphList& GeneralizedStateSpace::get_problem_graphs() const { return m_problem_graphs; }

const ClassStateSpace& GeneralizedStateSpace::get_class_state_space() const { return m_class_state_space; }

const ProblemGraph& GeneralizedStateSpace::get_problem_graph(const ClassVertex& vertex) const { return m_problem_graphs.at(get_problem_index(vertex)); }

const ProblemGraph& GeneralizedStateSpace::get_problem_graph(const ClassEdge& edge) const { return m_problem_graphs.at(get_problem_index(edge)); }

const ProblemVertex& GeneralizedStateSpace::get_problem_vertex(const ClassVertex& vertex) const
{
    return m_problem_graphs.at(get_problem_index(vertex)).get_graph().get_vertex(get_problem_vertex_index(vertex));
}

const ProblemEdge& GeneralizedStateSpace::get_problem_edge(const ClassEdge& edge) const
{
    return m_problem_graphs.at(get_problem_index(edge)).get_graph().get_edge(get_problem_edge_index(edge));
}

const ClassVertex& GeneralizedStateSpace::get_class_vertex(const ProblemVertex& vertex) const
{
    return m_class_state_space.get_graph().get_vertex(get_class_vertex_index(vertex));
}

const ClassEdge& GeneralizedStateSpace::get_class_edge(const ProblemEdge& edge) const
{
    return m_class_state_space.get_graph().get_edge(get_class_edge_index(edge));
}

static ClassStateSpace create_induced_subspace_helper(const IndexSet& subgraph_class_v_idxs, const ClassGraph& complete_graph)
{
    auto sub_graph = StaticClassGraph();

    /* 2. Instantiate vertices */
    auto old_to_new_class_v_idxs = IndexMap<Index> {};

    for (const auto& class_v_idx : subgraph_class_v_idxs)
    {
        const auto& class_v = complete_graph.get_vertex(class_v_idx);

        const auto new_class_v_idx = sub_graph.add_vertex(class_v);

        old_to_new_class_v_idxs.emplace(class_v_idx, new_class_v_idx);
    }

    /* 3. Instantiate edges */
    for (const auto& class_v_idx : subgraph_class_v_idxs)
    {
        for (const auto& class_e : complete_graph.get_adjacent_edges<ForwardTraversal>(class_v_idx))
        {
            if (subgraph_class_v_idxs.contains(class_e.get_target()))
            {
                const auto new_class_source_v_idx = old_to_new_class_v_idxs.at(class_e.get_source());
                const auto new_class_target_v_idx = old_to_new_class_v_idxs.at(class_e.get_target());

                sub_graph.add_directed_edge(new_class_source_v_idx, new_class_target_v_idx, class_e);
            }
        }
    }

    /* Return a `ClassStateSpace` that wraps a bidirectional `ClassGraph` obtained from the `StaticClassGraph`. */
    return ClassStateSpace(ClassGraph(std::move(sub_graph)));
}

ClassStateSpace GeneralizedStateSpace::create_induced_subspace_from_class_vertex_indices(const IndexList& class_vertex_indices) const
{
    auto unique_class_v_idxs = IndexSet(class_vertex_indices.begin(), class_vertex_indices.end());

    return create_induced_subspace_helper(unique_class_v_idxs, m_class_state_space.get_graph());
}

ClassStateSpace GeneralizedStateSpace::create_induced_subspace_from_problem_indices(const IndexList& problem_indices) const
{
    /* Collect unique class vertices */
    auto unique_class_v_idxs = IndexSet {};

    for (const auto& problem_idx : problem_indices)
    {
        const auto& problem_graph = m_problem_graphs.at(problem_idx);

        for (const auto& problem_vertex : problem_graph.get_vertices())
        {
            const auto& class_vertex = get_class_vertex(problem_vertex);

            unique_class_v_idxs.insert(class_vertex.get_index());
        }
    }

    return create_induced_subspace_helper(unique_class_v_idxs, m_class_state_space.get_graph());
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
