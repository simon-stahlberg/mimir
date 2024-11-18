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

#include "mimir/search/action.hpp"

namespace mimir
{

/* Plan */

Plan::Plan(std::vector<std::string> actions, ContinuousCost cost) : m_actions(std::move(actions)), m_cost(cost) {}

const std::vector<std::string>& Plan::get_actions() const { return m_actions; }

ContinuousCost Plan::get_cost() const { return m_cost; }

/* Utility */

Plan to_plan(const GroundActionList& action_view_list, const PDDLRepositories& factories)
{
    auto actions = std::vector<std::string> {};
    auto cost = ContinuousCost(0);
    for (const auto action : action_view_list)
    {
        std::stringstream ss;
        ss << std::make_tuple(action, std::cref(factories), PlanActionFormatterTag {});
        actions.push_back(ss.str());
        cost += action->get_cost();
    }
    return Plan(std::move(actions), cost);
}

std::ostream& operator<<(std::ostream& os, const Plan& plan)
{
    for (const auto& action : plan.get_actions())
    {
        os << action << "\n";
    }
    os << "; cost = " << plan.get_cost();

    return os;
}

}
