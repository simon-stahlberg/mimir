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

#include "mimir/search/partially_ordered_plan.hpp"

namespace mimir::search
{

static bool verify_precedes_initialization(const std::vector<std::vector<bool>>& precedes)
{
    // Ensure irreflexivity
    for (size_t i = 0; i < precedes.size(); ++i)
    {
        if (precedes[i][i])
            return false;
    }

    // Ensure transitivity
    for (size_t i = 0; i < precedes.size(); ++i)
    {
        for (size_t j = 0; j < precedes.size(); ++j)
        {
            for (size_t k = 0; k < precedes.size(); ++k)
            {
                if (precedes[i][j] && precedes[j][k] && !precedes[i][k])
                {
                    return false;
                }
            }
        }
    }

    return true;
}

PartiallyOrderedPlan::PartiallyOrderedPlan(Plan t_o_plan) :
    m_t_o_plan(std::move(t_o_plan)),
    m_precedes(m_t_o_plan.get_length(), std::vector<bool>(m_t_o_plan.get_length(), false))
{
    // TODO compute plan
    for (size_t i = 0; i < m_t_o_plan.get_length(); ++i)
    {
        const auto& lhs_action = m_t_o_plan.get_actions().at(i);

        for (size_t j = 0; j < m_t_o_plan.get_length(); ++j)
        {
            const auto& rhs_actions = m_t_o_plan.get_actions().at(j);
        }
    }

    assert(verify_precedes_initialization(m_precedes));
}

bool PartiallyOrderedPlan::must_precede(size_t i, size_t j) const { return m_precedes.at(i).at(j); }

Plan PartiallyOrderedPlan::compute_t_o_plan_with_minimal_context_switches() const
{
    // TODO
    return m_t_o_plan;
}

const Plan& PartiallyOrderedPlan::get_t_o_plan() const { return m_t_o_plan; }

const std::vector<std::vector<bool>>& PartiallyOrderedPlan::get_precedes() const { return m_precedes; }

}
