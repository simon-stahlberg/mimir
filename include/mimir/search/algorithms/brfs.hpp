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

#include "mimir/common/printers.hpp"
#include "mimir/formalism/formalism.hpp"
#include "mimir/search/algorithms/event_handlers.hpp"
#include "mimir/search/algorithms/interface.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/heuristics.hpp"
#include "mimir/search/search_nodes/cost.hpp"
#include "mimir/search/successor_state_generators.hpp"

#include <deque>
#include <flatmemory/flatmemory.hpp>
#include <functional>
#include <ostream>
#include <tuple>
#include <vector>

namespace mimir
{

/**
 * Specialized implementation class.
 */
class BrFsAlgorithm : public IAlgorithm
{
private:
    std::shared_ptr<IDynamicAAG> m_successor_generator;
    std::shared_ptr<IDynamicSSG> m_state_repository;
    State m_initial_state;
    std::deque<State> m_queue;
    flat::CostSearchNodeVector m_search_nodes;
    std::shared_ptr<IAlgorithmEventHandler> m_event_handler;

    /// @brief Compute the plan consisting of ground actions by collecting the creating actions
    ///        and reversing them.
    /// @param view The search node from which backtracking begins for which the goal was
    ///             satisfied.
    /// @param[out] out_plan The sequence of ground actions that leads from the initial state to
    ///                      the to the state underlying the search node.
    void set_plan(const ConstCostSearchNode& view, GroundActionList& out_plan) const
    {
        out_plan.clear();
        auto cur_view = view;

        while (cur_view.get_parent_state_id() != -1)
        {
            out_plan.push_back(m_successor_generator->get_action(cur_view.get_creating_action_id()));

            cur_view = ConstCostSearchNode(this->m_search_nodes[cur_view.get_parent_state_id()]);
        }

        std::reverse(out_plan.begin(), out_plan.end());
    }

    /// @brief Creates a CostSearchNodeBuilderProxy whose attributes are default initialized.
    static auto create_default_search_node_builder()
    {
        auto builder = CostSearchNodeBuilder();
        builder.set_status(SearchNodeStatus::CLOSED);
        builder.set_g_value(-1);
        builder.set_parent_state_id(-1);
        builder.set_creating_action_id(-1);
        builder.finish();
        return builder.get_flatmemory_builder();
    }

public:
    /// @brief Simplest construction
    explicit BrFsAlgorithm(std::shared_ptr<IDynamicAAG> applicable_action_generator) :
        BrFsAlgorithm(applicable_action_generator,
                      std::make_shared<SuccessorStateGenerator>(applicable_action_generator),
                      std::make_shared<DefaultAlgorithmEventHandler>())
    {
    }

    /// @brief Complete construction
    BrFsAlgorithm(std::shared_ptr<IDynamicAAG> applicable_action_generator,
                  std::shared_ptr<IDynamicSSG> successor_state_generator,
                  std::shared_ptr<IAlgorithmEventHandler> event_handler) :
        m_successor_generator(std::move(applicable_action_generator)),
        m_state_repository(std::move(successor_state_generator)),
        m_initial_state(m_state_repository->get_or_create_initial_state()),
        m_search_nodes(flat::CostSearchNodeVector(create_default_search_node_builder())),
        m_event_handler(std::move(event_handler))
    {
    }

    SearchStatus find_solution(GroundActionList& out_plan) override
    {
        const auto problem = m_successor_generator->get_problem();
        const auto& pddl_factories = m_successor_generator->get_pddl_factories();
        m_event_handler->on_start_search(problem, this->m_initial_state, pddl_factories);

        auto initial_search_node = CostSearchNode(this->m_search_nodes[this->m_initial_state.get_id()]);
        initial_search_node.get_g_value() = 0;
        initial_search_node.get_status() = SearchNodeStatus::OPEN;

        if (!problem->static_goal_holds())
        {
            m_event_handler->on_unsolvable();

            return SearchStatus::UNSOLVABLE;
        }

        const auto& fluent_goal_ground_literals = problem->get_fluent_goal_condition();
        const auto& derived_goal_ground_literals = problem->get_derived_goal_condition();

        auto applicable_actions = GroundActionList {};

        m_queue.emplace_back(m_initial_state);

        auto g_value = uint64_t { 0 };

        while (!m_queue.empty())
        {
            const auto state = m_queue.front();
            m_queue.pop_front();

            // We need this before goal test for correct statistics reporting.
            auto search_node = CostSearchNode(this->m_search_nodes[state.get_id()]);
            search_node.get_status() = SearchNodeStatus::CLOSED;
            if (static_cast<uint64_t>(search_node.get_g_value()) > g_value)
            {
                g_value = search_node.get_g_value();
                m_successor_generator->on_finish_f_layer();
                m_event_handler->on_finish_f_layer();
            }

            if (state.literals_hold(fluent_goal_ground_literals) && state.literals_hold(derived_goal_ground_literals))
            {
                set_plan(ConstCostSearchNode(this->m_search_nodes[state.get_id()]), out_plan);

                m_event_handler->on_end_search();
                m_successor_generator->on_end_search();
                m_event_handler->on_solved(out_plan);

                return SearchStatus::SOLVED;
            }

            if (g_value == 1)
            {
                // return SearchStatus::FAILED;
            }

            m_event_handler->on_expand_state(problem, state, pddl_factories);

            this->m_successor_generator->generate_applicable_actions(state, applicable_actions);

            for (const auto& action : applicable_actions)
            {
                const auto state_count = m_state_repository->get_state_count();
                const auto& successor_state = this->m_state_repository->get_or_create_successor_state(state, action);

                m_event_handler->on_generate_state(problem, action, successor_state, pddl_factories);

                if (state_count != m_state_repository->get_state_count())
                {
                    auto successor_search_node = CostSearchNode(this->m_search_nodes[successor_state.get_id()]);
                    successor_search_node.get_status() = SearchNodeStatus::OPEN;
                    successor_search_node.get_g_value() = search_node.get_g_value() + 1;
                    successor_search_node.get_parent_state_id() = state.get_id();
                    successor_search_node.get_creating_action_id() = action.get_id();

                    m_queue.emplace_back(successor_state);
                }
            }
        }

        m_event_handler->on_end_search();
        m_event_handler->on_exhausted();

        return SearchStatus::EXHAUSTED;
    }
};

}

#endif
