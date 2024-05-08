#include "mimir/search/applicable_action_generators/dense_lifted/assignment_set.hpp"

#include "mimir/formalism/factories.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted/consistency_graph.hpp"

#include <cassert>

namespace mimir
{
static size_t
get_assignment_position(int32_t first_position, int32_t first_object, int32_t second_position, int32_t second_object, int32_t arity, int32_t num_objects)
{
    const auto first = 1;
    const auto second = first * (arity + 1);
    const auto third = second * (arity + 1);
    const auto fourth = third * (num_objects + 1);
    const auto rank = (first * (first_position + 1)) + (second * (second_position + 1)) + (third * (first_object + 1)) + (fourth * (second_object + 1));
    return (size_t) rank;
}

static size_t num_assignments(int32_t arity, int32_t num_objects)
{
    const auto first = 1;
    const auto second = first * (arity + 1);
    const auto third = second * (arity + 1);
    const auto fourth = third * (num_objects + 1);
    const auto max = (first * arity) + (second * arity) + (third * num_objects) + (fourth * num_objects);
    return (size_t) (max + 1);
}

AssignmentSet::AssignmentSet(Problem problem, const GroundAtomList& ground_atoms) : m_problem(problem)
{
    const auto num_objects = problem->get_objects().size();
    const auto& predicates = problem->get_domain()->get_predicates();

    // D: Create assignment set for each predicate
    m_f.resize(predicates.size());
    for (const auto& predicate : predicates)
    {
        // Predicates must have indexing 0,1,2,... for this implementation
        assert(predicate->get_identifier() < predicates.size());

        auto& assignment_set = m_f[predicate->get_identifier()];
        assignment_set.resize(num_assignments(predicate->get_arity(), num_objects));
    }
    // D: Initialize m_f
    for (const auto& ground_atom : ground_atoms)
    {
        const auto& arity = ground_atom->get_arity();
        const auto& predicate = ground_atom->get_predicate();
        const auto& arguments = ground_atom->get_objects();
        auto& assignment_set = m_f[predicate->get_identifier()];

        for (size_t first_position = 0; first_position < arity; ++first_position)
        {
            const auto& first_object = arguments[first_position];
            assignment_set[get_assignment_position(first_position, first_object->get_identifier(), -1, -1, arity, num_objects)] = true;

            for (size_t second_position = first_position + 1; second_position < arity; ++second_position)
            {
                const auto& second_object = arguments[second_position];
                assignment_set[get_assignment_position(second_position, second_object->get_identifier(), -1, -1, arity, num_objects)] = true;
                assignment_set[get_assignment_position(first_position,
                                                       first_object->get_identifier(),
                                                       second_position,
                                                       second_object->get_identifier(),
                                                       arity,
                                                       num_objects)] = true;
            }
        }
    }
}

bool AssignmentSet::literal_all_consistent(const std::vector<Literal>& literals, const consistency_graph::Edge& consistent_edge) const
{
    for (const auto& literal : literals)
    {
        const auto arity = literal->get_atom()->get_predicate()->get_arity();

        if (literal->is_negated() && arity != 1 && arity != 2)
        {
            continue;
        }

        const auto find_assignment = [arity](int32_t index, const TermList& terms, const consistency_graph::Edge& edge)
        {
            for (; index < (int32_t) arity; ++index)
            {
                const auto& term = terms[index];

                if (const auto term_object = std::get_if<TermObjectImpl>(term))
                {
                    return std::make_pair(index, static_cast<int32_t>(term_object->get_object()->get_identifier()));
                }
                else if (const auto term_variable = std::get_if<TermVariableImpl>(term))
                {
                    const auto parameter_index = term_variable->get_variable()->get_parameter_index();

                    if (edge.get_src().get_param_index() == parameter_index)
                    {
                        return std::make_pair(index, static_cast<int32_t>(edge.get_src().get_object_index()));
                    }
                    else if (edge.get_dst().get_param_index() == parameter_index)
                    {
                        return std::make_pair(index, static_cast<int32_t>(edge.get_dst().get_object_index()));
                    }
                }
            }
            return std::make_pair(-1, -1);
        };

        // Find nonempty assignment
        const auto [first_position, first_object_id] = find_assignment(0, literal->get_atom()->get_terms(), consistent_edge);
        bool is_empty_assignment = (first_object_id == -1);
        if (is_empty_assignment)
        {
            continue;
        }
        const auto [second_position, second_object_id] = find_assignment(first_position + 1, literal->get_atom()->get_terms(), consistent_edge);

        // Test assignment
        const auto& assignment_set = m_f[literal->get_atom()->get_predicate()->get_identifier()];
        const auto assignment_rank = get_assignment_position(first_position,
                                                             first_object_id,
                                                             second_position,
                                                             second_object_id,
                                                             static_cast<int32_t>(arity),
                                                             static_cast<int32_t>(m_problem->get_objects().size()));
        assert(assignment_rank < assignment_set.size());
        const auto consistent_with_state = assignment_set[assignment_rank];
        if (!literal->is_negated() && !consistent_with_state)
        {
            return false;
        }
        else if (literal->is_negated() && consistent_with_state && ((arity == 1) || ((arity == 2) && (second_position >= 0))))
        {
            return false;
        }
    }

    return true;
}

bool AssignmentSet::literal_all_consistent(const std::vector<Literal>& literals, const consistency_graph::Vertex& consistent_vertex) const
{
    for (const auto& literal : literals)
    {
        const auto arity = literal->get_atom()->get_predicate()->get_arity();

        if (literal->is_negated() && arity != 1)
        {
            continue;
        }

        const auto find_assignment = [arity](int32_t index, const TermList& terms, const consistency_graph::Vertex& vertex)
        {
            for (; index < (int32_t) arity; ++index)
            {
                const auto& term = terms[index];

                if (const auto term_object = std::get_if<TermObjectImpl>(term))
                {
                    return std::make_pair(index, static_cast<int32_t>(term_object->get_object()->get_identifier()));
                }
                else if (const auto term_variable = std::get_if<TermVariableImpl>(term))
                {
                    const auto parameter_index = term_variable->get_variable()->get_parameter_index();

                    if (vertex.get_param_index() == parameter_index)
                    {
                        return std::make_pair(index, static_cast<int32_t>(vertex.get_object_index()));
                    }
                }
            }
            return std::make_pair(-1, -1);
        };

        // Find nonempy assignment
        const auto [position, object_id] = find_assignment(0, literal->get_atom()->get_terms(), consistent_vertex);
        bool is_empty_assignment = (object_id == -1);
        if (is_empty_assignment)
        {
            continue;
        }

        // Test assignment
        const auto& assignment_set = m_f[literal->get_atom()->get_predicate()->get_identifier()];
        const auto assignment_rank =
            get_assignment_position(position, object_id, -1, -1, static_cast<int32_t>(arity), static_cast<int32_t>(m_problem->get_objects().size()));
        assert(assignment_rank < assignment_set.size());
        const auto consistent_with_state = assignment_set[assignment_rank];
        if (literal->is_negated() == consistent_with_state)
        {
            return false;
        }
    }

    return true;
}

}