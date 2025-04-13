/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
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

#include "tuple_graph_factory.hpp"

#include "mimir/datasets/object_graph.hpp"
#include "mimir/datasets/state_space.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/graphs/algorithms/nauty.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/search_context.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/state_repository.hpp"

using namespace mimir::formalism;
using namespace mimir::search;

namespace mimir::datasets
{
class TupleGraphArityZeroComputation
{
private:
    const graphs::ProblemVertex& m_problem_vertex;
    const StateSpace& m_state_space;
    std::optional<CertificateMaps>& m_certificate_maps;

    graphs::StaticTupleGraph m_internal_tuple_graph;
    IndexGroupedVectorBuilder<const Index> m_v_idxs_grouped_by_distance;
    IndexGroupedVectorBuilder<const Index> m_problem_v_idxs_grouped_by_distance;

    Index m_root_v_idx;

    void compute_distance_zero_vertices()
    {
        m_v_idxs_grouped_by_distance.start_group();
        m_problem_v_idxs_grouped_by_distance.start_group();

        m_root_v_idx = m_internal_tuple_graph.add_vertex(search::iw::AtomIndexList {}, IndexList { m_problem_vertex.get_index() });

        m_v_idxs_grouped_by_distance.add_group_element(m_root_v_idx);
    }

    void compute_distance_one_vertices_without_symmetry_reduction()
    {
        m_v_idxs_grouped_by_distance.start_group();
        m_problem_v_idxs_grouped_by_distance.start_group();

        for (const auto& adj_problem_vertex : m_state_space->get_graph().get_adjacent_vertices<graphs::ForwardTag>(m_problem_vertex.get_index()))
        {
            if (adj_problem_vertex.get_index() == m_problem_vertex.get_index())
            {
                continue;  ///< self-looping edge
            }

            const auto adj_v_idx = m_internal_tuple_graph.add_vertex(search::iw::AtomIndexList {}, IndexList { adj_problem_vertex.get_index() });
            m_internal_tuple_graph.add_directed_edge(m_root_v_idx, adj_v_idx);

            m_v_idxs_grouped_by_distance.add_group_element(adj_v_idx);
            m_problem_v_idxs_grouped_by_distance.add_group_element(adj_problem_vertex.get_index());
        }
    }

    void compute_distance_one_vertices_with_symmetry_reduction()
    {
        m_v_idxs_grouped_by_distance.start_group();
        m_problem_v_idxs_grouped_by_distance.start_group();

        const auto root_state = graphs::get_state(m_problem_vertex);
        auto& applicable_action_generator = *m_state_space->get_search_context()->get_applicable_action_generator();
        auto& state_repository = *m_state_space->get_search_context()->get_state_repository();
        const auto& problem = *m_state_space->get_search_context()->get_problem();

        auto novel_problem_v_idxs = graphs::VertexIndexSet {};
        novel_problem_v_idxs.insert(m_problem_vertex.get_index());

        for (const auto& action : applicable_action_generator.create_applicable_action_generator(root_state))
        {
            const auto [successor_state, successor_state_metric] =
                state_repository.get_or_create_successor_state(root_state, action, compute_state_metric_value(root_state, problem));

            if (!m_certificate_maps->state_to_cert.contains(successor_state))
            {
                m_certificate_maps->state_to_cert.emplace(successor_state,
                                                          graphs::nauty::SparseGraph(create_object_graph(successor_state, problem)).canonize());
            }

            const auto problem_v_idx = m_certificate_maps->cert_to_v_idx.at(m_certificate_maps->state_to_cert.at(successor_state));

            if (!novel_problem_v_idxs.contains(problem_v_idx))
            {
                novel_problem_v_idxs.insert(problem_v_idx);

                const auto v_idx = m_internal_tuple_graph.add_vertex(search::iw::AtomIndexList {}, IndexList { problem_v_idx });
                m_internal_tuple_graph.add_directed_edge(m_root_v_idx, v_idx);

                m_v_idxs_grouped_by_distance.add_group_element(v_idx);
                m_problem_v_idxs_grouped_by_distance.add_group_element(problem_v_idx);
            }
        }
    }

public:
    TupleGraphArityZeroComputation(const graphs::ProblemVertex& problem_vertex,
                                   const StateSpace& state_space,
                                   std::optional<CertificateMaps>& certificate_maps) :
        m_problem_vertex(problem_vertex),
        m_state_space(state_space),
        m_certificate_maps(certificate_maps),
        m_internal_tuple_graph(),
        m_v_idxs_grouped_by_distance(),
        m_problem_v_idxs_grouped_by_distance()
    {
    }

