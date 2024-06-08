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

#ifndef MIMIR_SEARCH_PLAN_HPP_
#define MIMIR_SEARCH_PLAN_HPP_

#include "mimir/search/action.hpp"

#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

namespace mimir
{

class Plan
{
private:
    std::vector<std::string> m_actions;
    uint64_t m_cost;

public:
    Plan(std::vector<std::string> actions, uint64_t cost) : m_actions(std::move(actions)), m_cost(cost) {}

    const std::vector<std::string>& get_actions() const { return m_actions; }

    uint64_t get_cost() const { return m_cost; }
};

/// @brief Translates a ground action list to a plan
extern Plan to_plan(const GroundActionList& action_view_list);

/// @brief Write the plan to an ostream.
extern std::ostream& operator<<(std::ostream& os, const Plan& plan);

}

#endif
