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

namespace mimir
{
class TupleGraphArityZeroComputation
{
private:
    const ProblemVertex& m_problem_vertex;
    const ProblemGraph& m_graph;
    const ProblemContext& m_context;
    const TupleIndexMapper& m_index_mapper;
    const TupleGraphCollection::Options& m_options;

    StaticTupleGraph m_internal_tuple_graph;
    IndexGroupedVectorBuilder<const Index> m_vertices_grouped_by_distance;
    IndexGroupedVectorBuilder<const Index> m_problem_vertices_grouped_by_distance;

    Index m_root_v_idx;

    void compute_distance_zero_vertices()
    {
        const auto empty_t_idx = m_index_mapper.get_empty_tuple_index();  ///< we use empty tuple in all vertices

        m_vertices_grouped_by_distance.start_group();
        m_problem_vertices_grouped_by_distance.start_group();

        m_root_v_idx = m_internal_tuple_graph.add_vertex(empty_t_idx, IndexList { m_problem_vertex.get_index() });

        m_vertices_grouped_by_distance.add_group_element(m_root_v_idx);
    }

    void compute_distance_one_vertices()
    {
        const auto empty_t_idx = m_index_mapper.get_empty_tuple_index();  ///< we use empty tuple in all vertices

        m_vertices_grouped_by_distance.start_group();
        m_problem_vertices_grouped_by_distance.start_group();

        for (const auto& adj_problem_vertex : m_graph.get_adjacent_vertices<ForwardTraversal>(m_problem_vertex.get_index()))
        {
            if (adj_problem_vertex.get_index() == m_problem_vertex.get_index())
            {
                continue;  ///< self-looping edge
            }

            const auto adj_v_idx = m_internal_tuple_graph.add_vertex(empty_t_idx, IndexList { adj_problem_vertex.get_index() });
            m_internal_tuple_graph.add_directed_edge(m_root_v_idx, adj_v_idx);

            m_vertices_grouped_by_distance.add_group_element(adj_v_idx);
            m_problem_vertices_grouped_by_distance.add_group_element(adj_problem_vertex.get_index());
        }
    }

public:
    TupleGraphArityZeroComputation(const ProblemVertex& problem_vertex,
                                   const ProblemGraph& graph,
                                   const ProblemContext& context,
                                   const TupleIndexMapper& index_mapper,
                                   const TupleGraphCollection::Options& options) :
        m_problem_vertex(problem_vertex),
        m_graph(graph),
        m_context(context),
        m_index_mapper(index_mapper),
        m_options(options),
        m_internal_tuple_graph(),
        m_vertices_grouped_by_distance(),
        m_problem_vertices_grouped_by_distance()
    {
    }

    TupleGraph compute_and_get_result()
    {
        compute_distance_zero_vertices();

        compute_distance_one_vertices();

        return TupleGraph(m_context,
                          m_graph,
                          m_index_mapper,
                          InternalTupleGraph(std::move(m_internal_tuple_graph)),
                          m_vertices_grouped_by_distance.get_result(),
                          m_problem_vertices_grouped_by_distance.get_result());
    }
};

static TupleGraph create_tuple_graph_width_zero(const ProblemVertex& problem_vertex,
                                                const ProblemGraph& graph,
                                                const ProblemContext& context,
                                                const TupleIndexMapper& index_mapper,
                                                const TupleGraphCollection::Options& options)
{
    return TupleGraphArityZeroComputation(problem_vertex, graph, context, index_mapper, options).compute_and_get_result();
}

class TupleGraphArityGreaterZeroComputation
{
private:
    const ProblemVertex& m_problem_vertex;
    const ProblemGraph& m_graph;
    const ProblemContext& m_context;
    const TupleIndexMapper& m_index_mapper;
    const TupleGraphCollection::Options& m_options;

    StaticTupleGraph m_internal_tuple_graph;
    IndexGroupedVectorBuilder<const Index> m_vertices_grouped_by_distance;
    IndexGroupedVectorBuilder<const Index> m_problem_vertices_grouped_by_distance;

    // Common book-keeping
    IndexList prev_problem_v_idxs;
    IndexList curr_problem_v_idxs;
    IndexSet visited_problem_v_idxs;
    IndexList prev_v_idxs;
    IndexList curr_v_idxs;
    DynamicNoveltyTable novelty_table;  ///< it will never resize since the problem graph is complete.

