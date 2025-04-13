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

#include "mimir/search/heuristics/hstar.hpp"

#include "mimir/datasets/state_space.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/state_repository.hpp"

using namespace mimir::formalism;

namespace mimir::search
{

HStarHeuristicImpl::HStarHeuristicImpl(const SearchContext& context) : m_estimates()
{
    // We simply create a state space and copy the estimates
    auto state_space_options = datasets::StateSpaceImpl::Options();
    state_space_options.remove_if_unsolvable = false;

    auto state_space = datasets::StateSpaceImpl::create(context, state_space_options);
    assert(state_space);

    for (const auto& v : state_space->first->get_graph().get_vertices())
    {
        m_estimates.emplace(graphs::get_state(v), graphs::get_action_goal_distance(v));
    }
}

HStarHeuristic HStarHeuristicImpl::create(const SearchContext& context) { return std::make_shared<HStarHeuristicImpl>(context); }

double HStarHeuristicImpl::compute_heuristic(State state, bool is_goal_state) { return m_estimates.at(state); }
}