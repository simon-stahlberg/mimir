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

#ifndef MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
#define MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/algorithms/interface.hpp"
#include "mimir/search/declarations.hpp"

#include <memory>
#include <optional>
#include <vector>

namespace mimir
{

/**
 * Specialized implementation class.
 */
class AStarAlgorithm : public IAlgorithm
{
public:
    /// @brief Simplest construction
    AStarAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator, std::shared_ptr<IHeuristic> heuristic);

    /// @brief Complete construction
    AStarAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                   std::shared_ptr<StateRepository> successor_state_generator,
                   std::shared_ptr<IHeuristic> heuristic,
                   std::shared_ptr<IAStarAlgorithmEventHandler> event_handler);

    SearchStatus find_solution(GroundActionList& out_plan) override;

    SearchStatus find_solution(State start_state, GroundActionList& out_plan) override;

    SearchStatus find_solution(State start_state, GroundActionList& out_plan, std::optional<State>& out_goal_state) override;

    SearchStatus find_solution(State start_state,
                               std::unique_ptr<IGoalStrategy>&& goal_strategy,
                               std::unique_ptr<IPruningStrategy>&& pruning_strategy,
                               GroundActionList& out_plan,
                               std::optional<State>& out_goal_state);

    const std::shared_ptr<PDDLFactories>& get_pddl_factories() const override;

private:
    std::shared_ptr<IApplicableActionGenerator> m_aag;
    std::shared_ptr<StateRepository> m_ssg;
    std::shared_ptr<IHeuristic> m_heuristic;
    std::shared_ptr<IAStarAlgorithmEventHandler> m_event_handler;
};

}

#endif