    void compute_distance_zero_vertices()
    {
        m_vertices_grouped_by_distance.start_group();
        m_problem_vertices_grouped_by_distance.start_group();

        const auto root_state = get_state(m_problem_vertex);
        const auto root_problem_v_idx = m_problem_vertex.get_index();

        novelty_table.compute_novel_tuple_indices(root_state, novel_t_idx_vec);

        for (const auto& novel_tuple_index : novel_t_idx_vec)
        {
            const auto vertex_index = m_internal_tuple_graph.add_vertex(novel_tuple_index, IndexList { root_problem_v_idx });

            m_vertices_grouped_by_distance.add_group_element(vertex_index);

            if (m_options.enable_dominance_pruning)
            {
                // Break after the first tuple
                break;
            }
        }
        novelty_table.insert_tuple_indices(novel_t_idx_vec);

        m_problem_vertices_grouped_by_distance.add_group_element(root_problem_v_idx);
        curr_problem_v_idxs.clear();
        curr_problem_v_idxs.push_back(root_problem_v_idx);
        visited_problem_v_idxs.insert(root_problem_v_idx);
    }

    IndexSet novel_t_idx_set;
    IndexList novel_t_idx_vec;
    IndexMap<Index> novel_t_idx_to_problem_v_idx;       ///< one-to-one mapping
    IndexMap<IndexList> problem_v_idx_to_novel_t_idxs;  ///< one-to-many mapping

    IndexMap<IndexSet> cur_novel_t_idx_to_extended_problem_v_idx;
    IndexMap<IndexSet> cur_extended_novel_t_idx_to_prev_problem_v_idxs;
    IndexSet cur_extended_novel_t_idxs_set;
    IndexList cur_extended_novel_t_idxs_vec;

    IndexMap<IndexSet> t_idx_to_dominating_t_idxs;

public:
    TupleGraphArityGreaterZeroComputation(const ProblemVertex& problem_vertex,
                                          const ProblemGraph& graph,
                                          const ProblemContext& context,
                                          const TupleIndexMapper& index_mapper,
                                          const TupleGraphCollection::Options& options) :
        m_problem_vertex(problem_vertex),
        m_graph(graph),
        m_context(context),
        m_index_mapper(index_mapper),
        m_options(options),
        m_internal_tuple_graph(),
        m_vertices_grouped_by_distance(),
        m_problem_vertices_grouped_by_distance(),
        prev_problem_v_idxs(),
        curr_problem_v_idxs(),
        visited_problem_v_idxs(),
        prev_v_idxs(),
        curr_v_idxs(),
        novelty_table(m_index_mapper),
        novel_t_idx_set(),
        novel_t_idx_vec(),
        novel_t_idx_to_problem_v_idx(),
        problem_v_idx_to_novel_t_idxs(),
        cur_novel_t_idx_to_extended_problem_v_idx(),
        cur_extended_novel_t_idx_to_prev_problem_v_idxs(),
        cur_extended_novel_t_idxs_set(),
        cur_extended_novel_t_idxs_vec(),
        t_idx_to_dominating_t_idxs()
    {
    }

    TupleGraph compute_and_get_result()
    {
        compute_distance_zero_vertices();

        return TupleGraph(m_context,
                          m_graph,
                          m_index_mapper,
                          InternalTupleGraph(std::move(m_internal_tuple_graph)),
                          m_vertices_grouped_by_distance.get_result(),
                          m_problem_vertices_grouped_by_distance.get_result());
    }
};

static TupleGraph create_tuple_graph_width_greater_zero(const ProblemVertex& problem_vertex,
                                                        const ProblemGraph& graph,
                                                        const ProblemContext& context,
                                                        const TupleIndexMapper& index_mapper,
                                                        const TupleGraphCollection::Options& options)
{
    return TupleGraphArityGreaterZeroComputation(problem_vertex, graph, context, index_mapper, options).compute_and_get_result();
}

TupleGraph create_tuple_graph(const ProblemVertex& vertex,
                              const ProblemGraph& graph,
                              const ProblemContext& context,
                              const TupleIndexMapper& index_mapper,
                              const TupleGraphCollection::Options& options)
{
    return (options.width == 0) ? create_tuple_graph_width_zero(vertex, graph, context, index_mapper, options) :
                                  create_tuple_graph_width_greater_zero(vertex, graph, context, index_mapper, options);
}
}
