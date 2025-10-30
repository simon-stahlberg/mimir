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

#ifndef MIMIR_SEARCH_HEURISTICS_BLIND_HPP_
#define MIMIR_SEARCH_HEURISTICS_BLIND_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/heuristics/interface.hpp"

namespace mimir::search
{

class BlindHeuristicImpl : public IHeuristic
{
public:
    explicit BlindHeuristicImpl(formalism::Problem problem);

    static BlindHeuristic create(formalism::Problem problem);

    ContinuousCost compute_heuristic(const State& state, bool is_goal_state) override { return is_goal_state ? 0. : m_min_action_cost_value; }

private:
    /* If the lower bound on the action costs is non-zero we can use a nonzero heuristic estimate for non-goal states,
       resulting in pushing goal states to the front in states with the same g-value.
       Note that our approximation method throws an exception if it encounters division by zero.
       If that ever happens, we might want to catch the exception and simply use 0.0
    */
    ContinuousCost m_min_action_cost_value;
};

}

#endif
