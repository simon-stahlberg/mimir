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

#include "mimir/common/timers.hpp"
#include "mimir/datasets/generalized_state_space/options.hpp"
#include "mimir/datasets/object_graph.hpp"
#include "mimir/datasets/state_space.hpp"
#include "mimir/formalism/generalized_problem.hpp"
#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/parser.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/graphs/algorithms/nauty.hpp"
#include "mimir/graphs/bgl/graph_algorithms.hpp"
#include "mimir/graphs/bgl/static_graph_algorithms.hpp"
#include "mimir/graphs/static_graph.hpp"
#include "mimir/search/algorithms/brfs.hpp"
#include "mimir/search/algorithms/brfs/event_handlers/interface.hpp"
#include "mimir/search/algorithms/strategies/goal_strategy.hpp"
#include "mimir/search/algorithms/strategies/pruning_strategy.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/axiom_evaluators.hpp"
#include "mimir/search/delete_relaxed_problem_explorator.hpp"
#include "mimir/search/generalized_search_context.hpp"
#include "mimir/search/heuristics/blind.hpp"
#include "mimir/search/openlists/priority_queue.hpp"
#include "mimir/search/search_node.hpp"
#include "mimir/search/search_space.hpp"
#include "mimir/search/state.hpp"
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
 * GeneralizedStateSpaceImpl
 */

static std::pair<GeneralizedStateSpace, CertificateMapsList>
compute_generalized_state_space_with_symmetry_reduction(const std::vector<std::pair<StateSpace, std::optional<CertificateMaps>>>& state_spaces)
{
    /* Step 2: Compute the `ClassGraph` by looping through the `StaticProblemGraphs`.
        Meanwhile, translate each `StaticProblemGraph` into a `ProblemGraph` that maps to the `ClassVertices` and `ClassEdges`
     */

    auto class_graph = graphs::StaticClassGraph {};

    auto initial_vertices = IndexSet {};
    auto goal_vertices = IndexSet {};
    auto unsolvable_vertices = IndexSet {};

    auto vertex_mappings = std::unordered_map<const ProblemImpl*, graphs::VertexIndexList> {};
    auto edge_mappings = std::unordered_map<const ProblemImpl*, graphs::EdgeIndexList> {};

    auto state_space_idx = Index(0);

    auto certificate_to_class_v_idx = CertificateMap<graphs::VertexIndex> {};
    using EdgePair = std::pair<graphs::VertexIndex, graphs::VertexIndex>;
    auto class_e_idxs = std::unordered_map<EdgePair, graphs::EdgeIndex, loki::Hash<EdgePair>, loki::EqualTo<EdgePair>> {};

    auto final_state_spaces = StateSpaceList {};
    auto final_certificate_maps = CertificateMapsList {};

    for (const auto& [state_space, certificate_maps] : state_spaces)
    {
        assert(certificate_maps.has_value());

        const auto& graph = state_space->get_graph();

        const auto& initial_state_certificate = certificate_maps->state_to_cert.at(graphs::get_state(graph.get_vertex(0)));

        if (certificate_to_class_v_idx.contains(initial_state_certificate))
        {
            continue;  ///< StateSpace is isomorphic to a previous one.
        }

        auto final_problem_graph = graphs::StaticProblemGraph();
        auto problem_v_idx_to_class_v_idx = std::unordered_map<Index, Index> {};

        auto& vertex_mapping = vertex_mappings[state_space->get_search_context()->get_problem().get()];
        auto& edge_mapping = edge_mappings[state_space->get_search_context()->get_problem().get()];

        for (const auto& v : graph.get_vertices())
        {
            const auto v_idx = v.get_index();

            const auto& certificate = certificate_maps->state_to_cert.at(graphs::get_state(v));

            auto it = certificate_to_class_v_idx.find(certificate);

            if (it == certificate_to_class_v_idx.end())
            {
                /* Discovered new class vertex. */

                const auto class_v_idx = graphs::VertexIndex(class_graph.get_num_vertices());

                vertex_mapping.push_back(class_v_idx);

                if (graphs::is_initial(v))
                {
                    initial_vertices.insert(class_v_idx);
                }
                if (graphs::is_goal(v))
                {
                    goal_vertices.insert(class_v_idx);
                }
                else if (graphs::is_unsolvable(v))
                {
                    unsolvable_vertices.insert(class_v_idx);
                }

                class_graph.add_vertex(v_idx, state_space_idx);

                certificate_to_class_v_idx.emplace(certificate, v_idx);

                problem_v_idx_to_class_v_idx.emplace(v_idx, class_v_idx);
            }
            else
            {
                /* Encountered existing class vertex. */

                /* We create a node in the problem graph that points to the class vertex.
                   We cannot skip it to be able to compute tuple graphs in the problem graph of a representative of a class vertex. */
                const auto class_v_idx = it->second;

                vertex_mapping.push_back(class_v_idx);

                problem_v_idx_to_class_v_idx.emplace(v_idx, class_v_idx);
            }
        }

        for (const auto& e : graph.get_edges())
        {
            const auto e_idx = e.get_index();

            const auto class_src_v_idx = problem_v_idx_to_class_v_idx.at(e.get_source());
            const auto class_dst_v_idx = problem_v_idx_to_class_v_idx.at(e.get_target());

            /* Do not instantiate parallel edges. */
            auto it = class_e_idxs.find(std::make_pair(class_src_v_idx, class_dst_v_idx));

            if (it == class_e_idxs.end())
            {
                const auto class_e_idx = class_graph.add_directed_edge(class_src_v_idx, class_dst_v_idx, e_idx, state_space_idx);

                edge_mapping.push_back(class_e_idx);

                class_e_idxs.emplace(std::make_pair(class_src_v_idx, class_dst_v_idx), class_e_idx);
            }
            else
            {
                const auto class_e_idx = it->second;

                edge_mapping.push_back(class_e_idx);
            }
        }

        final_state_spaces.push_back(state_space);
        final_certificate_maps.push_back(certificate_maps.value());

        ++state_space_idx;
    }

    return { std::make_shared<GeneralizedStateSpaceImpl>(std::move(final_state_spaces),
                                                         graphs::ClassGraph(std::move(class_graph)),
                                                         std::move(initial_vertices),
                                                         std::move(goal_vertices),
                                                         std::move(unsolvable_vertices),
                                                         std::move(vertex_mappings),
                                                         std::move(edge_mappings)),
             final_certificate_maps };
}

