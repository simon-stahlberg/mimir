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

#ifndef MIMIR_SEARCH_ALGORITHMS_SIW_HPP_
#define MIMIR_SEARCH_ALGORITHMS_SIW_HPP_

#include "mimir/search/algorithms/event_handlers.hpp"
#include "mimir/search/algorithms/interface.hpp"
#include "mimir/search/algorithms/iw.hpp"
#include "mimir/search/algorithms/siw/goal_strategy.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/successor_state_generators.hpp"

#include <memory>
#include <optional>

namespace mimir
{

class SerializedIterativeWidthAlgorithm : public IAlgorithm
{
private:
    std::shared_ptr<IApplicableActionGenerator> m_aag;
    int m_max_arity;

    std::shared_ptr<ISuccessorStateGenerator> m_ssg;
    std::shared_ptr<IAlgorithmEventHandler> m_event_handler;

    std::shared_ptr<FluentAndDerivedMapper> m_atom_index_mapper;

    State m_initial_state;
    IWAlgorithm m_iw;

public:
    /// @brief Simplest construction
    SerializedIterativeWidthAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator, int max_arity) :
        SerializedIterativeWidthAlgorithm(applicable_action_generator,
                                          max_arity,
                                          std::make_shared<SuccessorStateGenerator>(applicable_action_generator),
                                          std::make_shared<DebugAlgorithmEventHandler>())
    {
    }

    /// @brief Complete construction
    SerializedIterativeWidthAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                                      int max_arity,
                                      std::shared_ptr<ISuccessorStateGenerator> successor_state_generator,
                                      std::shared_ptr<IAlgorithmEventHandler> event_handler) :
        m_aag(applicable_action_generator),
        m_max_arity(max_arity),
        m_ssg(successor_state_generator),
        m_event_handler(event_handler),
        m_atom_index_mapper(std::make_shared<FluentAndDerivedMapper>()),
        m_initial_state(m_ssg->get_or_create_initial_state()),
        m_iw(applicable_action_generator, max_arity, successor_state_generator, event_handler)
    {
        if (max_arity < 0)
        {
            throw std::runtime_error("Arity must be greater of equal than 0.");
        }
    }

    SearchStatus find_solution(GroundActionList& out_plan) override { return find_solution(m_initial_state, out_plan); }

    SearchStatus find_solution(const State start_state, GroundActionList& out_plan) override
    {
        std::optional<State> unused_out_state = std::nullopt;
        return find_solution(start_state, out_plan, unused_out_state);
    }

    SearchStatus find_solution(const State start_state, GroundActionList& out_plan, std::optional<State>& out_goal_state) override
    {
        auto problem_goal_test = std::make_unique<ProblemGoal>(m_aag->get_problem());

        if (!problem_goal_test->test_static_goal())
        {
            return SearchStatus::UNSOLVABLE;
        }

        auto cur_state = start_state;
        std::optional<State> goal_state = std::nullopt;

        while (!problem_goal_test->test_dynamic_goal(cur_state))
        {
            // Run IW to decrease goal counter

            auto partial_plan = GroundActionList {};

            auto search_status = m_iw.find_solution(cur_state, std::make_unique<ProblemGoalCounter>(m_aag->get_problem(), cur_state), partial_plan, goal_state);

            if (search_status != SearchStatus::SOLVED)
            {
                return search_status;
            }

            assert(goal_state.has_value());
            cur_state = goal_state.value();
        }

        out_goal_state = goal_state;
        return SearchStatus::SOLVED;
    }
};

using SIWAlgorithm = SerializedIterativeWidthAlgorithm;
}

#endif
