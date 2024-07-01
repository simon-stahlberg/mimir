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

#include "mimir/search/applicable_action_generators/lifted/consistency_graph.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/formalism/action.hpp"
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/factories.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/applicable_action_generators/lifted/assignment_set.hpp"

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
        VertexIDs vertex_partition;
        ObjectIDs object_partition;

        for (const auto& object : m_problem->get_objects())
        {
            const auto object_id = object->get_identifier();
            const auto vertex_id = VertexID { m_vertices.size() };
            auto vertex = Vertex(vertex_id, parameter_index, object_id);

            if (static_assignment_set.consistent_literals(static_conditions, vertex))
            {
                vertex_partition.push_back(vertex_id);
                object_partition.push_back(object_id);
                m_vertices.push_back(std::move(vertex));
            }
        }
        m_vertices_by_parameter_index.push_back(std::move(vertex_partition));
        m_objects_by_parameter_index.push_back(std::move(object_partition));
    }

    /* 2. Compute edges */

    for (size_t first_vertex_id = 0; first_vertex_id < m_vertices.size(); ++first_vertex_id)
    {
        for (size_t second_vertex_id = (first_vertex_id + 1); second_vertex_id < m_vertices.size(); ++second_vertex_id)
        {
            const auto& first_vertex = m_vertices.at(first_vertex_id);
            const auto& second_vertex = m_vertices.at(second_vertex_id);

            // Part 1 of definition of substitution consistency graph (Stahlberg-ecai2023): exclude I^\neq
            if (first_vertex.get_parameter_index() != second_vertex.get_parameter_index())
            {
                auto edge = Edge(Vertex(first_vertex_id, first_vertex.get_parameter_index(), first_vertex.get_object_id()),
                                 Vertex(second_vertex_id, second_vertex.get_parameter_index(), second_vertex.get_object_id()));

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
    m_universal_effects.reserve(action->get_universal_effects().size());
    for (const auto& universal_effect : action->get_universal_effects())
    {
        m_universal_effects.push_back(StaticConsistencyGraph(problem,
                                                             action->get_arity(),
                                                             action->get_arity() + universal_effect->get_arity(),
                                                             universal_effect->get_conditions<Static>(),
                                                             static_assignment_set));
    }
}

const StaticConsistencyGraph& Graphs::get_precondition_graph() const { return m_precondition; }

const std::vector<StaticConsistencyGraph>& Graphs::get_universal_effect_graphs() const { return m_universal_effects; }

std::ostream& operator<<(std::ostream& out, std::tuple<const StaticConsistencyGraph&, const PDDLFactories&> data)
{
    const auto& [graph, pddl_factories] = data;

    const auto create_node = [](const Vertex& vertex, const PDDLFactories& pddl_factories, std::ostream& out)
    {
        out << "  \"" << vertex.get_id() << "\" [label=\"#" << vertex.get_parameter_index() << " <- " << *pddl_factories.get_object(vertex.get_object_id())
            << "\"];\n";
    };

    const auto create_edge = [](const Edge& edge, std::ostream& out)
    { out << "  \"" << edge.get_src().get_id() << "\" -- \"" << edge.get_dst().get_id() << "\";\n"; };

    // Define the undirected graph
    out << "graph myGraph {\n";
    // Define the nodes
    for (const auto& vertex : graph.get_vertices())
    {
        create_node(vertex, pddl_factories, out);
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
