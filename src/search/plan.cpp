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

#include "mimir/search/plan.hpp"

#include "mimir/formalism/ground_action.hpp"
#include "mimir/search/search_context.hpp"

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

using namespace mimir::formalism;

namespace mimir::search
{

/* Plan */

Plan::Plan(SearchContext context, StateList states, GroundActionList actions, ContinuousCost cost) :
    m_context(std::move(context)),
    m_states(std::move(states)),
    m_actions(std::move(actions)),
    m_cost(cost)
{
}

const SearchContext& Plan::get_search_context() const { return m_context; }

const StateList& Plan::get_states() const { return m_states; }

const GroundActionList& Plan::get_actions() const { return m_actions; }

ContinuousCost Plan::get_cost() const { return m_cost; }

size_t Plan::get_length() const { return m_actions.size(); }

std::ostream& operator<<(std::ostream& out, const search::Plan& element) { return mimir::print(out, element); }
}

namespace mimir
{
std::ostream& print(std::ostream& out, const mimir::search::Plan& element)
{
    const auto& problem = *element.get_search_context()->get_problem();
    const auto formatter = GroundActionImpl::PlanFormatterTag {};

    fmt::print(out,
               "{}\n; cost = {}\n",
               fmt::join(element.get_actions()
                             | std::views::transform([&](auto&& action) { return to_string(std::make_tuple(action, std::cref(problem), formatter)); }),
                         "\n"),
               element.get_cost());
    return out;
}
}
