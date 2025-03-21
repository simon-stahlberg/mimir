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

using namespace mimir::formalism;
using namespace mimir::search;

namespace mimir::graphs
{

std::ostream& operator<<(std::ostream& out, const ClassVertex& vertex)
{
    out << "class_v_idx=" << vertex.get_index() << "\n"          //
        << " problem_idx=" << get_problem_index(vertex) << "\n"  //
        << " problem_v_idx=" << get_problem_vertex_index(vertex);
    return out;
}

std::ostream& operator<<(std::ostream& out, const ClassEdge& edge)
{
    out << "class_e_idx=" << edge.get_index() << "\n"          //
        << " class_src_idx=" << edge.get_source() << "\n"      //
        << " class_dst_idx=" << edge.get_target() << "\n"      //
        << " problem_idx=" << get_problem_index(edge) << "\n"  //
        << " problem_e_idx=" << get_problem_edge_index(edge);
    return out;
}

}

namespace mimir::datasets
{

/**
 * GeneralizedStateSpace
 */

static std::tuple<graphs::ProblemGraphList, graphs::ClassGraph, GeneralizedSearchContext>
compute_problem_and_class_state_spaces_with_symmetry_reduction(const GeneralizedSearchContext& context, const GeneralizedStateSpace::Options& options)
{
    auto result = compute_problem_graphs_with_symmetry_reduction(context, options.problem_options);

    if (options.sort_ascending_by_num_states)
    {
        std::sort(result.begin(),
                  result.end(),
                  [](auto&& lhs, auto&& rhs) { return lhs.first.get_graph().get_num_vertices() < rhs.first.get_graph().get_num_vertices(); });
    }

    /* Step 2: Compute the `ClassGraph` by looping through the `StaticProblemGraphs`.
        Meanwhile, translate each `StaticProblemGraph` into a `ProblemGraph` that maps to the `ClassVertices` and `ClassEdges`
     */

    auto final_problems = ProblemList {};
    auto final_search_contexts = SearchContextList {};
    auto final_problem_graphs = graphs::ProblemGraphList {};

    auto class_graph = graphs::StaticClassGraph {};

    auto certificate_to_class_v_idx = ToIndexMap<const nauty_wrapper::Certificate*> {};

    auto problem_idx = Index(0);

    for (auto& [problem_state_space, search_context] : result)
    {
        const auto& problem_graph = problem_state_space.get_graph();
        auto& problem_goal_vertices = problem_state_space.get_goal_vertices();
        auto& problem_unsolvable_vertices = problem_state_space.get_unsolvable_vertices();
        auto& problem_unit_goal_distances = problem_state_space.get_unit_goal_distances();
        auto& problem_action_goal_distances = problem_state_space.get_action_goal_distances();

        auto final_problem_graph = graphs::StaticProblemGraph();
        bool has_equivalent_initial_state = false;
        auto problem_v_idx_to_class_v_idx = std::unordered_map<Index, Index> {};
        auto problem_v_idx_to_final_v_idx = std::unordered_map<Index, Index> {};
        auto instantiated_class_v_idxs = IndexSet {};

        for (graphs::VertexIndex problem_v_idx = 0; problem_v_idx < problem_graph.get_num_vertices(); ++problem_v_idx)
        {
            const auto& v = problem_graph.get_vertex(problem_v_idx);
            const auto unit_goal_distance = problem_unit_goal_distances.at(problem_v_idx);
            const auto action_goal_distance = problem_action_goal_distances.at(problem_v_idx);
            const auto is_initial = (problem_v_idx == 0);
            const auto is_goal = problem_goal_vertices.contains(problem_v_idx);
            const auto is_unsolvable = problem_unsolvable_vertices.contains(problem_v_idx);
            const auto is_alive = (!(is_goal || is_unsolvable));

            const auto certificate = graphs::get_certificate(v);

            auto it = certificate_to_class_v_idx.find(certificate.get());

            if (it == certificate_to_class_v_idx.end())
            {
                /* Discovered new class vertex. */

                const auto class_v_idx = graphs::VertexIndex(class_graph.get_num_vertices());

                class_graph.add_vertex(class_v_idx,
                                       problem_idx,
                                       problem_v_idx,
                                       unit_goal_distance,
                                       action_goal_distance,
                                       is_initial,
                                       is_goal,
                                       is_unsolvable,
                                       is_alive);
                const auto final_problem_v_idx = final_problem_graph.add_vertex(class_v_idx, get_state(v), std::move(certificate));
                certificate_to_class_v_idx.emplace(graphs::get_certificate(final_problem_graph.get_vertex(final_problem_v_idx)).get(), final_problem_v_idx);

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
                    break;  ///< Remove the entire problem if it is isomorphic to another one
                }

                /* We create a node in the problem graph that points to the class vertex.
                   We cannot skip it to be able to compute tuple graphs in the problem graph of a representative of a class vertex. */
                const auto class_v_idx = it->second;
                const auto final_problem_v_idx = final_problem_graph.add_vertex(class_v_idx, get_state(v), certificate);

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

        final_problem_graphs.push_back(graphs::ProblemGraph(std::move(final_problem_graph)));
        final_problems.push_back(search_context.get_problem());
        final_search_contexts.push_back(search_context);

        ++problem_idx;
    }

    return { std::move(final_problem_graphs),
             graphs::ClassGraph(std::move(class_graph)),
             GeneralizedSearchContext(GeneralizedProblem(context.get_generalized_problem().get_domain(), final_problems), final_search_contexts) };
}

static std::tuple<graphs::ProblemGraphList, graphs::ClassGraph, GeneralizedSearchContext>
compute_problem_and_class_state_spaces_without_symmetry_reduction(const GeneralizedSearchContext& context, const GeneralizedStateSpace::Options& options)
{
    auto result = compute_problem_graphs_without_symmetry_reduction(context, options.problem_options);

    if (options.sort_ascending_by_num_states)
    {
        std::sort(result.begin(),
                  result.end(),
                  [](auto&& lhs, auto&& rhs) { return lhs.first.get_graph().get_num_vertices() < rhs.first.get_graph().get_num_vertices(); });
    }

    /* Step 2: Compute the `ClassGraph` by looping through the `StaticProblemGraphs`.
        Meanwhile, translate each `StaticProblemGraph` into a `ProblemGraph` that maps to the `ClassVertices` and `ClassEdges`
     */

    auto final_problems = ProblemList {};
    auto final_search_contexts = SearchContextList {};
    auto final_problem_graphs = graphs::ProblemGraphList {};

    auto class_graph = graphs::StaticClassGraph {};

    /* Simple instantiation without symmetry reduction. */
    auto per_problem_vertex_to_class_vertex = std::vector<IndexList>(result.size());

    auto problem_idx = Index(0);

    for (auto& [problem_state_space, search_context] : result)
    {
        const auto& problem_graph = problem_state_space.get_graph();
        auto& problem_goal_vertices = problem_state_space.get_goal_vertices();
        auto& problem_unsolvable_vertices = problem_state_space.get_unsolvable_vertices();
        auto& problem_unit_goal_distances = problem_state_space.get_unit_goal_distances();
        auto& problem_action_goal_distances = problem_state_space.get_action_goal_distances();

        auto final_problem_graph = graphs::StaticProblemGraph();

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
            final_problem_graph.add_vertex(class_v_idx, get_state(v), std::shared_ptr<const nauty_wrapper::Certificate>(nullptr));
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

        final_problems.push_back(search_context.get_problem());
        final_search_contexts.push_back(search_context);
        final_problem_graphs.push_back(graphs::ProblemGraph(std::move(final_problem_graph)));

        ++problem_idx;
    }

    return { std::move(final_problem_graphs),
             graphs::ClassGraph(std::move(class_graph)),
             GeneralizedSearchContext(GeneralizedProblem(context.get_generalized_problem().get_domain(), final_problems), final_search_contexts) };
}

GeneralizedStateSpace::GeneralizedStateSpace(GeneralizedSearchContext context, const GeneralizedStateSpace::Options& options) :
    m_context(std::move(context)),
    m_initial_vertices(),
    m_goal_vertices(),
    m_unsolvable_vertices()
{
    /* We write separate code for the two cases where symmetry pruning is either enabled or disabled
       because in the latter case we can write much simpler code and we dont have to always check again the option. */
    if (options.problem_options.symmetry_pruning)
    {
        auto [problem_state_spaces_, class_state_space_, context_] = compute_problem_and_class_state_spaces_with_symmetry_reduction(m_context, options);
        m_problem_graphs = std::move(problem_state_spaces_);
        m_graph = std::move(class_state_space_);
        m_context = std::move(context_);
    }
    else
    {
        auto [problem_state_spaces_, class_state_space_, context_] = compute_problem_and_class_state_spaces_without_symmetry_reduction(m_context, options);
        m_problem_graphs = std::move(problem_state_spaces_);
        m_graph = std::move(class_state_space_);
        m_context = std::move(context_);
    }

    for (const auto& vertex : get_graph().get_vertices())
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
    }
}

const GeneralizedSearchContext& GeneralizedStateSpace::get_generalized_search_context() const { return m_context; }

const graphs::ProblemGraphList& GeneralizedStateSpace::get_problem_graphs() const { return m_problem_graphs; }

const graphs::ClassGraph& GeneralizedStateSpace::get_graph() const { return m_graph; }

const IndexSet& GeneralizedStateSpace::get_initial_vertices() const { return m_initial_vertices; }

const IndexSet& GeneralizedStateSpace::get_goal_vertices() const { return m_goal_vertices; }

const IndexSet& GeneralizedStateSpace::get_unsolvable_vertices() const { return m_unsolvable_vertices; }

const graphs::ProblemGraph& GeneralizedStateSpace::get_problem_graph(const graphs::ClassVertex& vertex) const
{
    return m_problem_graphs.at(get_problem_index(vertex));
}

const graphs::ProblemGraph& GeneralizedStateSpace::get_problem_graph(const graphs::ClassEdge& edge) const
{
    return m_problem_graphs.at(get_problem_index(edge));
}

const graphs::ProblemVertex& GeneralizedStateSpace::get_problem_vertex(const graphs::ClassVertex& vertex) const
{
    return m_problem_graphs.at(get_problem_index(vertex)).get_graph().get_vertex(get_problem_vertex_index(vertex));
}

const graphs::ProblemEdge& GeneralizedStateSpace::get_problem_edge(const graphs::ClassEdge& edge) const
{
    return m_problem_graphs.at(get_problem_index(edge)).get_graph().get_edge(get_problem_edge_index(edge));
}

const Problem& GeneralizedStateSpace::get_problem(const graphs::ClassVertex& vertex) const
{
    return m_context.get_generalized_problem().get_problems().at(get_problem_index(vertex));
}

const Problem& GeneralizedStateSpace::get_problem(const graphs::ClassEdge& edge) const
{
    return m_context.get_generalized_problem().get_problems().at(get_problem_index(edge));
}

const graphs::ClassVertex& GeneralizedStateSpace::get_class_vertex(const graphs::ProblemVertex& vertex) const
{
    return get_graph().get_vertex(get_class_vertex_index(vertex));
}

const graphs::ClassEdge& GeneralizedStateSpace::get_class_edge(const graphs::ProblemEdge& edge) const
{
    return get_graph().get_edge(get_class_edge_index(edge));
}

static graphs::ClassGraph create_induced_subspace_helper(const IndexSet& subgraph_class_v_idxs, const graphs::ClassGraph& complete_graph)
{
    auto sub_graph = graphs::StaticClassGraph();

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
        for (const auto& class_e : complete_graph.get_adjacent_edges<graphs::Forward>(class_v_idx))
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
    return graphs::ClassGraph(std::move(sub_graph));
}

graphs::ClassGraph GeneralizedStateSpace::create_induced_subgraph_from_class_vertex_indices(const IndexList& class_vertex_indices) const
{
    auto unique_class_v_idxs = IndexSet(class_vertex_indices.begin(), class_vertex_indices.end());

    return create_induced_subspace_helper(unique_class_v_idxs, get_graph());
}

graphs::ClassGraph GeneralizedStateSpace::create_induced_subgraph_from_problem_indices(const IndexList& problem_indices) const
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

    return create_induced_subspace_helper(unique_class_v_idxs, get_graph());
}

}
