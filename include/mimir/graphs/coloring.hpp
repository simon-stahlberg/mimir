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

#include <set>
#include <unordered_map>
#include <vector>

namespace mimir
{
using Color = int;
using ColorList = std::vector<Color>;

class ProblemColorFunction
{
private:
    Problem m_problem;

    std::unordered_map<std::string, Color> m_name_to_color;
    std::unordered_map<Color, std::string> m_color_to_name;

    void initialize_predicates();

public:
    explicit ProblemColorFunction(Problem problem);

    /// @brief get color of object.
    Color get_color(const Object& object);

    /// @brief Get unique color of state atom.
    template<PredicateCategory P>
    Color get_color(const GroundAtom<P>& atom, int pos);

    /// @brief Get unique color of dynamic goal literal.
    template<DynamicPredicateCategory P>
    Color get_color(const State& state, const GroundLiteral<P>& literal, int pos, bool mark_true_goal_literal = false);

    /// @brief Get unique color of static goal literal.
    Color get_color(const State& state, const GroundLiteral<Static>& literal, int pos, bool mark_true_goal_literal = false);

    /// @brief Get name of color.
    const std::string& get_color_name(Color color) const;

    Problem get_problem() const;
};

/**
 * Implementations
 */

template<PredicateCategory P>
Color ProblemColorFunction::get_color(const GroundAtom<P>& atom, int pos)
{
    return m_name_to_color.at(atom->get_predicate()->get_name() + ":" + std::to_string(pos));
}

template<DynamicPredicateCategory P>
Color ProblemColorFunction::get_color(const State& state, const GroundLiteral<P>& literal, int pos, bool mark_true_goal_literal)
{
    bool is_satisfied_in_goal = state.literal_holds(literal);
    return m_name_to_color.at(literal->get_atom()->get_predicate()->get_name() + ":g"
                              + (mark_true_goal_literal ? (is_satisfied_in_goal ? ":true" : ":false") : "") + ":" + std::to_string(pos));
}

}
#endif