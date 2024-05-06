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
        assert(predicate->get_identifier() < static_cast<int>(predicates.size()));

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
        int32_t first_position = -1;
        int32_t second_position = -1;
        int32_t first_object_id = -1;
        int32_t second_object_id = -1;
        bool empty_assignment = true;

        const auto arity = literal->get_atom()->get_predicate()->get_arity();

        // TODO: decide whether we want to split this into unary and binary to make the distinction clear.
        for (std::size_t index = 0; index < literal->get_atom()->get_predicate()->get_arity(); ++index)
        {
            const auto& term = literal->get_atom()->get_terms()[index];

            if (const auto term_object = std::get_if<TermObjectImpl>(term))
            {
                if (arity <= 2)
                {
                    const auto object_id = term_object->get_object()->get_identifier();

                    if (first_position < 0)
                    {
                        first_position = index;
                        first_object_id = static_cast<int32_t>(object_id);
                    }
                    else
                    {
                        second_position = index;
                        second_object_id = static_cast<int32_t>(object_id);
                    }

                    empty_assignment = false;
                }
            }
            else if (const auto term_variable = std::get_if<TermVariableImpl>(term))
            {
                const auto parameter_index = term_variable->get_variable()->get_parameter_index();

                // D: [parameter_index]
                if (consistent_edge.get_src().get_param_index() == parameter_index)
                {
                    if (first_position < 0)
                    {
                        first_position = index;
                        first_object_id = static_cast<int32_t>(consistent_edge.get_src().get_object_index());
                    }
                    else
                    {
                        second_position = index;
                        second_object_id = static_cast<int32_t>(consistent_edge.get_src().get_object_index());
                        break;
                    }

                    empty_assignment = false;
                }
                else if (consistent_edge.get_dst().get_param_index() == parameter_index)
                {
                    if (first_position < 0)
                    {
                        first_position = index;
                        first_object_id = static_cast<int32_t>(consistent_edge.get_dst().get_object_index());
                    }
                    else
                    {
                        second_position = index;
                        second_object_id = static_cast<int32_t>(consistent_edge.get_dst().get_object_index());
                        break;
                    }

                    empty_assignment = false;
                }
            }
        }

        if (!empty_assignment)
        {
            const auto& assignment_set = m_f[literal->get_atom()->get_predicate()->get_identifier()];
            const auto assignment_rank = get_assignment_position(first_position,
                                                                 first_object_id,
                                                                 second_position,
                                                                 second_object_id,
                                                                 static_cast<int32_t>(arity),
                                                                 static_cast<int32_t>(m_problem->get_objects().size()));
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
    }

    return true;
}

bool AssignmentSet::literal_all_consistent(const std::vector<Literal>& literals, const consistency_graph::Vertex& consistent_vertex) const
{
    for (const auto& literal : literals)
    {
        int32_t position = -1;
        int32_t object_id = -1;
        bool empty_assignment = true;

        const auto arity = literal->get_atom()->get_predicate()->get_arity();

        for (std::size_t index = 0; index < literal->get_atom()->get_predicate()->get_arity(); ++index)
        {
            const auto& term = literal->get_atom()->get_terms()[index];

            if (const auto term_object = std::get_if<TermObjectImpl>(term))
            {
                continue;
            }
            else if (const auto term_variable = std::get_if<TermVariableImpl>(term))
            {
                const auto parameter_index = term_variable->get_variable()->get_parameter_index();

                // D: [parameter_index]
                if (consistent_vertex.get_param_index() == parameter_index)
                {
                    position = index;
                    object_id = static_cast<int32_t>(consistent_vertex.get_object_index());

                    empty_assignment = false;
                    break;
                }
            }
        }

        if (!empty_assignment)
        {
            const auto& assignment_set = m_f[literal->get_atom()->get_predicate()->get_identifier()];
            const auto assignment_rank =
                get_assignment_position(position, object_id, -1, -1, static_cast<int32_t>(arity), static_cast<int32_t>(m_problem->get_objects().size()));

            const auto consistent_with_state = assignment_set[assignment_rank];

            if (!literal->is_negated() && !consistent_with_state)
            {
                return false;
            }
            else if (literal->is_negated() && consistent_with_state && ((arity == 1)))
            {
                return false;
            }
        }
    }

    return true;
}

}
