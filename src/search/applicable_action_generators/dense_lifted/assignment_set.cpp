#include "mimir/search/applicable_action_generators/dense_lifted/assignment_set.hpp"

#include "mimir/formalism/atom.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/term.hpp"
#include "mimir/formalism/variable.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted/consistency_graph.hpp"

#include <cassert>
#include <limits>

namespace mimir
{
// We use this as special value and when adding 1 we obtain 0.
static const size_t MAX_VALUE = std::numeric_limits<size_t>::max();

/// @brief Encapsulate assignment of objects to variables of atoms.
struct Assignment
{
    size_t first_position;
    size_t first_object;
    size_t second_position;
    size_t second_object;
};

static size_t get_assignment_position(const Assignment& assignment, size_t arity, size_t num_objects)
{
    const auto first = 1;
    const auto second = first * (arity + 1);
    const auto third = second * (arity + 1);
    const auto fourth = third * (num_objects + 1);
    const auto rank = (first * (assignment.first_position + 1))      //
                      + (second * (assignment.second_position + 1))  //
                      + (third * (assignment.first_object + 1))      //
                      + (fourth * (assignment.second_object + 1));
    return rank;
}

static size_t num_assignments(size_t arity, size_t num_objects)
{
    const auto first = 1;
    const auto second = first * (arity + 1);
    const auto third = second * (arity + 1);
    const auto fourth = third * (num_objects + 1);
    const auto max = (first * arity)          //
                     + (second * arity)       //
                     + (third * num_objects)  //
                     + (fourth * num_objects);
    return max + 1;
}

AssignmentSet::AssignmentSet(Problem problem, const GroundAtomList& ground_atoms) : m_problem(problem)
{
    const auto num_objects = problem->get_objects().size();
    const auto& predicates = problem->get_domain()->get_predicates();
    const auto& domain_derived_predicates = problem->get_domain()->get_derived_predicates();
    const auto& problem_derived_predicates = problem->get_derived_predicates();

    // TODO: Make this robust to framented indexing. Perhaps we can introduce some ListProxy class
    // that maps from fragmented to nonfragmented storage through an additional vector.
    const auto num_total_predicates = predicates.size() + domain_derived_predicates.size() + problem_derived_predicates.size();
    m_f.resize(num_total_predicates);
    for (const auto& predicate : predicates)
    {
        // Predicates must have indexing 0,1,2,... for this implementation
        assert(predicate->get_identifier() < num_total_predicates);

        auto& assignment_set = m_f[predicate->get_identifier()];
        assignment_set.resize(num_assignments(predicate->get_arity(), num_objects));
    }
    for (const auto& predicate : domain_derived_predicates)
    {
        // Predicates must have indexing 0,1,2,... for this implementation
        assert(predicate->get_identifier() < num_total_predicates);

        auto& assignment_set = m_f[predicate->get_identifier()];
        assignment_set.resize(num_assignments(predicate->get_arity(), num_objects));
    }
    for (const auto& predicate : problem_derived_predicates)
    {
        // Predicates must have indexing 0,1,2,... for this implementation
        assert(predicate->get_identifier() < num_total_predicates);

        auto& assignment_set = m_f[predicate->get_identifier()];
        assignment_set.resize(num_assignments(predicate->get_arity(), num_objects));
    }

    for (const auto& ground_atom : ground_atoms)
    {
        const auto& arity = ground_atom->get_arity();
        const auto& predicate = ground_atom->get_predicate();
        const auto& arguments = ground_atom->get_objects();
        auto& assignment_set = m_f[predicate->get_identifier()];

        for (size_t first_position = 0; first_position < arity; ++first_position)
        {
            const auto& first_object = arguments[first_position];
            assignment_set[get_assignment_position(Assignment { first_position, first_object->get_identifier(), MAX_VALUE, MAX_VALUE }, arity, num_objects)] =
                true;

            for (size_t second_position = first_position + 1; second_position < arity; ++second_position)
            {
                const auto& second_object = arguments[second_position];
                assignment_set[get_assignment_position(Assignment { second_position, second_object->get_identifier(), MAX_VALUE, MAX_VALUE },
                                                       arity,
                                                       num_objects)] = true;
                assignment_set[get_assignment_position(
                    Assignment { first_position, first_object->get_identifier(), second_position, second_object->get_identifier() },
                    arity,
                    num_objects)] = true;
            }
        }
    }
}

void AssignmentSet::insert_ground_atom(GroundAtom ground_atom)
{
    const auto num_objects = m_problem->get_objects().size();

    const auto& arity = ground_atom->get_arity();
    const auto& predicate = ground_atom->get_predicate();
    const auto& arguments = ground_atom->get_objects();
    auto& assignment_set = m_f[predicate->get_identifier()];

    for (size_t first_position = 0; first_position < arity; ++first_position)
    {
        const auto& first_object = arguments[first_position];
        assignment_set[get_assignment_position(Assignment { first_position, first_object->get_identifier(), MAX_VALUE, MAX_VALUE }, arity, num_objects)] = true;

        for (size_t second_position = first_position + 1; second_position < arity; ++second_position)
        {
            const auto& second_object = arguments[second_position];
            assignment_set[get_assignment_position(Assignment { second_position, second_object->get_identifier(), MAX_VALUE, MAX_VALUE }, arity, num_objects)] =
                true;
            assignment_set[get_assignment_position(
                Assignment { first_position, first_object->get_identifier(), second_position, second_object->get_identifier() },
                arity,
                num_objects)] = true;
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

        const auto find_assignment = [arity](size_t index, const TermList& terms, const consistency_graph::Edge& edge)
        {
            for (; index < arity; ++index)
            {
                const auto& term = terms[index];

                if (const auto term_object = std::get_if<TermObjectImpl>(term))
                {
                    return std::make_pair(index, term_object->get_object()->get_identifier());
                }
                else if (const auto term_variable = std::get_if<TermVariableImpl>(term))
                {
                    const auto parameter_index = term_variable->get_variable()->get_parameter_index();

                    if (edge.get_src().get_param_index() == parameter_index)
                    {
                        return std::make_pair(index, edge.get_src().get_object_index());
                    }
                    else if (edge.get_dst().get_param_index() == parameter_index)
                    {
                        return std::make_pair(index, edge.get_dst().get_object_index());
                    }
                }
            }
            return std::make_pair(MAX_VALUE, MAX_VALUE);
        };

        // Find nonempty assignment
        const auto [first_position, first_object_id] = find_assignment(0, literal->get_atom()->get_terms(), consistent_edge);
        bool is_empty_assignment = (first_object_id == MAX_VALUE);
        if (is_empty_assignment)
        {
            continue;
        }
        const auto [second_position, second_object_id] = find_assignment(first_position + 1, literal->get_atom()->get_terms(), consistent_edge);

        // Test assignment
        const auto& assignment_set = m_f[literal->get_atom()->get_predicate()->get_identifier()];
        const auto assignment_rank =
            get_assignment_position(Assignment { first_position, first_object_id, second_position, second_object_id }, arity, m_problem->get_objects().size());
        assert(assignment_rank < assignment_set.size());
        const auto consistent_with_state = assignment_set[assignment_rank];

        if (!literal->is_negated() && !consistent_with_state)
        {
            return false;
        }
        else if (literal->is_negated() && consistent_with_state && ((arity == 1) || ((arity == 2) && (second_position != (size_t) -1))))
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

        const auto find_assignment = [arity](size_t index, const TermList& terms, const consistency_graph::Vertex& vertex)
        {
            for (; index < arity; ++index)
            {
                const auto& term = terms[index];

                if (const auto term_object = std::get_if<TermObjectImpl>(term))
                {
                    return std::make_pair(index, term_object->get_object()->get_identifier());
                }
                else if (const auto term_variable = std::get_if<TermVariableImpl>(term))
                {
                    const auto parameter_index = term_variable->get_variable()->get_parameter_index();

                    if (vertex.get_param_index() == parameter_index)
                    {
                        return std::make_pair(index, vertex.get_object_index());
                    }
                }
            }
            return std::make_pair(MAX_VALUE, MAX_VALUE);
        };

        // Find nonempty assignment
        const auto [position, object_id] = find_assignment(0, literal->get_atom()->get_terms(), consistent_vertex);
        bool is_empty_assignment = (object_id == MAX_VALUE);
        if (is_empty_assignment)
        {
            continue;
        }

        // Test assignment
        const auto& assignment_set = m_f[literal->get_atom()->get_predicate()->get_identifier()];
        const auto assignment_rank = get_assignment_position(Assignment { position, object_id, MAX_VALUE, MAX_VALUE }, arity, m_problem->get_objects().size());
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