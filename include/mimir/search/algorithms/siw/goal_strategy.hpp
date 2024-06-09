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

#ifndef MIMIR_SEARCH_ALGORITHMS_SIW_GOAL_STRATEGY_HPP_
#define MIMIR_SEARCH_ALGORITHMS_SIW_GOAL_STRATEGY_HPP_

#include "mimir/search/algorithms/strategies/goal_strategy.hpp"

namespace mimir
{

class ProblemGoalCounter : public IGoalStrategy
{
private:
    Problem m_problem;

    int m_initial_num_unsatisfied_goals;

    int count_unsatisfied_goals(const State state) const;

public:
    explicit ProblemGoalCounter(Problem problem, State state);

    bool test_static_goal() override;
    bool test_dynamic_goal(const State state) override;
};
}

#endif