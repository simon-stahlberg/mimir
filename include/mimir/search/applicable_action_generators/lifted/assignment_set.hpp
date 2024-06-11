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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_ASSIGNMENT_SET_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_ASSIGNMENT_SET_HPP_

#include "mimir/formalism/formalism.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/applicable_action_generators/lifted/consistency_graph.hpp"

#include <cassert>
#include <limits>
#include <tuple>
#include <vector>

namespace mimir
{

/// @brief Encapsulate assignment of objects to variables of atoms.
struct Assignment
{
    // We use this as special value and when adding 1 we obtain 0.
    static const size_t MAX_VALUE = std::numeric_limits<size_t>::max();

    size_t first_index;
    size_t first_object;
    size_t second_index;
    size_t second_object;

    Assignment(size_t index, size_t object);

    Assignment(size_t first_index, size_t first_object, size_t second_index, size_t second_object);

    size_t size() const;
};

class VertexAssignmentIterator
{
private:
    // We use this as special value and when adding 1 we obtain 0.
    static const size_t UNDEFINED = std::numeric_limits<size_t>::max();

    const TermList& m_terms;
    const consistency_graph::Vertex& m_vertex;

    size_t m_index;
    size_t m_object;

    size_t get_object_if_overlap(const Term& term);

    void find_next_binding();

public:
    VertexAssignmentIterator(const TermList& terms, const consistency_graph::Vertex& vertex);

    bool has_next() const;

    Assignment next();
};

class EdgeAssignmentIterator
{
private:
    // We use this as special value and when adding 1 we obtain 0.
    static const size_t UNDEFINED = std::numeric_limits<size_t>::max();

    const TermList& m_terms;
    const consistency_graph::Edge& m_edge;

    size_t m_first_index;
    size_t m_second_index;
    size_t m_first_object;
    size_t m_second_object;

    size_t get_object_if_overlap(const Term& term);

    void find_next_binding();

public:
    EdgeAssignmentIterator(const TermList& terms, const consistency_graph::Edge& edge);

    bool has_next() const;

    Assignment next();
};

inline size_t get_assignment_rank(const Assignment& assignment, size_t arity, size_t num_objects)
{
    const auto first = 1;
    const auto second = first * (arity + 1);
    const auto third = second * (arity + 1);
    const auto fourth = third * (num_objects + 1);
    const auto rank = (first * (assignment.first_index + 1))      //
                      + (second * (assignment.second_index + 1))  //
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
    bool consistent_literals(const LiteralList<P>& literals, const consistency_graph::Edge& consistent_edge) const;

    /// @brief Return true iff all literals are consistent with
    /// 1. the assignment set, and 2. the vertex of the consistency graph.
    ///
    /// The meaning is that the assignment [x/o] is a legal assignment,
    /// and therefore the vertex must be part of the consistency graph.
    /// @param literals
    /// @param consistent_vertex
    /// @return
    bool consistent_literals(const LiteralList<P>& literals, const consistency_graph::Vertex& consistent_vertex) const;
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

        for (size_t first_index = 0; first_index < arity; ++first_index)
        {
            const auto& first_object = arguments[first_index];
            assignment_set[get_assignment_rank(Assignment(first_index, first_object->get_identifier()), arity, num_objects)] = true;

            for (size_t second_index = first_index + 1; second_index < arity; ++second_index)
            {
                const auto& second_object = arguments[second_index];
                assignment_set[get_assignment_rank(Assignment(first_index, first_object->get_identifier(), second_index, second_object->get_identifier()), arity, num_objects)] = true;
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

    for (size_t first_index = 0; first_index < arity; ++first_index)
    {
        const auto& first_object = arguments[first_index];
        assignment_set[get_assignment_rank(Assignment(first_index, first_object->get_identifier()), arity, num_objects)] = true;

        for (size_t second_index = first_index + 1; second_index < arity; ++second_index)
        {
            const auto& second_object = arguments[second_index];
            assignment_set[get_assignment_rank(Assignment(first_index, first_object->get_identifier(), second_index, second_object->get_identifier()), arity, num_objects)] = true;
        }
    }
}

template<PredicateCategory P, typename AssignmentIterator>
bool consistent_literals_helper(const Problem& problem, const std::vector<std::vector<bool>>& assignment_sets, const LiteralList<P>& literals, const auto& element)
{
    // If the type of "element" is "Vertex", then "AssignmentIterator" must be "VertexAssignmentIterator". Likewise for "Edge" and "EdgeAssignmentIterator".
    using ExpectedElementType = typename std::conditional<std::is_same<AssignmentIterator, EdgeAssignmentIterator>::value, consistency_graph::Edge, consistency_graph::Vertex>::type;
    static_assert(std::is_same<AssignmentIterator, EdgeAssignmentIterator>::value || std::is_same<AssignmentIterator, VertexAssignmentIterator>::value, "Invalid AssignmentIterator type");
    static_assert(std::is_same<decltype(element), const ExpectedElementType&>::value, "Mismatch between AssignmentIterator and element type");

    for (const auto& literal : literals)
    {
        const auto arity = literal->get_atom()->get_predicate()->get_arity();
        const auto negated = literal->is_negated();

        if (negated && arity != 1 && arity != 2)
        {
            continue;
        }

        const auto num_objects = problem->get_objects().size();
        const auto& assignment_set = assignment_sets[literal->get_atom()->get_predicate()->get_identifier()];
        const auto& terms = literal->get_atom()->get_terms();
        auto assignment_iterator = AssignmentIterator(terms, element);

        while (assignment_iterator.has_next())
        {
            const auto assignment = assignment_iterator.next();
            assert(assignment.first_index < arity);
            assert(assignment.first_index < assignment.second_index);

            const auto assignment_rank = get_assignment_rank(assignment, arity, num_objects);
            const auto true_assignment = assignment_set[assignment_rank];

            if (!negated && !true_assignment)
            {
                return false;
            }

            if (negated && true_assignment && (assignment.size() == arity))
            {
                return false;
            }
        }
    }

    return true;
}

template<PredicateCategory P>
bool AssignmentSet<P>::consistent_literals(const LiteralList<P>& literals, const consistency_graph::Edge& edge) const
{
    return consistent_literals_helper<P, EdgeAssignmentIterator>(m_problem, m_f, literals, edge);
}

template<PredicateCategory P>
bool AssignmentSet<P>::consistent_literals(const LiteralList<P>& literals, const consistency_graph::Vertex& vertex) const
{
    return consistent_literals_helper<P, VertexAssignmentIterator>(m_problem, m_f, literals, vertex);
}

}

#endif
