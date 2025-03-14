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

#ifndef MIMIR_DATASETS_GENERALIZED_COLOR_FUNCTION_HPP_
#define MIMIR_DATASETS_GENERALIZED_COLOR_FUNCTION_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/generalized_problem.hpp"
#include "mimir/graphs/declarations.hpp"
#include "mimir/search/declarations.hpp"

#include <unordered_map>

namespace mimir::datasets
{

class GeneralizedColorFunction
{
private:
    GeneralizedProblem m_generalized_problem;

    std::unordered_map<graphs::Color, std::string> m_color_to_name;  ///< for visualizations

    PredicateMaps<graphs::Color, Static, Fluent, Derived> m_predicate_colors;

public:
    explicit GeneralizedColorFunction(GeneralizedProblem generalized_problem);

    /// @brief get color of object.
    graphs::Color get_color(Object object) const;

    /// @brief Get unique color of state atom.
    template<StaticOrFluentOrDerived P>
    graphs::Color get_color(GroundAtom<P> atom, size_t pos) const;

    /// @brief Get unique color of goal literal.
    template<StaticOrFluentOrDerived P>
    graphs::Color get_color(GroundLiteral<P> literal, size_t pos, search::State state, const ProblemImpl& problem, bool mark_true_goal_literal = false) const;

    /// @brief Get name of color.
    const std::string& get_color_name(graphs::Color color) const;

    /**
     * Getters
     */
};

}
#endif