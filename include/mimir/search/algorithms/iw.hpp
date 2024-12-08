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

#ifndef MIMIR_SEARCH_ALGORITHMS_IW_HPP_
#define MIMIR_SEARCH_ALGORITHMS_IW_HPP_

#include "mimir/search/algorithms/brfs.hpp"
#include "mimir/search/algorithms/iw/event_handlers/interface.hpp"

namespace mimir
{

class IterativeWidthAlgorithm : public IAlgorithm
{
public:
    /// @brief Simplest construction
    IterativeWidthAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                            std::shared_ptr<StateRepository> state_repository,
                            size_t max_arity);

    /// @brief Complete construction
    IterativeWidthAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                            std::shared_ptr<StateRepository> state_repository,
                            size_t max_arity,
                            std::shared_ptr<IBrFSAlgorithmEventHandler> brfs_event_handler,
                            std::shared_ptr<IIWAlgorithmEventHandler> iw_event_handler);

    SearchResult find_solution() override;

    SearchResult find_solution(State start_state) override;

    SearchResult find_solution(State start_state, std::unique_ptr<IGoalStrategy>&& goal_strategy);

private:
    std::shared_ptr<IApplicableActionGenerator> m_applicable_action_generator;
    std::shared_ptr<StateRepository> m_state_repository;
    size_t m_max_arity;
    std::shared_ptr<IBrFSAlgorithmEventHandler> m_brfs_event_handler;
    std::shared_ptr<IIWAlgorithmEventHandler> m_iw_event_handler;

    State m_initial_state;
    BrFSAlgorithm m_brfs;
};

using IWAlgorithm = IterativeWidthAlgorithm;
}

#endif