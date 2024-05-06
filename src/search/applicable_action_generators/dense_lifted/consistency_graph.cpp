#include "mimir/search/applicable_action_generators/dense_lifted/consistency_graph.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/formalism/factories.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted/assignment_set.hpp"

namespace mimir::consistency_graph
{
// To print vectors as [e1,...,en]
using mimir::operator<<;

StaticConsistencyGraph::StaticConsistencyGraph(Problem problem,
                                               size_t begin_parameter_index,
                                               size_t end_parameter_index,
                                               const LiteralList& static_conditions,
                                               const AssignmentSet& static_assignment_set) :
    m_problem(problem)
{
    /* 2. Compute vertices */

    for (uint32_t parameter_index = begin_parameter_index; parameter_index < end_parameter_index; ++parameter_index)
    {
        VertexIDs vertex_partition;
        ObjectIDs object_partition;

        for (const auto& object : m_problem->get_objects())
        {
            const auto object_id = object->get_identifier();
            const auto vertex_id = VertexID { m_vertices.size() };
            auto vertex = Vertex(vertex_id, parameter_index, object_id);

            if (static_assignment_set.literal_all_consistent(static_conditions, vertex))
            {
                vertex_partition.push_back(vertex_id);
                object_partition.push_back(object_id);
                m_vertices.push_back(std::move(vertex));
            }
        }
        m_vertices_by_parameter_index.push_back(std::move(vertex_partition));
        m_objects_by_parameter_index.push_back(std::move(object_partition));
    }

    /* 3. Compute edges */

    for (size_t first_vertex_id = 0; first_vertex_id < m_vertices.size(); ++first_vertex_id)
    {
        for (size_t second_vertex_id = (first_vertex_id + 1); second_vertex_id < m_vertices.size(); ++second_vertex_id)
        {
            const auto& first_vertex = m_vertices.at(first_vertex_id);
            const auto& second_vertex = m_vertices.at(second_vertex_id);

            if (first_vertex.get_param_index() != second_vertex.get_param_index())
            {
                auto edge = Edge(Vertex(first_vertex_id, first_vertex.get_param_index(), first_vertex.get_object_index()),
                                 Vertex(second_vertex_id, second_vertex.get_param_index(), second_vertex.get_object_index()));

                if (static_assignment_set.literal_all_consistent(static_conditions, edge))
                {
                    m_edges.push_back(std::move(edge));
                }
            }
        }
    }
}

std::ostream& operator<<(std::ostream& out, const StaticConsistencyGraph& graph)
{
    out << "StaticConsistencyGraph(\n"
        << "    vertices=" << graph.get_vertices() << "\n"
        << "    edges=" << graph.get_edges() << std::endl;
    return out;
}

Graphs::Graphs(Problem problem, Action action, const AssignmentSet& static_assignment_set)
{
    m_precondition = action->get_arity() < 2 ?
                         std::nullopt :
                         std::make_optional(StaticConsistencyGraph(problem, 0, action->get_arity(), action->get_static_conditions(), static_assignment_set));

    m_universal_effects.reserve(action->get_universal_effects().size());
    for (const auto& universal_effect : action->get_universal_effects())
    {
        m_universal_effects.push_back(StaticConsistencyGraph(problem,
                                                             action->get_arity(),
                                                             action->get_arity() + universal_effect->get_arity(),
                                                             universal_effect->get_static_conditions(),
                                                             static_assignment_set));
    }
}

const std::optional<StaticConsistencyGraph>& Graphs::get_precondition_graph() const { return m_precondition; }

const std::vector<StaticConsistencyGraph>& Graphs::get_universal_effect_graphs() const { return m_universal_effects; }

}
