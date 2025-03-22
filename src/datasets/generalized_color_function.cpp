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

#include "mimir/datasets/generalized_color_function.hpp"

#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/state.hpp"

using namespace mimir::formalism;
using namespace mimir::search;

namespace mimir::datasets
{

GeneralizedColorFunctionImpl::GeneralizedColorFunctionImpl(GeneralizedProblem generalized_problem) :
    m_generalized_problem(std::move(generalized_problem)),
    m_color_to_name(),
    m_predicate_colors()
{
    auto next_color = 1;  // 0 is reserved for objects

    boost::hana::for_each(m_generalized_problem.get_domain()->get_hana_predicates(),
                          [&](auto&& pair)
                          {
                              const auto& key = boost::hana::first(pair);
                              const auto& value = boost::hana::second(pair);

                              for (const auto& predicate : value)
                              {
                                  boost::hana::at_key(m_predicate_colors, key).emplace(predicate, next_color);
                                  m_color_to_name.emplace(next_color++, to_string(predicate));
                                  m_color_to_name.emplace(next_color++, to_string(predicate) + "_true");
                                  m_color_to_name.emplace(next_color++, to_string(predicate) + "_false");
                              }
                          });

    /* Problem specific colors differ from problem to problem.
       This is a current limitation of this implementation, which limits to problems with conjunctive goal atoms.
       Since description logics also has this limitation, we can live with this.
    */
    for (const auto& problem : m_generalized_problem.get_problems())
    {
        for (const auto& predicate : problem->get_derived_predicates())
        {
            boost::hana::at_key(m_predicate_colors, boost::hana::type<DerivedTag> {}).emplace(predicate, next_color);
            m_color_to_name.emplace(next_color++, to_string(predicate));
            m_color_to_name.emplace(next_color++, to_string(predicate) + "_true");
            m_color_to_name.emplace(next_color++, to_string(predicate) + "_false");
        }
    }
}

GeneralizedColorFunctionImpl::GeneralizedColorFunctionImpl(formalism::Problem problem) :
    GeneralizedColorFunctionImpl(formalism::GeneralizedProblem(problem->get_domain(), formalism::ProblemList { problem }))
{
}

graphs::Color GeneralizedColorFunctionImpl::get_color(Object object) const { return 0; }

template<IsStaticOrFluentOrDerivedTag P>
graphs::Color GeneralizedColorFunctionImpl::get_color(GroundAtom<P> atom, size_t pos) const
{
    return boost::hana::at_key(m_predicate_colors, boost::hana::type<P> {}).at(atom->get_predicate());
}

template graphs::Color GeneralizedColorFunctionImpl::get_color(GroundAtom<StaticTag> atom, size_t pos) const;
template graphs::Color GeneralizedColorFunctionImpl::get_color(GroundAtom<FluentTag> atom, size_t pos) const;
template graphs::Color GeneralizedColorFunctionImpl::get_color(GroundAtom<DerivedTag> atom, size_t pos) const;

template<IsStaticOrFluentOrDerivedTag P>
graphs::Color
GeneralizedColorFunctionImpl::get_color(GroundLiteral<P> literal, size_t pos, State state, const ProblemImpl& problem, bool mark_true_goal_literal) const
{
    bool is_satisfied_in_goal = state->literal_holds(literal);
    const auto literal_color_offset = (!mark_true_goal_literal || is_satisfied_in_goal) ? 1 : 2;
    return boost::hana::at_key(m_predicate_colors, boost::hana::type<P> {}).at(literal->get_atom()->get_predicate()) + literal_color_offset;
}

template<>
graphs::Color GeneralizedColorFunctionImpl::get_color(GroundLiteral<StaticTag> literal,
                                                      size_t pos,
                                                      State state,
                                                      const ProblemImpl& problem,
                                                      bool mark_true_goal_literal) const
{
    bool is_satisfied_in_goal = problem.static_literal_holds(literal);
    const auto literal_color_offset = (!mark_true_goal_literal || is_satisfied_in_goal) ? 1 : 2;
    return boost::hana::at_key(m_predicate_colors, boost::hana::type<StaticTag> {}).at(literal->get_atom()->get_predicate()) + literal_color_offset;
}

template graphs::Color GeneralizedColorFunctionImpl::get_color(GroundLiteral<FluentTag> literal,
                                                               size_t pos,
                                                               State state,
                                                               const ProblemImpl& problem,
                                                               bool mark_true_goal_literal) const;
template graphs::Color GeneralizedColorFunctionImpl::get_color(GroundLiteral<DerivedTag> literal,
                                                               size_t pos,
                                                               State state,
                                                               const ProblemImpl& problem,
                                                               bool mark_true_goal_literal) const;

const std::string& GeneralizedColorFunctionImpl::get_color_name(graphs::Color color) const { return m_color_to_name.at(color); }

}
