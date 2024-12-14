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

#include "mimir/formalism/assignment_set.hpp"

#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/term.hpp"
#include "mimir/formalism/variable.hpp"

namespace mimir
{

/*
 * Assignment
 */

Assignment::Assignment(Index index, Index object) : first_index(index), first_object(object), second_index(MAX_VALUE), second_object(MAX_VALUE) {}

Assignment::Assignment(Index first_index, Index first_object, Index second_index, Index second_object) :
    first_index(first_index),
    first_object(first_object),
    second_index(second_index),
    second_object(second_object)
{
}

/**
 * AssignmentSet
 */

template<PredicateTag P>
AssignmentSet<P>::AssignmentSet(size_t num_objects, const PredicateList<P>& predicates) : m_num_objects(num_objects)
{
    auto max_predicate_index = Index(0);
    for (const auto& predicate : predicates)
    {
        max_predicate_index = std::max(max_predicate_index, predicate->get_index());
    }
    per_predicate_assignment_set.resize(max_predicate_index + 1);

    for (const auto& predicate : predicates)
    {
        auto& assignment_set = per_predicate_assignment_set.at(predicate->get_index());
        assignment_set.resize(num_assignments(predicate->get_arity(), m_num_objects));
    }
}

template<PredicateTag P>
void AssignmentSet<P>::clear()
{
    for (auto& assignment_set : per_predicate_assignment_set)
    {
        std::fill(assignment_set.begin(), assignment_set.end(), false);
    }
}

template<PredicateTag P>
void AssignmentSet<P>::insert_ground_atoms(const GroundAtomList<P>& ground_atoms)
{
    for (const auto& ground_atom : ground_atoms)
    {
        const auto& arity = ground_atom->get_arity();
        const auto& predicate = ground_atom->get_predicate();
        const auto& arguments = ground_atom->get_objects();
        auto& assignment_set = per_predicate_assignment_set.at(predicate->get_index());

        for (size_t first_index = 0; first_index < arity; ++first_index)
        {
            const auto& first_object = arguments[first_index];
            assignment_set[get_assignment_rank(Assignment(first_index, first_object->get_index()), arity, m_num_objects)] = true;

            for (size_t second_index = first_index + 1; second_index < arity; ++second_index)
            {
                const auto& second_object = arguments[second_index];
                assignment_set[get_assignment_rank(Assignment(first_index, first_object->get_index(), second_index, second_object->get_index()),
                                                   arity,
                                                   m_num_objects)] = true;
            }
        }
    }
}

template<PredicateTag P>
void AssignmentSet<P>::insert_ground_atom(GroundAtom<P> ground_atom)
{
    const auto& arity = ground_atom->get_arity();
    const auto& predicate = ground_atom->get_predicate();
    const auto& arguments = ground_atom->get_objects();
    auto& assignment_set = per_predicate_assignment_set.at(predicate->get_index());

    for (size_t first_index = 0; first_index < arity; ++first_index)
    {
        const auto& first_object = arguments[first_index];
        assignment_set[get_assignment_rank(Assignment(first_index, first_object->get_index()), arity, m_num_objects)] = true;

        for (size_t second_index = first_index + 1; second_index < arity; ++second_index)
        {
            const auto& second_object = arguments[second_index];
            assignment_set[get_assignment_rank(Assignment(first_index, first_object->get_index(), second_index, second_object->get_index()),
                                               arity,
                                               m_num_objects)] = true;
        }
    }
}

template class AssignmentSet<Static>;
template class AssignmentSet<Fluent>;
template class AssignmentSet<Derived>;

}
