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

#include "mimir/formalism/color_function.hpp"

#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/generalized_problem.hpp"
#include "mimir/formalism/problem.hpp"

#include <fmt/core.h>

namespace mimir::formalism
{

ColorFunctionImpl::ColorFunctionImpl(GeneralizedProblem generalized_problem,
                                     std::unordered_map<graphs::Color, std::string> color_to_name,
                                     PredicateMaps<graphs::Color, StaticTag, FluentTag, DerivedTag> predicate_to_color_offsets) :
    m_generalized_problem(std::move(generalized_problem)),
    m_color_to_name(std::move(color_to_name)),
    m_predicate_to_color_offsets(std::move(predicate_to_color_offsets))
{
}

ColorFunction ColorFunctionImpl::create(GeneralizedProblem generalized_problem)
{
    /* Assign global colors and global positional argument vertex indices. */
    auto color_to_name = std::unordered_map<graphs::Color, std::string> {};
    auto predicate_to_color_offsets = PredicateMaps<graphs::Color, StaticTag, FluentTag, DerivedTag> {};

    auto next_color = 1;  // 0 is reserved for objects

    boost::hana::for_each(generalized_problem->get_domain()->get_hana_predicates(),
                          [&](auto&& pair)
                          {
                              const auto& key = boost::hana::first(pair);
                              const auto& value = boost::hana::second(pair);

                              for (const auto& predicate : value)
                              {
                                  boost::hana::at_key(predicate_to_color_offsets, key).emplace(predicate, next_color);

                                  for (size_t i = 0; i < predicate->get_arity(); ++i)
                                  {
                                      color_to_name.emplace(next_color++, fmt::format("{}_{}", to_string(predicate), i));
                                      color_to_name.emplace(next_color++, fmt::format("{}_{}_negative", to_string(predicate), i));
                                      color_to_name.emplace(next_color++, fmt::format("{}_{}_positive", to_string(predicate), i));
                                  }
                              }
                          });

    /* Problem-specific colors differ from problem to problem.
       This is a current limitation of this implementation, which limits to problems with conjunctive goal atoms.
       Since description logics also has this limitation, we can live with this.
    */
    for (const auto& problem : generalized_problem->get_problems())
    {
        for (const auto& predicate : problem->get_derived_predicates())
        {
            boost::hana::at_key(predicate_to_color_offsets, boost::hana::type<DerivedTag> {}).emplace(predicate, next_color);

            for (size_t i = 0; i < predicate->get_arity(); ++i)
            {
                color_to_name.emplace(next_color++, fmt::format("{}_{}", to_string(predicate), i));
                color_to_name.emplace(next_color++, fmt::format("{}_{}_negative", to_string(predicate), i));
                color_to_name.emplace(next_color++, fmt::format("{}_{}_positive", to_string(predicate), i));
            }
        }
    }

    return std::shared_ptr<ColorFunctionImpl>(
        new ColorFunctionImpl(std::move(generalized_problem), std::move(color_to_name), std::move(predicate_to_color_offsets)));
}

ColorFunction ColorFunctionImpl::create(Problem problem) { return create(GeneralizedProblemImpl::create(problem->get_domain(), ProblemList { problem })); }

graphs::Color ColorFunctionImpl::get_color(Object object) const { return 0; }

template<IsStaticOrFluentOrDerivedTag P>
graphs::Color ColorFunctionImpl::get_color(GroundAtom<P> atom, size_t pos) const
{
    assert(pos < atom->get_predicate()->get_arity());

    return boost::hana::at_key(m_predicate_to_color_offsets, boost::hana::type<P> {}).at(atom->get_predicate()) + 3 * pos;
}

template graphs::Color ColorFunctionImpl::get_color(GroundAtom<StaticTag> atom, size_t pos) const;
template graphs::Color ColorFunctionImpl::get_color(GroundAtom<FluentTag> atom, size_t pos) const;
template graphs::Color ColorFunctionImpl::get_color(GroundAtom<DerivedTag> atom, size_t pos) const;

template<IsStaticOrFluentOrDerivedTag P>
graphs::Color ColorFunctionImpl::get_color(GroundLiteral<P> literal, size_t pos) const
{
    assert(pos < literal->get_atom()->get_predicate()->get_arity());

    return boost::hana::at_key(m_predicate_to_color_offsets, boost::hana::type<P> {}).at(literal->get_atom()->get_predicate()) + 3 * pos + 1
           + literal->get_polarity();
}

template graphs::Color ColorFunctionImpl::get_color(GroundLiteral<StaticTag> literal, size_t pos) const;
template graphs::Color ColorFunctionImpl::get_color(GroundLiteral<FluentTag> literal, size_t pos) const;
template graphs::Color ColorFunctionImpl::get_color(GroundLiteral<DerivedTag> literal, size_t pos) const;

const std::string& ColorFunctionImpl::get_color_name(graphs::Color color) const { return m_color_to_name.at(color); }

}
