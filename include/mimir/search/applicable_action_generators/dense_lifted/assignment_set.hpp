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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_LIFTED_ASSIGNMENT_SET_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_LIFTED_ASSIGNMENT_SET_HPP_

#include "mimir/formalism/formalism.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted/consistency_graph.hpp"

#include <cassert>
#include <limits>
#include <vector>

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

inline size_t get_assignment_position(const Assignment& assignment, size_t arity, size_t num_objects)
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

inline size_t num_assignments(size_t arity, size_t num_objects)
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

/// @brief AssignmentSet is a helper class representing a set of functions
/// f : Predicates x Params(A) x Object x Params(A) x Object -> {true, false} where
///   1. f(p,i,o,j,o') = true iff there exists an atom p(...,o_i,...,o'_j)
///   2. f(p,i,o,-1,-1) = true iff there exists an atom p(...,o_i,...)
/// with respective meanings
///   1. the assignment [i/o], [j/o'] is consistent
///   2. the assignment [i/o] is consistent
///
/// We say that an assignment set is static if all atoms it considers are static.
template<PredicateCategory P>
class AssignmentSet
{
private:
    Problem m_problem;

    // The underlying function
    std::vector<std::vector<bool>> m_f;

public:
    /// @brief Construct from a given set of ground atoms.
    AssignmentSet(Problem problem, const PredicateList<P>& predicates, const GroundAtomList<P>& ground_atoms);

    /// @brief Insert ground atoms into the assignment set.
    void insert_ground_atom(GroundAtom<P> ground_atom);

    /// @brief Return true iff all literals are consistent with
    /// 1. the assignment set, and 2. the edge of the consistency graph.
    ///
    /// The meaning of the result being true is that the edge remains consistent.
    bool literal_all_consistent(const std::vector<Literal<P>>& literals, const consistency_graph::Edge& consistent_edge) const;

    /// @brief Return true iff all literals are consistent with
    /// 1. the assignment set, and 2. the vertex of the consistency graph.
    ///
    /// The meaning is that the assignment [x/o] is a legal assignment,
    /// and therefore the vertex must be part of the consistency graph.
    /// @param literals
    /// @param consistent_vertex
    /// @return
    bool literal_all_consistent(const std::vector<Literal<P>>& literals, const consistency_graph::Vertex& consistent_vertex) const;
};

template<PredicateCategory P>
AssignmentSet<P>::AssignmentSet(Problem problem, const PredicateList<P>& predicates, const GroundAtomList<P>& ground_atoms) : m_problem(problem)
{
    const auto num_objects = problem->get_objects().size();

    m_f.resize(predicates.size());

    for (const auto& predicate : predicates)
    {
        // Predicates must have indexing 0,1,2,... for this implementation
        assert(predicate->get_identifier() < predicates.size());

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
                assignment_set[get_assignment_position(
                    Assignment { first_position, first_object->get_identifier(), second_position, second_object->get_identifier() },
                    arity,
                    num_objects)] = true;
            }
        }
    }
}

template<PredicateCategory P>
void AssignmentSet<P>::insert_ground_atom(GroundAtom<P> ground_atom)
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
            assignment_set[get_assignment_position(
                Assignment { first_position, first_object->get_identifier(), second_position, second_object->get_identifier() },
                arity,
                num_objects)] = true;
        }
    }
}

template<PredicateCategory P>
bool AssignmentSet<P>::literal_all_consistent(const std::vector<Literal<P>>& literals, const consistency_graph::Edge& consistent_edge) const
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

        // Test all nonempty assignments
        const auto& assignment_set = m_f[literal->get_atom()->get_predicate()->get_identifier()];
        const auto& terms = literal->get_atom()->get_terms();
        const auto num_objects = m_problem->get_objects().size();
        for (size_t index = 0; index < arity; ++index)
        {
            const auto [first_position, first_object_id] = find_assignment(index, terms, consistent_edge);
            bool is_empty_assignment = (first_object_id == MAX_VALUE);
            if (is_empty_assignment)
            {
                // Stop searching for nonempty assignments
                break;
            }
            const auto [second_position, second_object_id] = find_assignment(first_position + 1, terms, consistent_edge);
            assert(first_position < second_position);

            // Test assignment
            const auto assignment_rank =
                get_assignment_position(Assignment { first_position, first_object_id, second_position, second_object_id }, arity, num_objects);
            assert(assignment_rank < assignment_set.size());
            const auto consistent_with_state = assignment_set[assignment_rank];

            // Note: We also check vertex consistency here to avoid literal_all_consistent for each of the vertices of the edge
            // This has the benefit of looping only once over the literals instead of three times.
            // As a result, this check becomes a bit more complicated in the negated case.
            if (!literal->is_negated() && !consistent_with_state)
            {
                // Positive literal conflict of assignment represented by the given vertex was found.
                return false;
            }
            else if (literal->is_negated() && consistent_with_state && ((arity == 1) || ((arity == 2) && (second_position != MAX_VALUE))))
            {
                // Negative literal conflict of assignment represented by the given vertex was found.
                return false;
            }
        }
    }

    return true;
}

template<PredicateCategory P>
bool AssignmentSet<P>::literal_all_consistent(const std::vector<Literal<P>>& literals, const consistency_graph::Vertex& consistent_vertex) const
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

        // Test all nonempty assignments
        const auto& assignment_set = m_f[literal->get_atom()->get_predicate()->get_identifier()];
        const auto& terms = literal->get_atom()->get_terms();
        const auto num_objects = m_problem->get_objects().size();
        for (size_t index = 0; index < arity; ++index)
        {
            const auto [position, object_id] = find_assignment(index, terms, consistent_vertex);
            bool is_empty_assignment = (object_id == MAX_VALUE);
            if (is_empty_assignment)
            {
                // Stop searching for nonempty assignments
                break;
            }

            // Test assignment
            const auto assignment_rank = get_assignment_position(Assignment { position, object_id, MAX_VALUE, MAX_VALUE }, arity, num_objects);
            assert(assignment_rank < assignment_set.size());
            const auto consistent_with_state = assignment_set[assignment_rank];
            if (literal->is_negated() == consistent_with_state)
            {
                // Conflict of assignment represented by the given vertex was found.
                return false;
            }
        }
    }

    return true;
}

}

#endif