    TupleGraph compute_and_get_result()
    {
        compute_distance_zero_vertices();

        (m_certificate_maps) ? compute_distance_one_vertices_with_symmetry_reduction() : compute_distance_one_vertices_without_symmetry_reduction();

        return std::make_shared<TupleGraphImpl>(m_state_space,
                                                graphs::InternalTupleGraph(std::move(m_internal_tuple_graph)),
                                                m_v_idxs_grouped_by_distance.get_result(),
                                                m_problem_v_idxs_grouped_by_distance.get_result());
    }
};

static TupleGraph
create_tuple_graph_width_zero(const graphs::ProblemVertex& problem_vertex, const StateSpace& state_space, std::optional<CertificateMaps>& certificate_maps)
{
    return TupleGraphArityZeroComputation(problem_vertex, state_space, certificate_maps).compute_and_get_result();
}

class TupleGraphArityGreaterZeroComputation
{
private:
    const graphs::ProblemVertex& m_problem_vertex;
    const StateSpace& m_state_space;
    std::optional<CertificateMaps>& m_certificate_maps;
    const TupleGraphImpl::Options& m_options;

    graphs::StaticTupleGraph m_internal_tuple_graph;
    IndexGroupedVectorBuilder<const Index> m_v_idxs_grouped_by_distance;
    IndexGroupedVectorBuilder<const Index> m_problem_v_idxs_grouped_by_distance;

    // Common book-keeping
    IndexList m_prev_problem_v_idxs;
    IndexList m_curr_problem_v_idxs;
    IndexSet m_visited_problem_v_idxs;
    StateSet m_prev_states;
    StateSet m_curr_states;
    StateSet m_visited_states;
    IndexList m_prev_v_idxs;
    IndexList m_curr_v_idxs;
    iw::DynamicNoveltyTable m_novelty_table;

    // Map tuples to an index that avoids invalidation of tuple indices upon resizing of the DynamicNoveltyTable.
    std::unordered_map<IndexList, Index, loki::Hash<IndexList>, loki::EqualTo<IndexList>> m_tuple_to_index;
    IndexMap<IndexList> m_index_to_tuple;
    std::vector<IndexList> m_novel_tuples_vec;
    IndexSet m_novel_t_idxs_set;
    IndexMap<IndexSet> m_novel_t_idx_to_problem_v_idxs;   ///< one-to-one mapping
    IndexMap<IndexList> m_problem_v_idx_to_novel_t_idxs;  ///< one-to-many mapping

    IndexMap<IndexSet> m_cur_novel_t_idx_to_extended_problem_v_idx;
    IndexMap<IndexSet> m_cur_extended_novel_t_idx_to_prev_problem_v_idxs;
    IndexSet m_cur_extended_novel_t_idxs_set;
    IndexList m_cur_extended_novel_t_idxs_vec;

    IndexMap<IndexSet> m_t_idx_to_dominating_t_idxs;

