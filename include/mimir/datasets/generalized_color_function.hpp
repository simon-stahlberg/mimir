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

#include "mimir/datasets/declarations.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/graphs/declarations.hpp"

#include <unordered_map>

namespace mimir::datasets
{

class GeneralizedColorFunctionImpl
{
private:
    formalism::GeneralizedProblem m_generalized_problem;

    std::unordered_map<graphs::Color, std::string> m_color_to_name;  ///< for visualizations

    formalism::PredicateMaps<graphs::Color, formalism::StaticTag, formalism::FluentTag, formalism::DerivedTag> m_predicate_to_color_offsets;

    GeneralizedColorFunctionImpl(
        formalism::GeneralizedProblem generalized_problem,
        std::unordered_map<graphs::Color, std::string> color_to_name,
        formalism::PredicateMaps<graphs::Color, formalism::StaticTag, formalism::FluentTag, formalism::DerivedTag> predicate_to_color_offsets);

public:
    static GeneralizedColorFunction create(formalism::GeneralizedProblem generalized_problem);
    static GeneralizedColorFunction create(formalism::Problem problem);

    /// @brief get color of object.
    graphs::Color get_color(formalism::Object object) const;

    /// @brief Get unique color of state atom.
    template<formalism::IsStaticOrFluentOrDerivedTag P>
    graphs::Color get_color(formalism::GroundAtom<P> atom, size_t pos) const;

    /// @brief Get unique color of goal literal.
    template<formalism::IsStaticOrFluentOrDerivedTag P>
    graphs::Color get_color(formalism::GroundLiteral<P> literal, size_t pos) const;

    /// @brief Get name of color.
    const std::string& get_color_name(graphs::Color color) const;
};

}
#endif