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

#ifndef MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_
#define MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/algorithms/brfs/event_handlers/interface.hpp"
#include "mimir/search/algorithms/interface.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/search_nodes/uninformed.hpp"

#include <deque>
#include <functional>
#include <optional>
#include <ostream>
#include <tuple>
#include <vector>

namespace mimir
{

/**
 * Specialized implementation class.
 */
class BrFSAlgorithm : public IAlgorithm
{
private:
    std::shared_ptr<IApplicableActionGenerator> m_aag;
    std::shared_ptr<SuccessorStateGenerator> m_ssg;
    State m_initial_state;
    std::deque<State> m_queue;
    FlatUninformedSearchNodeVector m_search_nodes;
    std::shared_ptr<IBrFSAlgorithmEventHandler> m_event_handler;

    /// @brief Compute the plan consisting of ground actions by collecting the creating actions
    ///        and reversing them.
    /// @param view The search node from which backtracking begins for which the goal was
    ///             satisfied.
    /// @param[out] out_plan The sequence of ground actions that leads from the initial state to
    ///                      the to the state underlying the search node.
    void set_plan(const ConstUninformedCostSearchNode& view, GroundActionList& out_plan) const;

public:
    /// @brief Simplest construction
    explicit BrFSAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator);

    /// @brief Complete construction
    BrFSAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                  std::shared_ptr<SuccessorStateGenerator> successor_state_generator,
                  std::shared_ptr<IBrFSAlgorithmEventHandler> event_handler);

    SearchStatus find_solution(GroundActionList& out_plan) override;

    SearchStatus find_solution(State start_state, GroundActionList& out_plan) override;

    SearchStatus find_solution(State start_state, GroundActionList& out_plan, std::optional<State>& out_goal_state) override;

    SearchStatus find_solution(State start_state,
                               std::unique_ptr<IGoalStrategy>&& goal_strategy,
                               std::unique_ptr<IPruningStrategy>&& pruning_strategy,
                               GroundActionList& out_plan,
                               std::optional<State>& out_goal_state);
};

}

#endif
