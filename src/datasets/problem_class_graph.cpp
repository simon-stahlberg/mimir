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
#include "mimir/formalism/ground_action.hpp"

namespace mimir::datasets
{
using CertificateToIndex =
    std::unordered_map<nauty_wrapper::Certificate, Index, loki::Hash<nauty_wrapper::Certificate>, loki::EqualTo<nauty_wrapper::Certificate>>;

static std::optional<std::pair<StaticProblemGraph, CertificateToIndex>> compute_static_problem_graph(const ProblemContext& context,
                                                                                                     const ProblemOptions& options)
{
}

static std::vector<std::pair<StaticProblemGraph, CertificateToIndex>> compute_static_problem_graphs(const ProblemContextList& contexts,
                                                                                                    const ProblemOptions& options)
{
    auto problem_graphs = std::vector<std::pair<StaticProblemGraph, CertificateToIndex>> {};
    for (const auto& context : contexts)
    {
        auto problem_graph = compute_static_problem_graph(context, options);

        if (!problem_graph)
        {
            continue;
        }

        problem_graphs.push_back(std::move(problem_graph.value()));
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