static GeneralizedStateSpace
compute_generalized_state_space_without_symmetry_reduction(const std::vector<std::pair<StateSpace, std::optional<CertificateMaps>>>& state_space_results)
{
    /* Step 2: Compute the `ClassGraph` by looping through the `StaticProblemGraphs`.
        Meanwhile, translate each `StaticProblemGraph` into a `ProblemGraph` that maps to the `ClassVertices` and `ClassEdges`
     */

    auto class_graph = graphs::StaticClassGraph {};

    auto initial_vertices = IndexSet {};
    auto goal_vertices = IndexSet {};
    auto unsolvable_vertices = IndexSet {};

    auto vertex_mappings = std::unordered_map<const ProblemImpl*, graphs::VertexIndexList> {};
    auto edge_mappings = std::unordered_map<const ProblemImpl*, graphs::EdgeIndexList> {};

    auto state_space_idx = Index(0);
    auto state_spaces = StateSpaceList {};

    for (const auto& [state_space, certificate_maps] : state_space_results)
    {
        const auto& graph = state_space->get_graph();

        const auto v_offset = class_graph.get_num_vertices();
        const auto e_offset = class_graph.get_num_edges();

        initial_vertices.insert(v_offset);

        for (const auto& goal_v_idx : state_space->get_goal_vertices())
        {
            goal_vertices.insert(v_offset + goal_v_idx);
        }
        for (const auto& unsolvable_v_idx : state_space->get_unsolvable_vertices())
        {
            unsolvable_vertices.insert(v_offset + unsolvable_v_idx);
        }

        auto& vertex_mapping = vertex_mappings[state_space->get_search_context()->get_problem().get()];
        auto& edge_mapping = edge_mappings[state_space->get_search_context()->get_problem().get()];

        for (const auto& v : graph.get_vertices())
        {
            const auto v_idx = v.get_index();

            class_graph.add_vertex(v_idx, state_space_idx);
            vertex_mapping.push_back(v_offset + v_idx);
        }
        for (const auto& e : graph.get_edges())
        {
            const auto e_idx = e.get_index();

            class_graph.add_directed_edge(v_offset + e.get_source(), v_offset + e.get_target(), e_idx, state_space_idx);
            edge_mapping.push_back(e_offset + e_idx);
        }

        ++state_space_idx;
        state_spaces.push_back(state_space);
    }

    return std::make_shared<GeneralizedStateSpaceImpl>(std::move(state_spaces),
                                                       graphs::ClassGraph(std::move(class_graph)),
                                                       std::move(initial_vertices),
                                                       std::move(goal_vertices),
                                                       std::move(unsolvable_vertices),
                                                       std::move(vertex_mappings),
                                                       std::move(edge_mappings));
}

