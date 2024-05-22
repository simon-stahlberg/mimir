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

#include "mimir/search/algorithms/event_handlers.hpp"
#include "mimir/search/algorithms/interface.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/heuristics.hpp"
#include "mimir/search/successor_state_generators.hpp"

#include <vector>

namespace mimir
{

/**
 * Specialized implementation class.
 */
class AStarAlgorithm : public IAlgorithm
{
private:
    std::shared_ptr<IDynamicAAG> m_successor_generator;
    std::shared_ptr<IDynamicSSG> m_state_repository;
    State m_initial_state;
    std::shared_ptr<IDynamicHeuristic> m_heuristic;
    std::shared_ptr<IAlgorithmEventHandler> m_event_handler;

public:
    /// @brief Simplest construction
    AStarAlgorithm(std::shared_ptr<IDynamicAAG> applicable_action_generator, std::shared_ptr<IDynamicHeuristic> heuristic) :
        AStarAlgorithm(applicable_action_generator,
                       std::make_shared<SuccessorStateGenerator>(applicable_action_generator),
                       std::move(heuristic),
                       std::make_shared<DefaultAlgorithmEventHandler>())
    {
    }

    /// @brief Complete construction
    AStarAlgorithm(std::shared_ptr<IDynamicAAG> applicable_action_generator,
                   std::shared_ptr<IDynamicSSG> successor_state_generator,
                   std::shared_ptr<IDynamicHeuristic> heuristic,
                   std::shared_ptr<IAlgorithmEventHandler> event_handler) :
        m_successor_generator(std::move(applicable_action_generator)),
        m_state_repository(std::move(successor_state_generator)),
        m_initial_state(m_state_repository->get_or_create_initial_state()),
        m_heuristic(std::move(heuristic)),
        m_event_handler(std::move(event_handler))
    {
    }

    SearchStatus find_solution(std::vector<GroundAction>& out_plan) override
    {
        // TODO (Dominik): implement
        return SearchStatus::FAILED;
    }
};

}

#endif
