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

#include "mimir/search/heuristics/blind.hpp"

#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/requirements.hpp"

namespace mimir
{
BlindHeuristic::BlindHeuristic(Problem problem) :
    m_min_action_cost_value((problem->get_domain()->get_requirements()->test(loki::RequirementEnum::ACTION_COSTS)) ? 0. : 1.)
{
    // TODO: try to infer tighter upper bounds on the m_min_action_cost_value
}
}
