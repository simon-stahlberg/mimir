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

#ifndef MIMIR_FORMALISM_COLOR_FUNCTION_HPP_
#define MIMIR_FORMALISM_COLOR_FUNCTION_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/graphs/types.hpp"

#include <unordered_map>

namespace mimir::formalism
{

class ColorFunctionImpl
{
private:
    GeneralizedProblem m_generalized_problem;

    std::unordered_map<graphs::Color, std::string> m_color_to_name;  ///< for visualizations

    PredicateMaps<graphs::Color, StaticTag, FluentTag, DerivedTag> m_predicate_to_color_offsets;

    ColorFunctionImpl(GeneralizedProblem generalized_problem,
                      std::unordered_map<graphs::Color, std::string> color_to_name,
                      PredicateMaps<graphs::Color, StaticTag, FluentTag, DerivedTag> predicate_to_color_offsets);

public:
    static ColorFunction create(GeneralizedProblem generalized_problem);
    static ColorFunction create(Problem problem);

    /// @brief Get the color of an object vertex.
    graphs::Color get_color(Object object) const;

    /// @brief Get the color of a positional argument vertex associated with a ground atom.
    template<IsStaticOrFluentOrDerivedTag P>
    graphs::Color get_color(GroundAtom<P> atom, size_t pos) const;

    /// @brief Get the color of a positional argument vertex associated with a ground literal.
    template<IsStaticOrFluentOrDerivedTag P>
    graphs::Color get_color(GroundLiteral<P> literal, size_t pos) const;

    /// @brief Get the name of a color.
    const std::string& get_color_name(graphs::Color color) const;
};

}
#endif