    void compute_distance_zero_vertices()
    {
        m_v_idxs_grouped_by_distance.start_group();
        m_problem_v_idxs_grouped_by_distance.start_group();

        const auto root_state = get_state(m_problem_vertex);
        const auto root_problem_v_idx = m_problem_vertex.get_index();

        m_novelty_table.compute_novel_tuples(root_state, m_novel_tuples_vec);
        for (const auto& novel_tuple : m_novel_tuples_vec)
        {
            const auto it = m_tuple_to_index.find(novel_tuple);
            auto tuple_index = Index();
            if (it != m_tuple_to_index.end())
            {
                tuple_index = it->second;
            }
            else
            {
                tuple_index = m_tuple_to_index.size();
                m_tuple_to_index.emplace(novel_tuple, tuple_index);
                m_index_to_tuple.emplace(tuple_index, novel_tuple);
            }

            const auto v_idx = m_internal_tuple_graph.add_vertex(novel_tuple, IndexList { root_problem_v_idx });

            m_curr_v_idxs.push_back(v_idx);
            m_v_idxs_grouped_by_distance.add_group_element(v_idx);

            if (m_options.enable_dominance_pruning)
            {
                // Break after the first tuple
                break;
            }
        }
        m_novelty_table.test_novelty_and_update_table(root_state);

        m_problem_v_idxs_grouped_by_distance.add_group_element(root_problem_v_idx);
        m_curr_states.insert(root_state);
        m_visited_states.insert(root_state);
        m_curr_problem_v_idxs.clear();
        m_curr_problem_v_idxs.push_back(root_problem_v_idx);
        m_visited_problem_v_idxs.insert(root_problem_v_idx);
    }

    bool compute_next_state_layer()
    {
        bool success = false;

        for (const auto& prev_problem_v_idx : m_prev_problem_v_idxs)
        {
            for (const auto& curr_problem_v_idx : m_state_space->get_graph().get_adjacent_vertex_indices<graphs::ForwardTag>(prev_problem_v_idx))
            {
                if (!m_visited_problem_v_idxs.contains(curr_problem_v_idx))
                {
                    m_curr_problem_v_idxs.push_back(curr_problem_v_idx);
                    m_visited_problem_v_idxs.insert(curr_problem_v_idx);
                    success = true;
                }
            }
        }

        /* Compute all states in next layer, even those that are symmetric, to compute all tuple indices. */
        for (const auto& prev_state : m_prev_states)
        {
            for (const auto& action : m_state_space->get_search_context()->get_applicable_action_generator()->create_applicable_action_generator(prev_state))
            {
                const auto [successor_state, successor_state_metric_value] =
                    m_state_space->get_search_context()->get_state_repository()->get_or_create_successor_state(prev_state, action, 0);
                if (!m_visited_states.contains(successor_state))
                {
                    m_curr_states.insert(successor_state);
                    m_visited_states.insert(successor_state);
                }
            }
        }

        return success;
    }

    bool compute_next_layer()
    {
        // Swap prev and curr data structures.
        std::swap(m_curr_problem_v_idxs, m_prev_problem_v_idxs);
        std::swap(m_curr_states, m_prev_states);
        std::swap(m_curr_v_idxs, m_prev_v_idxs);
        m_curr_problem_v_idxs.clear();
        m_curr_states.clear();
        m_curr_v_idxs.clear();

        {
            const auto success = compute_next_state_layer();

            if (!success)
            {
                return false;
            }
        }

        {
            (m_certificate_maps) ? compute_next_novel_tuple_indices_with_symmetry_reduction() : compute_next_novel_tuple_indices_without_symmetry_reduction();

            if (m_novel_t_idxs_set.empty())
            {
                return false;
            }
        }

        {
            extend_optimal_plans_from_prev_layer();

            if (m_cur_extended_novel_t_idxs_set.empty())
            {
                return false;
            }
        }

        {
            const auto success = instantiate_next_layer();

            if (!success)
            {
                return false;
            }
        }

        // Create nonempty group.
        m_problem_v_idxs_grouped_by_distance.start_group();
        for (const auto& problem_v_idx : m_curr_problem_v_idxs)
        {
            m_problem_v_idxs_grouped_by_distance.add_group_element(problem_v_idx);
        }
        m_v_idxs_grouped_by_distance.start_group();
        for (const auto& v_idx : m_curr_v_idxs)
        {
            m_v_idxs_grouped_by_distance.add_group_element(v_idx);
        }

        return true;
    }

