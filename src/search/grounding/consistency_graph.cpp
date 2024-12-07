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

#include "mimir/search/grounding/consistency_graph.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/formalism/action.hpp"
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/grounding/assignment_set.hpp"

#include <cstdint>

namespace mimir::consistency_graph
{
// To print vectors as [e1,...,en]
using mimir::operator<<;

StaticConsistencyGraph::StaticConsistencyGraph(Problem problem,
                                               size_t begin_parameter_index,
                                               size_t end_parameter_index,
                                               const LiteralList<Static>& static_conditions,
                                               const AssignmentSet<Static>& static_assignment_set) :
    m_problem(problem)
{
    /* 1. Compute vertices */

    for (uint32_t parameter_index = begin_parameter_index; parameter_index < end_parameter_index; ++parameter_index)
    {
        VertexIndexList vertex_partition;
        ObjectIndexList object_partition;

        for (const auto& object : m_problem->get_objects())
        {
            const auto object_index = object->get_index();
            const auto vertex_index = VertexIndex { m_vertices.size() };
            auto vertex = Vertex(vertex_index, parameter_index, object_index);

            if (static_assignment_set.consistent_literals(static_conditions, vertex))
            {
                vertex_partition.push_back(vertex_index);
                object_partition.push_back(object_index);
                m_vertices.push_back(std::move(vertex));
            }
        }
        m_vertices_by_parameter_index.push_back(std::move(vertex_partition));
        m_objects_by_parameter_index.push_back(std::move(object_partition));
    }

    /* 2. Compute edges */

    for (size_t first_vertex_index = 0; first_vertex_index < m_vertices.size(); ++first_vertex_index)
    {
        for (size_t second_vertex_index = (first_vertex_index + 1); second_vertex_index < m_vertices.size(); ++second_vertex_index)
        {
            const auto& first_vertex = m_vertices.at(first_vertex_index);
            const auto& second_vertex = m_vertices.at(second_vertex_index);

            // Part 1 of definition of substitution consistency graph (Stahlberg-ecai2023): exclude I^\neq
            if (first_vertex.get_parameter_index() != second_vertex.get_parameter_index())
            {
                auto edge = Edge(Vertex(first_vertex_index, first_vertex.get_parameter_index(), first_vertex.get_object_index()),
                                 Vertex(second_vertex_index, second_vertex.get_parameter_index(), second_vertex.get_object_index()));

                if (static_assignment_set.consistent_literals(static_conditions, edge))
                {
                    m_edges.push_back(std::move(edge));
                }
            }
        }
    }
}

Graphs::Graphs(Problem problem, Action action, const AssignmentSet<Static>& static_assignment_set) :
    m_precondition(StaticConsistencyGraph(problem, 0, action->get_arity(), action->get_conditions<Static>(), static_assignment_set))
{
    m_conditional_effects.reserve(action->get_conditional_effects().size());
    for (const auto& conditional_effect : action->get_conditional_effects())
    {
        m_conditional_effects.push_back(StaticConsistencyGraph(problem,
                                                               action->get_arity(),
                                                               action->get_arity() + conditional_effect->get_arity(),
                                                               conditional_effect->get_conditions<Static>(),
                                                               static_assignment_set));
    }
}

const StaticConsistencyGraph& Graphs::get_precondition_graph() const { return m_precondition; }

const std::vector<StaticConsistencyGraph>& Graphs::get_conditional_effect_graphs() const { return m_conditional_effects; }
}

namespace mimir
{
template<>
std::ostream& operator<<(std::ostream& out, const std::tuple<const consistency_graph::StaticConsistencyGraph&, const PDDLRepositories&>& data)
{
    const auto& [graph, pddl_repositories] = data;

    const auto create_node = [](const consistency_graph::Vertex& vertex, const PDDLRepositories& pddl_repositories, std::ostream& out)
    {
        out << "  \"" << vertex.get_index() << "\" [label=\"#" << vertex.get_parameter_index() << " <- "
            << *pddl_repositories.get_object(vertex.get_object_index()) << "\"];\n";
    };

    const auto create_edge = [](const consistency_graph::Edge& edge, std::ostream& out)
    { out << "  \"" << edge.get_src().get_index() << "\" -- \"" << edge.get_dst().get_index() << "\";\n"; };

    // Define the undirected graph
    out << "graph myGraph {\n";
    // Define the nodes
    for (const auto& vertex : graph.get_vertices())
    {
        create_node(vertex, pddl_repositories, out);
    }
    // Define the edges
    for (const auto& edge : graph.get_edges())
    {
        create_edge(edge, out);
    }
    out << "}\n";
    return out;
}

}