GeneralizedStateSpaceImpl::GeneralizedStateSpaceImpl(StateSpaceList state_spaces,
                                                     graphs::ClassGraph graph,
                                                     IndexSet initial_vertices,
                                                     IndexSet goal_vertices,
                                                     IndexSet unsolvable_vertices,
                                                     std::unordered_map<const formalism::ProblemImpl*, graphs::VertexIndexList> vertex_mappings,
                                                     std::unordered_map<const formalism::ProblemImpl*, graphs::EdgeIndexList> edge_mappings) :
    m_state_spaces(std::move(state_spaces)),
    m_graph(std::move(graph)),
    m_initial_vertices(std::move(initial_vertices)),
    m_goal_vertices(std::move(goal_vertices)),
    m_unsolvable_vertices(std::move(unsolvable_vertices)),
    m_vertex_mappings(std::move(vertex_mappings)),
    m_edge_mappings(std::move(edge_mappings))
{
}

std::pair<GeneralizedStateSpace, std::optional<std::vector<CertificateMaps>>>
GeneralizedStateSpaceImpl::create(const std::vector<std::pair<StateSpace, std::optional<CertificateMaps>>>& state_spaces, const Options& options)
{
    if (options.symmetry_pruning)
    {
        auto result = compute_generalized_state_space_with_symmetry_reduction(state_spaces);

        return std::make_pair(std::move(result.first), std::make_optional(std::move(result.second)));
    }
    else
    {
        return { compute_generalized_state_space_without_symmetry_reduction(state_spaces), std::nullopt };
    }
}

const StateSpaceList& GeneralizedStateSpaceImpl::get_state_spaces() const { return m_state_spaces; }

const graphs::ClassGraph& GeneralizedStateSpaceImpl::get_graph() const { return m_graph; }

const IndexSet& GeneralizedStateSpaceImpl::get_initial_vertices() const { return m_initial_vertices; }

const IndexSet& GeneralizedStateSpaceImpl::get_goal_vertices() const { return m_goal_vertices; }

const IndexSet& GeneralizedStateSpaceImpl::get_unsolvable_vertices() const { return m_unsolvable_vertices; }

const StateSpace& GeneralizedStateSpaceImpl::get_state_space(const graphs::ClassVertex& vertex) const { return m_state_spaces.at(get_problem_index(vertex)); }

const StateSpace& GeneralizedStateSpaceImpl::get_state_space(const graphs::ClassEdge& edge) const { return m_state_spaces.at(get_problem_index(edge)); }

const graphs::ProblemVertex& GeneralizedStateSpaceImpl::get_problem_vertex(const graphs::ClassVertex& vertex) const
{
    return m_state_spaces.at(get_problem_index(vertex))->get_graph().get_vertex(get_problem_vertex_index(vertex));
}

const graphs::ProblemEdge& GeneralizedStateSpaceImpl::get_problem_edge(const graphs::ClassEdge& edge) const
{
    return m_state_spaces.at(get_problem_index(edge))->get_graph().get_edge(get_problem_edge_index(edge));
}

const graphs::ClassVertex& GeneralizedStateSpaceImpl::get_class_vertex(const graphs::ProblemVertex& vertex) const
{
    return get_graph().get_vertex(m_vertex_mappings.at(graphs::get_problem(vertex).get()).at(vertex.get_index()));
}

const graphs::ClassEdge& GeneralizedStateSpaceImpl::get_class_edge(const graphs::ProblemEdge& edge) const
{
    return get_graph().get_edge(m_edge_mappings.at(graphs::get_problem(edge).get()).at(edge.get_index()));
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
        for (const auto& class_e : complete_graph.get_adjacent_edges<graphs::ForwardTag>(class_v_idx))
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

graphs::ClassGraph GeneralizedStateSpaceImpl::create_induced_subgraph_from_class_vertex_indices(const IndexList& class_vertex_indices) const
{
    auto unique_class_v_idxs = IndexSet(class_vertex_indices.begin(), class_vertex_indices.end());

    return create_induced_subspace_helper(unique_class_v_idxs, get_graph());
}

graphs::ClassGraph GeneralizedStateSpaceImpl::create_induced_subgraph_from_problem_indices(const IndexList& problem_indices) const
{
    /* Collect unique class vertices */
    auto unique_class_v_idxs = IndexSet {};

    for (const auto& problem_idx : problem_indices)
    {
        const auto& problem_graph = m_state_spaces.at(problem_idx)->get_graph();

        for (const auto& problem_vertex : problem_graph.get_vertices())
        {
            const auto& class_vertex = get_class_vertex(problem_vertex);

            unique_class_v_idxs.insert(class_vertex.get_index());
        }
    }

    return create_induced_subspace_helper(unique_class_v_idxs, get_graph());
}

}
