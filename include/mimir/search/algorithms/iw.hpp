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
#include "mimir/search/algorithms/brfs/event_handlers.hpp"
#include "mimir/search/algorithms/interface.hpp"
#include "mimir/search/algorithms/iw/event_handlers.hpp"
#include "mimir/search/algorithms/iw/index_mappers.hpp"
#include "mimir/search/algorithms/iw/pruning_strategy.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/successor_state_generator.hpp"

namespace mimir
{

class IterativeWidthAlgorithm : public IAlgorithm
{
private:
    std::shared_ptr<IApplicableActionGenerator> m_aag;
    int m_max_arity;

    std::shared_ptr<SuccessorStateGenerator> m_ssg;
    std::shared_ptr<IBrFSAlgorithmEventHandler> m_brfs_event_handler;
    std::shared_ptr<IIWAlgorithmEventHandler> m_iw_event_handler;

    std::shared_ptr<FluentAndDerivedMapper> m_atom_index_mapper;

    State m_initial_state;
    BrFSAlgorithm m_brfs;

public:
    /// @brief Simplest construction
    IterativeWidthAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator, int max_arity) :
        IterativeWidthAlgorithm(applicable_action_generator,
                                max_arity,
                                std::make_shared<SuccessorStateGenerator>(applicable_action_generator),
                                std::make_shared<DefaultBrFSAlgorithmEventHandler>(),
                                std::make_shared<DefaultIWAlgorithmEventHandler>())
    {
    }

    /// @brief Complete construction
    IterativeWidthAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                            int max_arity,
                            std::shared_ptr<SuccessorStateGenerator> successor_state_generator,
                            std::shared_ptr<IBrFSAlgorithmEventHandler> brfs_event_handler,
                            std::shared_ptr<IIWAlgorithmEventHandler> iw_event_handler) :
        m_aag(applicable_action_generator),
        m_max_arity(max_arity),
        m_ssg(successor_state_generator),
        m_brfs_event_handler(brfs_event_handler),
        m_iw_event_handler(iw_event_handler),
        m_atom_index_mapper(std::make_shared<FluentAndDerivedMapper>()),
        m_initial_state(m_ssg->get_or_create_initial_state()),
        m_brfs(applicable_action_generator, successor_state_generator, brfs_event_handler)
    {
        if (max_arity < 0)
        {
            throw std::runtime_error("Arity must be greater of equal than 0.");
        }
    }

    SearchStatus find_solution(GroundActionList& out_plan) override { return find_solution(m_initial_state, out_plan); }

    SearchStatus find_solution(State start_state, GroundActionList& out_plan) override
    {
        std::optional<State> unused_out_state = std::nullopt;
        return find_solution(start_state, std::make_unique<ProblemGoal>(m_aag->get_problem()), out_plan, unused_out_state);
    }

    SearchStatus find_solution(State start_state, GroundActionList& out_plan, std::optional<State>& out_goal_state) override
    {
        return find_solution(start_state, std::make_unique<ProblemGoal>(m_aag->get_problem()), out_plan, out_goal_state);
    }

    SearchStatus
    find_solution(State start_state, std::unique_ptr<IGoalStrategy>&& goal_strategy, GroundActionList& out_plan, std::optional<State>& out_goal_state)
    {
        const auto problem = m_aag->get_problem();
        const auto& pddl_factories = *m_aag->get_pddl_factories();
        m_iw_event_handler->on_start_search(problem, start_state, pddl_factories);

        int cur_arity = 0;
        while (cur_arity <= m_max_arity)
        {
            m_iw_event_handler->on_start_arity_search(problem, start_state, pddl_factories, cur_arity);

            auto search_status = (cur_arity > 0) ?
                                     m_brfs.find_solution(start_state,
                                                          std::move(goal_strategy),
                                                          std::make_unique<ArityKNoveltyPruning>(cur_arity, INITIAL_TABLE_ATOMS, m_atom_index_mapper),
                                                          out_plan,
                                                          out_goal_state) :
                                     m_brfs.find_solution(start_state,
                                                          std::move(goal_strategy),
                                                          std::make_unique<ArityZeroNoveltyPruning>(start_state),
                                                          out_plan,
                                                          out_goal_state);

            m_iw_event_handler->on_end_arity_search(m_brfs_event_handler->get_statistics());

            if (search_status == SearchStatus::SOLVED)
            {
                m_iw_event_handler->on_end_search();
                if (!m_iw_event_handler->is_quiet())
                {
                    m_aag->on_end_search();
                }
                m_iw_event_handler->on_solved(out_plan);
                return SearchStatus::SOLVED;
            }
            else if (search_status == SearchStatus::UNSOLVABLE)
            {
                m_iw_event_handler->on_unsolvable();
                return SearchStatus::UNSOLVABLE;
            }

            ++cur_arity;
        }
        return SearchStatus::FAILED;
    }
};

using IWAlgorithm = IterativeWidthAlgorithm;
}

#endif
