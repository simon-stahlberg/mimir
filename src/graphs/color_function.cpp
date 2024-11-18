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

#include "mimir/graphs/color_function.hpp"

#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/problem.hpp"

namespace mimir
{
/**
 * ProblemColorFunction
 */

ProblemColorFunction::ProblemColorFunction(Problem problem) : m_problem(problem), m_name_to_color(), m_color_to_name() { initialize_predicates(); }

void ProblemColorFunction::initialize_predicates()
{
    std::vector<std::pair<std::string, size_t>> lexicographically_sorted_predicates;

    auto add_predicates = [&](const auto& predicates)
    {
        for (const auto& predicate : predicates)
        {
            lexicographically_sorted_predicates.emplace_back(predicate->get_name(), predicate->get_arity());
        }
    };

    add_predicates(m_problem->get_domain()->get_predicates<Static>());
    add_predicates(m_problem->get_domain()->get_predicates<Fluent>());
    add_predicates(m_problem->get_domain()->get_predicates<Derived>());

    // Sort the vector lexicographically by the string in the pair
    std::sort(lexicographically_sorted_predicates.begin(),
              lexicographically_sorted_predicates.end(),
              [](const std::pair<std::string, size_t>& a, const std::pair<std::string, size_t>& b) { return a.first < b.first; });

    // Object color
    const auto color = m_color_to_name.size();
    const auto name = "";
    m_color_to_name.emplace(color, name);
    m_name_to_color.emplace(name, color);

    // Predicate colors
    for (size_t predicate_index = 0; predicate_index < lexicographically_sorted_predicates.size(); ++predicate_index)
    {
        const auto& [predicate_name, predicate_arity] = lexicographically_sorted_predicates.at(predicate_index);

        for (size_t pos = 0; pos < predicate_arity; ++pos)
        {
            // atom
            auto color = m_color_to_name.size();
            auto name = predicate_name + ":" + std::to_string(pos);
            m_color_to_name.emplace(color, name);
            m_name_to_color.emplace(name, color);
            // goal atom
            color = m_color_to_name.size();
            name = predicate_name + ":g:" + std::to_string(pos);
            m_color_to_name.emplace(color, name);
            m_name_to_color.emplace(name, color);
            // marked goal atoms
            color = m_color_to_name.size();
            name = predicate_name + ":g:true:" + std::to_string(pos);
            m_color_to_name.emplace(color, name);
            m_name_to_color.emplace(name, color);
            color = m_color_to_name.size();
            name = predicate_name + ":g:false:" + std::to_string(pos);
            m_color_to_name.emplace(color, name);
            m_name_to_color.emplace(name, color);
        }
    }
}

Color ProblemColorFunction::get_color(Object object) const { return m_name_to_color.at(""); }

template<PredicateTag P>
Color ProblemColorFunction::get_color(GroundAtom<P> atom, size_t pos) const
{
    return m_name_to_color.at(atom->get_predicate()->get_name() + ":" + std::to_string(pos));
}

template Color ProblemColorFunction::get_color(GroundAtom<Static> atom, size_t pos) const;
template Color ProblemColorFunction::get_color(GroundAtom<Fluent> atom, size_t pos) const;
template Color ProblemColorFunction::get_color(GroundAtom<Derived> atom, size_t pos) const;

template<DynamicPredicateTag P>
Color ProblemColorFunction::get_color(State state, GroundLiteral<P> literal, size_t pos, bool mark_true_goal_literal) const
{
    bool is_satisfied_in_goal = state->literal_holds(literal);
    return m_name_to_color.at(literal->get_atom()->get_predicate()->get_name() + ":g"
                              + (mark_true_goal_literal ? (is_satisfied_in_goal ? ":true" : ":false") : "") + ":" + std::to_string(pos));
}

template Color ProblemColorFunction::get_color(State state, GroundLiteral<Fluent> literal, size_t pos, bool mark_true_goal_literal) const;
template Color ProblemColorFunction::get_color(State state, GroundLiteral<Derived> literal, size_t pos, bool mark_true_goal_literal) const;

Color ProblemColorFunction::get_color(State state, GroundLiteral<Static> literal, size_t pos, bool mark_true_goal_literal) const
{
    bool is_satisfied_in_goal = m_problem->static_literal_holds(literal);
    return m_name_to_color.at(literal->get_atom()->get_predicate()->get_name() + ":g"
                              + (mark_true_goal_literal ? (is_satisfied_in_goal ? ":true" : ":false") : "") + ":" + std::to_string(pos));
}

const std::string& ProblemColorFunction::get_color_name(Color color) const
{
    if (!m_color_to_name.contains(color))
    {
        throw std::runtime_error("ProblemColorFunction::get_color_name: failed to map integer color \"" + std::to_string(color) + "\" to string.");
    }
    return m_color_to_name.at(color);
}

Problem ProblemColorFunction::get_problem() const { return m_problem; }

const std::unordered_map<std::string, Color>& ProblemColorFunction::get_name_to_color() const { return m_name_to_color; }

const std::unordered_map<Color, std::string>& ProblemColorFunction::get_color_to_name() const { return m_color_to_name; }

}