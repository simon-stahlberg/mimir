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

#include "mimir/search/algorithms/interface.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/heuristics.hpp"
#include "mimir/search/successor_state_generator.hpp"

#include <vector>

namespace mimir
{

/**
 * Specialized implementation class.
 */
class AStarAlgorithm : public IAlgorithm
{
private:
    std::shared_ptr<IApplicableActionGenerator> m_aag;
    std::shared_ptr<SuccessorStateGenerator> m_ssg;
    State m_initial_state;
    std::shared_ptr<IHeuristic> m_heuristic;

public:
    /// @brief Simplest construction
    AStarAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator, std::shared_ptr<IHeuristic> heuristic) :
        AStarAlgorithm(applicable_action_generator, std::make_shared<SuccessorStateGenerator>(applicable_action_generator), std::move(heuristic))
    {
    }

    /// @brief Complete construction
    AStarAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                   std::shared_ptr<SuccessorStateGenerator> successor_state_generator,
                   std::shared_ptr<IHeuristic> heuristic) :
        m_aag(std::move(applicable_action_generator)),
        m_ssg(std::move(successor_state_generator)),
        m_initial_state(m_ssg->get_or_create_initial_state()),
        m_heuristic(std::move(heuristic))
    {
    }

    SearchStatus find_solution(std::vector<GroundAction>& out_plan) override { return find_solution(m_initial_state, out_plan); }

    SearchStatus find_solution(State start_state, std::vector<GroundAction>& out_plan) override
    {
        // TODO (Dominik): implement
        return SearchStatus::FAILED;
    }

    SearchStatus find_solution(State start_state, std::vector<GroundAction>& out_plan, std::optional<State>& out_goal_state) override
    {
        // TODO (Dominik): implement
        return SearchStatus::FAILED;
    }
};

}

#endif