    void compute_next_novel_tuple_indices_without_symmetry_reduction()
    {
        // Clear data structures
        m_novel_t_idxs_set.clear();
        m_novel_t_idx_to_problem_v_idxs.clear();
        m_problem_v_idx_to_novel_t_idxs.clear();

        for (const auto& problem_v_idx : m_curr_problem_v_idxs)
        {
            m_novelty_table.compute_novel_tuples(get_state(m_state_space->get_graph().get_vertex(problem_v_idx)), m_novel_tuples_vec);

            for (const auto& novel_tuple : m_novel_tuples_vec)
            {
                const auto it = m_tuple_to_index.find(novel_tuple);
                auto novel_t_idx = Index();
                if (it != m_tuple_to_index.end())
                {
                    novel_t_idx = it->second;
                }
                else
                {
                    novel_t_idx = m_tuple_to_index.size();
                    m_tuple_to_index.emplace(novel_tuple, novel_t_idx);
                    m_index_to_tuple.emplace(novel_t_idx, novel_tuple);
                }

                m_novel_t_idx_to_problem_v_idxs[novel_t_idx].insert(problem_v_idx);
                m_problem_v_idx_to_novel_t_idxs[problem_v_idx].push_back(novel_t_idx);
                m_novel_t_idxs_set.insert(novel_t_idx);
            }
        }

        /* Ensure that tuples of all states in layer are marked as not novel! */
        for (const auto& state : m_curr_states)
        {
            m_novelty_table.test_novelty_and_update_table(state);
        }
    }

    void compute_next_novel_tuple_indices_with_symmetry_reduction()
    {
        // Clear data structures
        m_novel_t_idxs_set.clear();
        m_novel_t_idx_to_problem_v_idxs.clear();
        m_problem_v_idx_to_novel_t_idxs.clear();

        const auto& problem = *m_state_space->get_search_context()->get_problem();

        for (const auto& state : m_curr_states)
        {
            m_novelty_table.compute_novel_tuples(state, m_novel_tuples_vec);

            /* Map state to representative vertex. */
            if (!m_certificate_maps->state_to_cert.contains(state))
            {
                m_certificate_maps->state_to_cert.emplace(state, graphs::nauty::SparseGraph(create_object_graph(state, problem)).canonize());
            }

            const auto problem_v_idx = m_certificate_maps->cert_to_v_idx.at(m_certificate_maps->state_to_cert.at(state));

            /* Map novel tuples between representative vertex. */
            for (const auto& novel_tuple : m_novel_tuples_vec)
            {
                const auto it = m_tuple_to_index.find(novel_tuple);
                auto novel_t_idx = Index();
                if (it != m_tuple_to_index.end())
                {
                    novel_t_idx = it->second;
                }
                else
                {
                    novel_t_idx = m_tuple_to_index.size();
                    m_tuple_to_index.emplace(novel_tuple, novel_t_idx);
                    m_index_to_tuple.emplace(novel_t_idx, novel_tuple);
                }

                m_novel_t_idx_to_problem_v_idxs[novel_t_idx].insert(problem_v_idx);
                m_problem_v_idx_to_novel_t_idxs[problem_v_idx].push_back(novel_t_idx);
                m_novel_t_idxs_set.insert(novel_t_idx);
            }
        }

        /* Ensure that tuples of all states in layer are marked as not novel! */
        for (const auto& state : m_curr_states)
        {
            m_novelty_table.test_novelty_and_update_table(state);
        }
    }

