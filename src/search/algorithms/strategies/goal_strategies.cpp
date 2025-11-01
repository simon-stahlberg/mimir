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

#include "mimir/formalism/problem.hpp"
#include "mimir/search/algorithms/strategies/goal_strategy.hpp"
#include "mimir/search/applicability.hpp"
#include "mimir/search/state.hpp"

using namespace mimir::formalism;

namespace mimir::search
{
ProblemGoalStrategyImpl::ProblemGoalStrategyImpl(Problem problem) : m_problem(problem) {}

bool ProblemGoalStrategyImpl::test_static_goal() { return m_problem->static_goal_holds(); }

bool ProblemGoalStrategyImpl::test_dynamic_goal(const State& state) { return is_dynamically_applicable(m_problem->get_goal_condition(), state); }

ProblemGoalStrategy ProblemGoalStrategyImpl::create(formalism::Problem problem) { return std::make_shared<ProblemGoalStrategyImpl>(problem); }
}
