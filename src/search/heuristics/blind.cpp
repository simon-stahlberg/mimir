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

#include "mimir/formalism/action.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/function.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/function_skeleton.hpp"
#include "mimir/formalism/ground_function.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/requirements.hpp"

using namespace mimir::formalism;

namespace mimir::search
{

/// @brief `BlindHeuristicImpl` returns 0 iff a state is a goal and otherwise, a non-zero value that is as large as possible.
/// If :numeric-fluents or :action-costs is enabled we cannot return a nonzero value without closer inspection of the problem.
/// @param problem
BlindHeuristicImpl::BlindHeuristicImpl(Problem problem) :
    m_min_action_cost_value((problem->get_domain()->get_requirements()->test(loki::RequirementEnum::NUMERIC_FLUENTS)
                             || problem->get_domain()->get_requirements()->test(loki::RequirementEnum::ACTION_COSTS)) ?
                                0. :
                                1.)
{
}

BlindHeuristic BlindHeuristicImpl::create(formalism::Problem problem) { return std::make_shared<BlindHeuristicImpl>(problem); }
}
