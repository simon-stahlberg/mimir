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

#ifndef MIMIR_SEARCH_ALGORITHMS_STRATEGIES_GOAL_STRATEGY_HPP_
#define MIMIR_SEARCH_ALGORITHMS_STRATEGIES_GOAL_STRATEGY_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir::search
{

/// @brief `IGoalStrategy` encapsulates logic to test whether a state is a goal.
class IGoalStrategy
{
public:
    virtual ~IGoalStrategy() = default;

    virtual bool test_static_goal() = 0;
    virtual bool test_dynamic_goal(State state) = 0;
};

/// @brief `ProblemGoalStrategyImpl` identifies a state as a goal if and only if it satisfies the goal in the given problem.
class ProblemGoalStrategyImpl : public IGoalStrategy
{
private:
    formalism::Problem m_problem;

public:
    explicit ProblemGoalStrategyImpl(formalism::Problem problem);

    bool test_static_goal() override;
    bool test_dynamic_goal(State state) override;

    static ProblemGoalStrategy create(formalism::Problem problem);
};
}

#endif