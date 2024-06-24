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
#include "mimir/search/algorithms/brfs/event_handlers.hpp"
#include "mimir/search/algorithms/interface.hpp"
#include "mimir/search/algorithms/strategies/goal_strategy.hpp"
#include "mimir/search/algorithms/strategies/pruning_strategy.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/search_nodes/uninformed.hpp"
#include "mimir/search/successor_state_generator.hpp"

#include <deque>
#include <flatmemory/flatmemory.hpp>
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
    void set_plan(const ConstUninformedCostSearchNode& view, GroundActionList& out_plan) const
    {
        out_plan.clear();
        auto cur_view = view;

        while (cur_view.get_parent_state_id() != -1)
        {
            out_plan.push_back(m_aag->get_action(cur_view.get_creating_action_id()));

            cur_view = ConstUninformedCostSearchNode(this->m_search_nodes[cur_view.get_parent_state_id()]);
        }

        std::reverse(out_plan.begin(), out_plan.end());
    }

    /// @brief Creates a CostSearchNodeBuilderProxy whose attributes are default initialized.
    static auto create_default_search_node_builder()
    {
        auto builder = UninformedSearchNodeBuilder();
        builder.set_status(SearchNodeStatus::CLOSED);
        builder.set_g_value(-1);
        builder.set_parent_state_id(-1);
        builder.set_creating_action_id(-1);
        builder.finish();
        return builder.get_flatmemory_builder();
    }

public:
    /// @brief Simplest construction
    explicit BrFSAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator) :
        BrFSAlgorithm(applicable_action_generator,
                      std::make_shared<SuccessorStateGenerator>(applicable_action_generator),
                      std::make_shared<DefaultBrFSAlgorithmEventHandler>())
    {
    }

    /// @brief Complete construction
    BrFSAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                  std::shared_ptr<SuccessorStateGenerator> successor_state_generator,
                  std::shared_ptr<IBrFSAlgorithmEventHandler> event_handler) :
        m_aag(std::move(applicable_action_generator)),
        m_ssg(std::move(successor_state_generator)),
        m_initial_state(m_ssg->get_or_create_initial_state()),
        m_search_nodes(FlatUninformedSearchNodeVector(create_default_search_node_builder())),
        m_event_handler(std::move(event_handler))
    {
    }

    SearchStatus find_solution(GroundActionList& out_plan) override { return find_solution(m_initial_state, out_plan); }

    SearchStatus find_solution(State start_state, GroundActionList& out_plan) override
    {
        std::optional<State> unused_out_state = std::nullopt;
        return find_solution(start_state,
                             std::make_unique<ProblemGoal>(m_aag->get_problem()),
                             std::make_unique<DuplicateStatePruning>(),
                             out_plan,
                             unused_out_state);
    }

    SearchStatus find_solution(State start_state, GroundActionList& out_plan, std::optional<State>& out_goal_state) override
    {
        return find_solution(start_state,
                             std::make_unique<ProblemGoal>(m_aag->get_problem()),
                             std::make_unique<DuplicateStatePruning>(),
                             out_plan,
                             out_goal_state);
    }

    SearchStatus find_solution(State start_state,
                               std::unique_ptr<IGoalStrategy>&& goal_strategy,
                               std::unique_ptr<IPruningStrategy>&& pruning_strategy,
                               GroundActionList& out_plan,
                               std::optional<State>& out_goal_state)
    {
        // Clear data structures
        m_search_nodes.clear();
        m_queue.clear();

        const auto problem = m_aag->get_problem();
        const auto& pddl_factories = *m_aag->get_pddl_factories();
        m_event_handler->on_start_search(problem, start_state, pddl_factories);

        auto initial_search_node = UninformedSearchNode(this->m_search_nodes[start_state.get_id()]);
        initial_search_node.get_g_value() = 0;
        initial_search_node.get_status() = SearchNodeStatus::OPEN;

        if (!goal_strategy->test_static_goal())
        {
            m_event_handler->on_unsolvable();

            return SearchStatus::UNSOLVABLE;
        }

        auto applicable_actions = GroundActionList {};

        if (pruning_strategy->test_prune_initial_state(start_state))
        {
            return SearchStatus::FAILED;
        }

        m_queue.emplace_back(start_state);

        auto g_value = uint64_t { 0 };

        while (!m_queue.empty())
        {
            const auto state = m_queue.front();
            m_queue.pop_front();

            // We need this before goal test for correct statistics reporting.
            auto search_node = UninformedSearchNode(this->m_search_nodes[state.get_id()]);
            search_node.get_status() = SearchNodeStatus::CLOSED;

            if (static_cast<uint64_t>(search_node.get_g_value()) > g_value)
            {
                g_value = search_node.get_g_value();
                m_aag->on_finish_f_layer();
                m_event_handler->on_finish_f_layer();
            }

            if (goal_strategy->test_dynamic_goal(state))
            {
                set_plan(ConstUninformedCostSearchNode(this->m_search_nodes[state.get_id()]), out_plan);
                out_goal_state = state;
                m_event_handler->on_end_search();
                if (!m_event_handler->is_quiet())
                {
                    m_aag->on_end_search();
                }
                m_event_handler->on_solved(out_plan);

                return SearchStatus::SOLVED;
            }

            m_event_handler->on_expand_state(problem, state, pddl_factories);

            this->m_aag->generate_applicable_actions(state, applicable_actions);

            for (const auto& action : applicable_actions)
            {
                const auto state_count = m_ssg->get_state_count();
                const auto& successor_state = this->m_ssg->get_or_create_successor_state(state, action);

                m_event_handler->on_generate_state(problem, action, successor_state, pddl_factories);

                bool is_new_successor_state = (state_count != m_ssg->get_state_count());

                if (!pruning_strategy->test_prune_successor_state(state, successor_state, is_new_successor_state))
                {
                    auto successor_search_node = UninformedSearchNode(this->m_search_nodes[successor_state.get_id()]);
                    successor_search_node.get_status() = SearchNodeStatus::OPEN;
                    successor_search_node.get_g_value() = search_node.get_g_value() + 1;
                    successor_search_node.get_parent_state_id() = state.get_id();
                    successor_search_node.get_creating_action_id() = action.get_id();

                    m_queue.emplace_back(successor_state);
                }
                else
                {
                    m_event_handler->on_prune_state(problem, successor_state, pddl_factories);
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
