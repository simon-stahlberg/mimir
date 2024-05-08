#include "mimir/search/applicable_action_generators/dense_lifted/assignment_set.hpp"

#include "mimir/formalism/factories.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted/consistency_graph.hpp"

#include <cassert>

namespace mimir
{

size_t UnaryAssignmentSet::get_assignment_position(const UnaryAssignment& assignment, size_t arity, size_t num_objects)
{
    const auto position_factor = 1;
    const auto object_factor = position_factor * arity;
    return assignment.position * position_factor + assignment.object * object_factor;
}

size_t UnaryAssignmentSet::get_num_assignments(size_t arity, size_t num_objects) { return arity * num_objects; }

UnaryAssignmentSet::UnaryAssignmentSet(Problem problem, const GroundAtomList& atoms) : m_problem(problem), m_f()
{
    const auto num_objects = problem->get_objects().size();
    const auto& predicates = problem->get_domain()->get_predicates();

    m_f.resize(predicates.size());
    for (const auto& predicate : predicates)
    {
        // Predicates must have indexing 0,1,2,... for this implementation
        assert(predicate->get_identifier() < predicates.size());

        const auto arity = predicate->get_arity();

        auto& assignment_set = m_f[predicate->get_identifier()];

        assignment_set.resize(UnaryAssignmentSet::get_num_assignments(arity, num_objects));
    }

    for (const auto& ground_atom : atoms)
    {
        const auto arity = ground_atom->get_arity();

        const auto& predicate = ground_atom->get_predicate();
        const auto& arguments = ground_atom->get_objects();
        auto& assignment_set = m_f[predicate->get_identifier()];

        for (size_t position = 0; position < arity; ++position)
        {
            const auto& first_object = arguments[position];
            assignment_set[get_assignment_position(UnaryAssignment { position, first_object->get_identifier() }, arity, num_objects)] = true;
        }
    }
}

bool UnaryAssignmentSet::literal_all_consistent(const std::vector<Literal>& literals, const consistency_graph::Vertex& consistent_vertex) const
{
    /*
        For given vertex in consistency graph, check whether there exists either
        I^+) p1 in positive literals such that p1/v (partial assignment) does not match any p2 in assignment set, or
        I^-) p1 in negative literals such that p1/v (complete assignment) is in assignment set.
    */
    for (const auto& literal : literals)
    {
        const auto arity = literal->get_atom()->get_predicate()->get_arity();

        if (literal->is_negated() && arity != 1)
        {
            // No complete unary assignment is possible
            continue;
        }

        // Helper function to find a next matching assignment for a given vertex, if it exists.
        const auto find_assignment = [arity](size_t& index, const TermList& terms, const consistency_graph::Vertex& vertex)
        {
            for (; index < arity; ++index)
            {
                const auto& term = terms[index];

                if (const auto term_object = std::get_if<TermObjectImpl>(term))
                {
                    const auto object_id = term_object->get_object()->get_identifier();

                    return object_id;
                }
                else if (const auto term_variable = std::get_if<TermVariableImpl>(term))
                {
                    const auto parameter_index = term_variable->get_variable()->get_parameter_index();

                    if (vertex.get_param_index() == parameter_index)
                    {
                        return vertex.get_object_index();
                    }
                }
            }
            return (size_t) -1;
        };

        auto index = (size_t) 0;
        const auto object_id = find_assignment(index, literal->get_atom()->get_terms(), consistent_vertex);
        const auto position = index;
        bool is_incomplete_unary_assignment = (object_id == (size_t) -1);
        if (is_incomplete_unary_assignment)
        {
            continue;
        }

        const auto& assignment_set = m_f[literal->get_atom()->get_predicate()->get_identifier()];
        const auto assignment_rank =
            UnaryAssignmentSet::get_assignment_position(UnaryAssignment { position, object_id }, arity, m_problem->get_objects().size());
        assert(assignment_rank < assignment_set.size());
        const auto consistent_with_state = assignment_set[assignment_rank];
        if (literal->is_negated() == consistent_with_state)
        {
            return false;
        }
    }

    return true;
}

size_t BinaryAssignmentSet::get_assignment_position(const BinaryAssignment& assignment, size_t arity, size_t num_objects)
{
    const auto first_position_factor = 1;
    const auto second_position_factor = first_position_factor * arity;
    const auto first_object_factor = second_position_factor * arity;
    const auto second_object_factor = first_object_factor * num_objects;
    return assignment.first_position * first_position_factor + assignment.second_position * second_position_factor
           + assignment.first_object * first_object_factor + assignment.second_object * second_object_factor;
}

size_t BinaryAssignmentSet::get_num_assignments(size_t arity, size_t num_objects) { return arity * arity * num_objects * num_objects; }

BinaryAssignmentSet::BinaryAssignmentSet(Problem problem, const GroundAtomList& atoms) : m_problem(problem), m_f()
{
    const auto num_objects = problem->get_objects().size();
    const auto& predicates = problem->get_domain()->get_predicates();

    m_f.resize(predicates.size());
    for (const auto& predicate : predicates)
    {
        // Predicates must have indexing 0,1,2,... for this implementation
        assert(predicate->get_identifier() < predicates.size());

        auto& assignment_set = m_f[predicate->get_identifier()];
        assignment_set.resize(BinaryAssignmentSet::get_num_assignments(predicate->get_arity(), num_objects));
    }

    for (const auto& ground_atom : atoms)
    {
        const auto& arity = ground_atom->get_arity();

        const auto& predicate = ground_atom->get_predicate();
        const auto& arguments = ground_atom->get_objects();
        auto& assignment_set = m_f[predicate->get_identifier()];

        for (size_t first_position = 0; first_position < arity; ++first_position)
        {
            const auto& first_object = arguments[first_position];

            for (size_t second_position = first_position + 1; second_position < arity; ++second_position)
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
    /*
        For given vertex in consistency graph, check whether there exists either
        I^+) p1 in positive literals such that p1/{v1,v2} (partial assignment) does not match any p2 in assignment set, or
        I^-) p1 in negative literals such that p1/{v1,v2} (complete assignment) is in assignment set.
    */
    for (const auto& literal : literals)
    {
        const auto arity = literal->get_atom()->get_predicate()->get_arity();

        if (literal->is_negated() && arity != 2)
        {
            // No complete binary assignment is possiblee
            continue;
        }

        // Helper function to find a next matching assignment for a given vertex, if it exists.
        const auto find_assignment = [arity](size_t& index, const TermList& terms, const consistency_graph::Edge& edge)
        {
            for (; index < arity; ++index)
            {
                const auto& term = terms[index];

                if (const auto term_object = std::get_if<TermObjectImpl>(term))
                {
                    const auto object_id = term_object->get_object()->get_identifier();

                    return object_id;
                }
                else if (const auto term_variable = std::get_if<TermVariableImpl>(term))
                {
                    const auto parameter_index = term_variable->get_variable()->get_parameter_index();

                    if (edge.get_src().get_param_index() == parameter_index)
                    {
                        return edge.get_src().get_object_index();
                    }
                    else if (edge.get_dst().get_param_index() == parameter_index)
                    {
                        return edge.get_dst().get_object_index();
                    }
                }
            }
            return (size_t) -1;
        };

        auto index = size_t { 0 };
        // Find assignment from current index
        const auto first_object_id = find_assignment(index, literal->get_atom()->get_terms(), consistent_edge);
        const auto first_position = index;
        // Find assignment from next index
        const auto second_object_id = find_assignment(++index, literal->get_atom()->get_terms(), consistent_edge);
        const auto second_position = index;
        bool is_incomplete_binary_assignment = (second_object_id == (size_t) -1);
        if (is_incomplete_binary_assignment)
        {
            continue;
        }

        const auto& assignment_set = m_f[literal->get_atom()->get_predicate()->get_identifier()];
        const auto assignment_rank = get_assignment_position(BinaryAssignment { first_position, first_object_id, second_position, second_object_id },
                                                             arity,
                                                             m_problem->get_objects().size());
        assert(assignment_rank < assignment_set.size());
        const auto consistent_with_state = assignment_set[assignment_rank];
        if (literal->is_negated() == consistent_with_state)
        {
            return false;
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
