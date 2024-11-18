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

namespace mimir
{

HStarHeuristic::HStarHeuristic(Problem problem,
                               std::shared_ptr<PDDLRepositories> pddl_repositories,
                               std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                               std::shared_ptr<StateRepository> state_repository) :
    m_estimates()
{
    // We simply create a state space and copy the estimates
    auto state_space_options = StateSpaceOptions();
    state_space_options.use_unit_cost_one = false;
    state_space_options.remove_if_unsolvable = false;
    auto state_space = StateSpace::create(problem, pddl_repositories, applicable_action_generator, state_repository, state_space_options).value();
    for (size_t state_index = 0; state_index < state_space.get_num_vertices(); ++state_index)
    {
        m_estimates.emplace(get_state(state_space.get_vertex(state_index)), state_space.get_goal_distance(state_index));
    }
}

double HStarHeuristic::compute_heuristic(State state) { return m_estimates.at(state); }

}