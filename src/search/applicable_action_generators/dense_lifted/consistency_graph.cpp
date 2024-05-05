#include "mimir/search/applicable_action_generators/dense_lifted/consistency_graph.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/formalism/factories.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted/assignment_set.hpp"

namespace mimir::consistency_graph
{
// To print vectors as [e1,...,en]
using mimir::operator<<;

StaticConsistencyGraph::StaticConsistencyGraph(Problem problem, size_t arity, const LiteralList& static_conditions) : m_problem(problem), m_arity(arity)
{
    /* 1. Compute vertices */

    auto static_initial_atoms = GroundAtomList {};
    to_ground_atoms(m_problem->get_static_initial_literals(), static_initial_atoms);
    const auto static_assignment_set = AssignmentSet(m_problem, static_initial_atoms);

    // Bookkeeping to test whether [x/o] satisfies static preconditions.
    // We use this data structure to avoid explicit grounding.
    auto unary_groundings = std::unordered_map<Predicate, std::unordered_set<Object>> {};
    for (const auto& literal : problem->get_static_initial_literals())
    {
        const auto& atom = literal->get_atom();
        const auto& predicate = atom->get_predicate();
        if (predicate->get_arity() == 1)
        {
            const auto& object = atom->get_objects().front();

            unary_groundings[predicate].insert(object);
        }
    }

    for (uint32_t parameter_index = 0; parameter_index < arity; ++parameter_index)
    {
        VertexIDs partition;

        for (const auto& object : m_problem->get_objects())
        {
            // Check whether [x/o] satisfies static preconditions
            // Adding this test results in a smaller consistency graph
            // and resulted in 20% performance increase on the tests.
            bool all_static_unary_preconditions_satisfied = true;
            for (const auto& literal : static_conditions)
            {
                if (literal->get_atom()->get_predicate()->get_arity() == 1)
                {
                    if (const auto term_variable = std::get_if<TermVariableImpl>(literal->get_atom()->get_terms().front()))
                    {
                        if (term_variable->get_variable()->get_parameter_index() == parameter_index)
                        {
                            // Check whether the unary ground atom is true in the initial state
                            if (!unary_groundings[literal->get_atom()->get_predicate()].count(object))
                            {
                                all_static_unary_preconditions_satisfied = false;
                            }
                        }
                    }
                }
            }
            if (all_static_unary_preconditions_satisfied)
            {
                // D: Partition [x/o] by x
                auto vertex_id = VertexID { m_vertices.size() };
                partition.push_back(vertex_id);
                // D: Create nodes [x/o]
                m_vertices.push_back(Vertex(vertex_id, parameter_index, object->get_identifier()));
            }
        }
        m_vertices_by_parameter_index.push_back(std::move(partition));
    }

    /* 2. Compute edges */

    for (size_t first_id = 0; first_id < m_vertices.size(); ++first_id)
    {
        for (size_t second_id = (first_id + 1); second_id < m_vertices.size(); ++second_id)
        {
            const auto& first_vertex = m_vertices.at(first_id);
            const auto& second_vertex = m_vertices.at(second_id);

            if (first_vertex.get_param_index() != second_vertex.get_param_index())
            {
                const auto edge = Edge(Vertex(first_id, first_vertex.get_param_index(), first_vertex.get_object_index()),
                                       Vertex(second_id, second_vertex.get_param_index(), second_vertex.get_object_index()));

                if (static_assignment_set.literal_all_consistent(static_conditions, edge))
                {
                    m_edges.push_back(edge);
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

Graphs::Graphs(Problem problem, Action action)
{
    m_precondition =
        action->get_arity() < 2 ? std::nullopt : std::make_optional(StaticConsistencyGraph(problem, action->get_arity(), action->get_static_conditions()));

    m_universal_effects.reserve(action->get_universal_effects().size());
    for (const auto& universal_effect : action->get_universal_effects())
    {
        universal_effect->get_arity() < 2 ? m_universal_effects.push_back(std::nullopt) :
                                            m_universal_effects.push_back(std::make_optional(
                                                StaticConsistencyGraph(problem, universal_effect->get_arity(), universal_effect->get_static_conditions())));
    }
}

const std::optional<StaticConsistencyGraph>& Graphs::get_precondition_graph() const { return m_precondition; }

const std::vector<std::optional<StaticConsistencyGraph>>& Graphs::get_universal_effect_graphs() const { return m_universal_effects; }

}
