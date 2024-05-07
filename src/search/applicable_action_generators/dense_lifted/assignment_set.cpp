#include "mimir/search/applicable_action_generators/dense_lifted/assignment_set.hpp"

#include "mimir/formalism/factories.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted/consistency_graph.hpp"

#include <cassert>

namespace mimir
{

size_t UnaryAssignmentSet::get_assignment_position(const UnaryAssignment& assignment, int32_t arity, int32_t num_objects)
{
    const auto position_factor = 1;
    const auto object_factor = position_factor * arity;
    return assignment.position * position_factor + assignment.object * object_factor;
}

size_t UnaryAssignmentSet::get_num_assignments(int32_t arity, int32_t num_objects) { return (size_t) arity * num_objects; }

UnaryAssignmentSet::UnaryAssignmentSet(Problem problem, const GroundAtomList& atoms) : m_problem(problem), m_f()
{
    const auto num_objects = problem->get_objects().size();
    const auto& predicates = problem->get_domain()->get_predicates();

    // D: Create assignment set for each predicate
    m_f.resize(predicates.size());
    for (const auto& predicate : predicates)
    {
        // Predicates must have indexing 0,1,2,... for this implementation
        assert(predicate->get_identifier() < static_cast<int>(predicates.size()));

        const auto arity = predicate->get_arity();

        if (arity != 1)
        {
            continue;
        }

        auto& assignment_set = m_f[predicate->get_identifier()];

        assignment_set.resize(UnaryAssignmentSet::get_num_assignments(arity, num_objects));
    }
    // D: Initialize m_f
    for (const auto& ground_atom : atoms)
    {
        const auto arity = ground_atom->get_arity();

        if (arity != 1)
        {
            continue;
        }

        const auto& predicate = ground_atom->get_predicate();
        const auto& arguments = ground_atom->get_objects();
        auto& assignment_set = m_f[predicate->get_identifier()];

        for (int32_t position = 0; position < static_cast<int32_t>(arity); ++position)
        {
            const auto& first_object = arguments[position];
            assignment_set[get_assignment_position(UnaryAssignment { position, first_object->get_identifier() }, arity, num_objects)] = true;
        }
    }
}

bool UnaryAssignmentSet::literal_all_consistent(const std::vector<Literal>& literals, const consistency_graph::Vertex& consistent_vertex) const
{
    for (const auto& literal : literals)
    {
        const auto arity = literal->get_atom()->get_predicate()->get_arity();

        if (arity != 1)
        {
            continue;
        }

        int32_t position = -1;
        int32_t object_id = -1;

        for (std::size_t index = 0; index < literal->get_atom()->get_predicate()->get_arity(); ++index)
        {
            const auto& term = literal->get_atom()->get_terms()[index];

            if (const auto term_object = std::get_if<TermObjectImpl>(term))
            {
                position = index;
                object_id = term_object->get_object()->get_identifier();
                break;
            }
            else if (const auto term_variable = std::get_if<TermVariableImpl>(term))
            {
                const auto parameter_index = term_variable->get_variable()->get_parameter_index();

                if (consistent_vertex.get_param_index() == parameter_index)
                {
                    position = index;
                    object_id = consistent_vertex.get_object_index();
                    break;
                }
            }
        }

        bool is_empty_assignment = (position == -1);

        if (!is_empty_assignment)
        {
            const auto& assignment_set = m_f[literal->get_atom()->get_predicate()->get_identifier()];
            const auto assignment_rank = UnaryAssignmentSet::get_assignment_position(UnaryAssignment { position, object_id },
                                                                                     static_cast<int32_t>(arity),
                                                                                     static_cast<int32_t>(m_problem->get_objects().size()));
            assert(assignment_rank < assignment_set.size());
            const auto consistent_with_state = assignment_set[assignment_rank];

            if (literal->is_negated() == consistent_with_state)
            {
                return false;
            }
        }
    }

    return true;
}

size_t BinaryAssignmentSet::get_assignment_position(const BinaryAssignment& assignment, int32_t arity, int32_t num_objects)
{
    const auto first_position_factor = 1;
    const auto second_position_factor = first_position_factor * arity;
    const auto first_object_factor = second_position_factor * arity;
    const auto second_object_factor = first_object_factor * num_objects;
    return (size_t) assignment.first_position * first_position_factor + assignment.second_position * second_position_factor
           + assignment.first_object * first_object_factor + assignment.second_object * second_object_factor;
}

size_t BinaryAssignmentSet::get_num_assignments(int32_t arity, int32_t num_objects) { return (size_t) arity * arity * num_objects * num_objects; }

BinaryAssignmentSet::BinaryAssignmentSet(Problem problem, const GroundAtomList& atoms) : m_problem(problem), m_f()
{
    const auto num_objects = problem->get_objects().size();
    const auto& predicates = problem->get_domain()->get_predicates();

    // D: Create assignment set for each predicate
    m_f.resize(predicates.size());
    for (const auto& predicate : predicates)
    {
        // Predicates must have indexing 0,1,2,... for this implementation
        assert(predicate->get_identifier() < static_cast<int>(predicates.size()));

        const auto arity = predicate->get_arity();

        if (arity != 2)
        {
            continue;
        }

        auto& assignment_set = m_f[predicate->get_identifier()];
        assignment_set.resize(BinaryAssignmentSet::get_num_assignments(predicate->get_arity(), num_objects));
    }
    // D: Initialize m_f
    for (const auto& ground_atom : atoms)
    {
        const auto& arity = ground_atom->get_arity();

        if (arity != 2)
        {
            continue;
        }

        const auto& predicate = ground_atom->get_predicate();
        const auto& arguments = ground_atom->get_objects();
        auto& assignment_set = m_f[predicate->get_identifier()];

        for (int32_t first_position = 0; first_position < static_cast<int32_t>(arity); ++first_position)
        {
            const auto& first_object = arguments[first_position];

            for (int32_t second_position = first_position + 1; second_position < static_cast<int32_t>(arity); ++second_position)
            {
                const auto& second_object = arguments[second_position];
                assignment_set[BinaryAssignmentSet::get_assignment_position(
                    BinaryAssignment { first_position, first_object->get_identifier(), second_position, second_object->get_identifier() },
                    arity,
                    num_objects)] = true;
            }
        }
    }
}

bool BinaryAssignmentSet::literal_all_consistent(const std::vector<Literal>& literals, const consistency_graph::Edge& consistent_edge) const
{
    for (const auto& literal : literals)
    {
        const auto arity = literal->get_atom()->get_predicate()->get_arity();

        if (arity != 2)
        {
            continue;
        }

        const auto fetch_assignment = [](int32_t& index, const TermList& terms, const consistency_graph::Edge& edge)
        {
            for (; index < static_cast<int32_t>(terms.size()); ++index)
            {
                const auto& term = terms[index];

                if (const auto term_object = std::get_if<TermObjectImpl>(term))
                {
                    const auto object_id = term_object->get_object()->get_identifier();

                    return std::make_pair(index, (int32_t) object_id);
                }
                else if (const auto term_variable = std::get_if<TermVariableImpl>(term))
                {
                    const auto parameter_index = term_variable->get_variable()->get_parameter_index();

                    if (edge.get_src().get_param_index() == parameter_index)
                    {
                        return std::make_pair(index, (int32_t) edge.get_src().get_object_index());
                    }
                    else if (edge.get_dst().get_param_index() == parameter_index)
                    {
                        return std::make_pair(index, (int32_t) edge.get_dst().get_object_index());
                    }
                }
            }
            return std::make_pair(-1, -1);
        };

        auto begin = int32_t { 0 };
        const auto [first_position, first_object_id] = fetch_assignment(begin, literal->get_atom()->get_terms(), consistent_edge);
        ++begin;
        const auto [second_position, second_object_id] = fetch_assignment(begin, literal->get_atom()->get_terms(), consistent_edge);

        bool is_empty_assignment = (second_position == -1);

        if (!is_empty_assignment)
        {
            assert(first_position < second_position);

            const auto& assignment_set = m_f[literal->get_atom()->get_predicate()->get_identifier()];
            const auto assignment_rank = get_assignment_position(BinaryAssignment { first_position, first_object_id, second_position, second_object_id },
                                                                 static_cast<int32_t>(arity),
                                                                 static_cast<int32_t>(m_problem->get_objects().size()));
            assert(assignment_rank < assignment_set.size());
            const auto consistent_with_state = assignment_set[assignment_rank];

            if (literal->is_negated() == consistent_with_state)
            {
                return false;
            }
        }
    }

    return true;
}

AssignmentSet::AssignmentSet(Problem problem, const GroundAtomList& ground_atoms) :
    m_problem(problem),
    m_unary_assignment_set(UnaryAssignmentSet(problem, ground_atoms)),
    m_binary_assignment_set(BinaryAssignmentSet(problem, ground_atoms))
{
}

bool AssignmentSet::literal_all_consistent(const std::vector<Literal>& literals, const consistency_graph::Edge& consistent_edge) const
{
    return m_unary_assignment_set.literal_all_consistent(literals, consistent_edge.get_src())
           && m_unary_assignment_set.literal_all_consistent(literals, consistent_edge.get_dst())
           && m_binary_assignment_set.literal_all_consistent(literals, consistent_edge);
}

bool AssignmentSet::literal_all_consistent(const std::vector<Literal>& literals, const consistency_graph::Vertex& consistent_vertex) const
{
    return m_unary_assignment_set.literal_all_consistent(literals, consistent_vertex);
}

}
