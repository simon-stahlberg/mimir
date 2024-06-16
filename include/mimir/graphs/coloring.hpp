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

#ifndef MIMIR_GRAPHS_COLORING_HPP_
#define MIMIR_GRAPHS_COLORING_HPP_

#include "mimir/formalism/formalism.hpp"
#include "mimir/search/state.hpp"

#include <unordered_map>
#include <vector>

namespace mimir
{
using Color = int;
using ColorList = std::vector<int>;

class ProblemColorFunction
{
private:
    Problem m_problem;
    int m_num_objects;
    int m_num_predicates;
    int m_max_predicate_arity;

    std::unordered_map<std::string, int> m_lexicographically_sorted_predicate_names_to_index;

public:
    ProblemColorFunction(const Problem problem);

    /// @brief get color of object.
    Color get_color(const Object object);

    /// @brief Get unique color of state atom.
    template<PredicateCategory P>
    Color get_color(const GroundAtom<P> atom, int pos);

    /// @brief Get unique color of goal literal.
    template<PredicateCategory P>
    Color get_color(const State state, const GroundLiteral<P> literal, int pos, bool mark_true_goal_literal = false);

    Problem get_problem() const;
};

/**
 * Implementations
 */

template<PredicateCategory P>
Color ProblemColorFunction::get_color(const GroundAtom<P> atom, int pos)
{
    const auto predicate_index = m_lexicographically_sorted_predicate_names_to_index.at(atom->get_predicate()->get_name());
    const auto offset_objects = 1;
    return offset_objects + m_max_predicate_arity * predicate_index + pos;
}

template<PredicateCategory P>
Color ProblemColorFunction::get_color(const State state, const GroundLiteral<P> literal, int pos, bool mark_true_goal_literal)
{
    bool is_satisfied_in_goal = state.literal_holds(literal);
    const auto predicate_index = m_lexicographically_sorted_predicate_names_to_index.at(literal->get_atom()->get_predicate()->get_name());
    const auto offset_objects = 1;
    const auto offset_atoms = m_max_predicate_arity * m_num_predicates;
    return offset_objects + offset_atoms + 3 * m_max_predicate_arity * predicate_index
           + (mark_true_goal_literal ? (is_satisfied_in_goal ? 2 : 1) : 0) * m_max_predicate_arity + pos;
}

Problem ProblemColorFunction::get_problem() const { return m_problem; }

}
#endif