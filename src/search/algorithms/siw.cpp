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

#include "mimir/search/algorithms/siw.hpp"

// Keep all implementations in the same translation unit
#include "mimir/search/algorithms/siw/goal_strategy.hpp"

#include <sstream>

namespace mimir
{

ProblemGoalCounter::ProblemGoalCounter(Problem problem, State state) : m_problem(problem), m_initial_num_unsatisfied_goals(count_unsatisfied_goals(state)) {}

int ProblemGoalCounter::count_unsatisfied_goals(const State state) const
{
    int num_unsatisfied_goals = 0;
    for (const auto& literal : m_problem->get_goal_condition<Fluent>())
    {
        if (!state.literal_holds(literal))
        {
            ++num_unsatisfied_goals;
        }
    }
    for (const auto& literal : m_problem->get_goal_condition<Derived>())
    {
        if (!state.literal_holds(literal))
        {
            ++num_unsatisfied_goals;
        }
    }
    return num_unsatisfied_goals;
}

bool ProblemGoalCounter::test_static_goal() { return m_problem->static_goal_holds(); }

bool ProblemGoalCounter::test_dynamic_goal(const State state) { return count_unsatisfied_goals(state) < m_initial_num_unsatisfied_goals; }

}
