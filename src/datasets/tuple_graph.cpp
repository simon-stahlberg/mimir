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

#include "mimir/datasets/tuple_graph.hpp"

namespace mimir
{
TupleGraph::TupleGraph(const ProblemGraph& problem_graph,
                       const TupleIndexMapper& index_mapper,
                       InternalTupleGraph graph,
                       IndexGroupedVector<const Index> vertices_grouped_by_distance,
                       IndexGroupedVector<const Index> problem_vertices_grouped_by_distance) :
    m_problem_graph(problem_graph),
    m_index_mapper(index_mapper),
    m_graph(std::move(graph)),
    m_vertices_grouped_by_distance(std::move(vertices_grouped_by_distance)),
    m_problem_vertices_grouped_by_distance(std::move(problem_vertices_grouped_by_distance))
{
}

const ProblemGraph& TupleGraph::get_problem_graph() const { return m_problem_graph; }

const TupleIndexMapper& TupleGraph::get_index_mapper() const { return m_index_mapper; }

const InternalTupleGraph& TupleGraph::get_graph() const { return m_graph; }

const IndexGroupedVector<const Index>& TupleGraph::get_vertices_grouped_by_distance() const { return m_vertices_grouped_by_distance; }

const IndexGroupedVector<const Index>& TupleGraph::get_problem_vertices_grouped_by_distance() const { return m_problem_vertices_grouped_by_distance; }

TupleGraphCollection::TupleGraphCollection(const GeneralizedStateSpace& state_space, const Options& options) {}

const TupleGraphList& TupleGraphCollection::get_per_class_vertex_tuple_graph() const { return m_per_class_vertex_tuple_graph; }
}
