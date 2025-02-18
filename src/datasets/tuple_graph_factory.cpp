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
static TupleGraph create_tuple_graph_width_zero(const ProblemVertex& problem_vertex,
                                                const ProblemGraph& graph,
                                                const ProblemContext& context,
                                                const TupleIndexMapper& index_mapper,
                                                const TupleGraphCollection::Options& options)
{
    auto internal_tuple_graph = StaticTupleGraph();
    auto vertices_grouped_by_distance = IndexGroupedVectorBuilder<const Index>();
    auto problem_vertices_grouped_by_distance = IndexGroupedVectorBuilder<const Index>();

    const auto empty_t_idx = index_mapper.get_empty_tuple_index();  ///< we use empty tuple in all vertices

    /* Root layer */

    vertices_grouped_by_distance.start_group();
    problem_vertices_grouped_by_distance.start_group();

    const auto root_v_idx = internal_tuple_graph.add_vertex(empty_t_idx, IndexList { problem_vertex.get_index() });

    vertices_grouped_by_distance.add_group_element(root_v_idx);
    problem_vertices_grouped_by_distance.add_group_element(problem_vertex.get_index());

    /* Distance 1 layer */

    vertices_grouped_by_distance.start_group();
    problem_vertices_grouped_by_distance.start_group();

    for (const auto& adj_problem_vertex : graph.get_adjacent_vertices<ForwardTraversal>(problem_vertex.get_index()))
    {
        if (adj_problem_vertex.get_index() == problem_vertex.get_index())
        {
            continue;  ///< self-looping edge
        }

        const auto adj_v_idx = internal_tuple_graph.add_vertex(empty_t_idx, IndexList { adj_problem_vertex.get_index() });
        internal_tuple_graph.add_directed_edge(root_v_idx, adj_v_idx);

        vertices_grouped_by_distance.add_group_element(adj_v_idx);
        problem_vertices_grouped_by_distance.add_group_element(adj_problem_vertex.get_index());
    }

    return TupleGraph(context,
                      graph,
                      index_mapper,
                      InternalTupleGraph(std::move(internal_tuple_graph)),
                      vertices_grouped_by_distance.get_result(),
                      problem_vertices_grouped_by_distance.get_result());
}

static TupleGraph create_tuple_graph_width_greater_zero(const ProblemVertex& problem_vertex,
                                                        const ProblemGraph& graph,
                                                        const ProblemContext& context,
                                                        const TupleIndexMapper& index_mapper,
                                                        const TupleGraphCollection::Options& options)
{
    IndexGroupedVectorBuilder<const TupleGraphVertex> m_vertices_grouped_by_distance;
    IndexGroupedVectorBuilder<const State> m_states_grouped_by_distance;

    // Common book-keeping
    StateList prev_states;
    StateList curr_states;
    TupleGraphVertexList prev_vertices;
    TupleGraphVertexList curr_vertices;
    StateSet visited_states;
    auto novelty_table = DynamicNoveltyTable(index_mapper);
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
