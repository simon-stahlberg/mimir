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

#include "mimir/datasets/generalized_state_space.hpp"
#include "mimir/search/state_repository.hpp"

namespace mimir
{

HStarHeuristic::HStarHeuristic(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator, std::shared_ptr<StateRepository> state_repository) :
    m_estimates()
{
    // We simply create a state space and copy the estimates
    auto class_options = ClassOptions();
    class_options.problem_options.remove_if_unsolvable = false;

    auto problem_class_state_space =
        GeneralizedStateSpace(ProblemContext { state_repository->get_problem(), state_repository, applicable_action_generator }, class_options);

    const auto& class_graph = problem_class_state_space.get_class_state_space().get_graph();

    for (const auto& class_vertex : class_graph.get_vertices())
    {
        const auto& problem_vertex = problem_class_state_space.get_problem_vertex(class_vertex);
        m_estimates.emplace(get_state(problem_vertex), get_action_goal_distance(class_vertex));
    }
}

double HStarHeuristic::compute_heuristic(State state, bool is_goal_state) { return m_estimates.at(state); }

}