    void extend_optimal_plans_from_prev_layer()
    {
        // Clear data structures
        m_cur_extended_novel_t_idxs_set.clear();
        m_cur_extended_novel_t_idxs_vec.clear();
        m_cur_extended_novel_t_idx_to_prev_problem_v_idxs.clear();

        // Part 2 of definition of width: Check whether all optimal plans for tuple t_{i-1}
        // can be extended into an optimal plan for tuple t_i by means of a single action.
        for (const auto& prev_v_idx : m_prev_v_idxs)
        {
            m_cur_novel_t_idx_to_extended_problem_v_idx.clear();

            // Bookkeeping..
            for (const auto prev_problem_v_idx : get_problem_vertices(m_internal_tuple_graph.get_vertex(prev_v_idx)))
            {
                // "[...] by means of a single action".
                for (const auto& curr_problem_v_idx : m_state_space->get_graph().get_adjacent_vertex_indices<graphs::ForwardTag>(prev_problem_v_idx))
                {
                    if (m_problem_v_idx_to_novel_t_idxs.contains(curr_problem_v_idx))
                    {
                        for (const auto target_tuple_index : m_problem_v_idx_to_novel_t_idxs.at(curr_problem_v_idx))
                        {
                            m_cur_novel_t_idx_to_extended_problem_v_idx[target_tuple_index].insert(prev_problem_v_idx);
                        }
                    }
                }
            }

            // "Check whether all optimal plans for tuple t_{i-1}
            // can be extended into an optimal plan for tuple t_i [...]""
            for (const auto& [cur_novel_t_idx, extended_states] : m_cur_novel_t_idx_to_extended_problem_v_idx)
            {
                bool all_optimal_plans_extended = (extended_states.size() == get_problem_vertices(m_internal_tuple_graph.get_vertex(prev_v_idx)).size());

                if (all_optimal_plans_extended)
                {
                    m_cur_extended_novel_t_idxs_set.insert(cur_novel_t_idx);
                    m_cur_extended_novel_t_idx_to_prev_problem_v_idxs[cur_novel_t_idx].insert(prev_v_idx);
                }
            }
        }
        m_cur_extended_novel_t_idxs_vec.insert(m_cur_extended_novel_t_idxs_vec.end(),
                                               m_cur_extended_novel_t_idxs_set.begin(),
                                               m_cur_extended_novel_t_idxs_set.end());
    }

    bool instantiate_next_layer()
    {  // Clear data structures
        m_t_idx_to_dominating_t_idxs.clear();

        if (m_options.enable_dominance_pruning)
        {
            for (size_t i = 0; i < m_cur_extended_novel_t_idxs_vec.size(); ++i)
            {
                const auto tuple_index_1 = m_cur_extended_novel_t_idxs_vec.at(i);

                const auto& problem_v_idxs_1 = m_novel_t_idx_to_problem_v_idxs.at(tuple_index_1);

                for (size_t j = 0; j < m_cur_extended_novel_t_idxs_vec.size(); ++j)
                {
                    const auto tuple_index_2 = m_cur_extended_novel_t_idxs_vec.at(j);

                    const auto& problem_v_idxs_2 = m_novel_t_idx_to_problem_v_idxs.at(tuple_index_2);

                    if (i < j && problem_v_idxs_1 == problem_v_idxs_2)
                    {
                        // Keep smallest tuple_index with a specific set of underlying states.
                        m_cur_extended_novel_t_idxs_set.erase(tuple_index_2);
                        continue;
                    }

                    if (problem_v_idxs_1.size() == problem_v_idxs_2.size())
                    {
                        continue;  ///< not a strict subset!
                    }

                    const auto is_subset = std::all_of(problem_v_idxs_2.begin(),
                                                       problem_v_idxs_2.end(),
                                                       [&problem_v_idxs_1](const auto& element) { return problem_v_idxs_1.count(element); });
                    if (is_subset)
                    {
                        // tuple_index_1 is dominated by tuple_index_2 because problem_v_idxs_2 < problem_v_idxs_1.
                        m_t_idx_to_dominating_t_idxs[tuple_index_2].insert(tuple_index_1);
                    }
                }
            }

            // Keep only tuple indices whose underlying states is a smallest subset.
            for (const auto& [dominating_tuple_index, dominated_tuple_indices] : m_t_idx_to_dominating_t_idxs)
            {
                for (const auto& dominated_tuple_index : dominated_tuple_indices)
                {
                    m_cur_extended_novel_t_idxs_set.erase(dominated_tuple_index);
                }
            }
        }

        if (m_cur_extended_novel_t_idxs_set.size() == 0)
        {
            return false;
        }

        for (const auto& t_idx : m_cur_extended_novel_t_idxs_set)
        {
            const auto& cur_problem_v_idxs = m_novel_t_idx_to_problem_v_idxs.at(t_idx);

            const auto cur_v_idx =
                m_internal_tuple_graph.add_vertex(m_index_to_tuple.at(t_idx), IndexList(cur_problem_v_idxs.begin(), cur_problem_v_idxs.end()));

            m_curr_v_idxs.push_back(cur_v_idx);

            for (const auto prev_v_idx : m_cur_extended_novel_t_idx_to_prev_problem_v_idxs.at(t_idx))  // .at?
            {
                m_internal_tuple_graph.add_directed_edge(prev_v_idx, cur_v_idx);
            }
        }

        return true;
    }

public:
    TupleGraphArityGreaterZeroComputation(const graphs::ProblemVertex& problem_vertex,
                                          const StateSpace& state_space,
                                          std::optional<CertificateMaps>& certificate_maps,
                                          const TupleGraphImpl::Options& options) :
        m_problem_vertex(problem_vertex),
        m_state_space(state_space),
        m_certificate_maps(certificate_maps),
        m_options(options),
        m_internal_tuple_graph(),
        m_v_idxs_grouped_by_distance(),
        m_problem_v_idxs_grouped_by_distance(),
        m_prev_problem_v_idxs(),
        m_curr_problem_v_idxs(),
        m_visited_problem_v_idxs(),
        m_prev_v_idxs(),
        m_curr_v_idxs(),
        m_novelty_table(options.width),
        m_novel_t_idxs_set(),
        m_novel_t_idx_to_problem_v_idxs(),
        m_problem_v_idx_to_novel_t_idxs(),
        m_cur_novel_t_idx_to_extended_problem_v_idx(),
        m_cur_extended_novel_t_idx_to_prev_problem_v_idxs(),
        m_cur_extended_novel_t_idxs_set(),
        m_cur_extended_novel_t_idxs_vec(),
        m_t_idx_to_dominating_t_idxs()
    {
    }

    TupleGraph compute_and_get_result()
    {
        compute_distance_zero_vertices();

        while (true)
        {
            auto success = compute_next_layer();

            if (!success)
            {
                break;
            }
        }

        return std::make_shared<TupleGraphImpl>(m_state_space,
                                                graphs::InternalTupleGraph(std::move(m_internal_tuple_graph)),
                                                m_v_idxs_grouped_by_distance.get_result(),
                                                m_problem_v_idxs_grouped_by_distance.get_result());
    }
};

static TupleGraph create_tuple_graph_width_greater_zero(const graphs::ProblemVertex& problem_vertex,
                                                        const StateSpace& state_space,
                                                        std::optional<CertificateMaps>& certificate_maps,
                                                        const TupleGraphImpl::Options& options)
{
    return TupleGraphArityGreaterZeroComputation(problem_vertex, state_space, certificate_maps, options).compute_and_get_result();
}

TupleGraph create_tuple_graph(const graphs::ProblemVertex& problem_vertex,
                              const StateSpace& state_space,
                              std::optional<CertificateMaps>& certificate_maps,
                              const TupleGraphImpl::Options& options)
{
    return (options.width == 0) ? create_tuple_graph_width_zero(problem_vertex, state_space, certificate_maps) :
                                  create_tuple_graph_width_greater_zero(problem_vertex, state_space, certificate_maps, options);
}
}
